/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"
#include "GmshReaderException.hpp"

#include <algorithm>
#include <boost/container/flat_set.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>

#include <json/json.h>

namespace gmsh
{
Reader::Reader(std::string const& fileName, NodalOrdering ordering, IndexingBase base)
    : fileName(fileName),
      useZeroBasedIndexing(base == IndexingBase::Zero),
      useLocalNodalConnectivity(ordering == NodalOrdering::Local)
{
    gmshFile.precision(sizeof(double));
    fillMesh();
}

void Reader::fillMesh()
{
    auto const start = std::chrono::high_resolution_clock::now();

    gmshFile.open(fileName.c_str());

    if (!gmshFile.is_open())
    {
        throw GmshReaderException("Filename " + fileName + " is not valid");
    }

    std::string token, null;

    std::vector<std::string> physicalNames;

    // Loop around file and read in keyword tokens
    while (!gmshFile.eof())
    {
        gmshFile >> token;

        if (token == "$MeshFormat")
        {
            float gmshVersion; // File format version
            short dataType;    // Precision

            gmshFile >> gmshVersion >> dataType >> null;
            checkSupportedGmsh(gmshVersion);
        }
        else if (token == "$PhysicalNames")
        {
            std::string physicalName;
            int physicalIds = 0;
            gmshFile >> physicalIds;
            for (auto i = 0; i < physicalIds; ++i)
            {
                int dimension = 0, physicalId = 0;
                gmshFile >> dimension >> physicalId >> physicalName;

                // Extract the name from the quotes
                physicalName.erase(remove(physicalName.begin(), physicalName.end(), '\"'),
                                   physicalName.end());
                physicalGroupMap.emplace(physicalId, physicalName);
            }
            token.clear();
            gmshFile >> token;
        }
        else if (token == "$Nodes")
        {
            int nodeIds = 0;
            gmshFile >> nodeIds;
            nodeList.resize(nodeIds);

            for (auto& node : nodeList)
            {
                gmshFile >> node.id >> node.coordinates[0] >> node.coordinates[1] >>
                    node.coordinates[2];
            }
        }
        else if (token == "$Elements")
        {
            int elementIds = 0;
            gmshFile >> elementIds;

            for (int elementId = 0; elementId < elementIds; elementId++)
            {
                int id = 0, numberOfTags = 0, elementTypeId = 0;

                gmshFile >> id >> elementTypeId >> numberOfTags;

                auto const numberOfNodes = mapElementData(elementTypeId);

                std::vector<int> tags(numberOfTags, 0);
                std::vector<int> nodalConnectivity(numberOfNodes, 0);

                for (auto& tag : tags)
                {
                    gmshFile >> tag;
                }
                for (auto& nodeId : nodalConnectivity)
                {
                    gmshFile >> nodeId;
                }

                auto const physicalId = tags[0];

                ElementData elementData(nodalConnectivity, tags, elementTypeId, id);

                // Update the total number of partitions on the fly
                number_of_partitions = std::max(elementData.maxProcessId(), number_of_partitions);

                gmshMesh[physicalGroupMap[physicalId]].push_back(elementData);

                meshes[{physicalGroupMap[physicalId], elementTypeId}].push_back(elementData);

                if (elementData.isSharedByMultipleProcesses())
                {
                    for (int i = 4; i < tags[2] + 3; ++i)
                    {
                        auto const ownership = std::make_pair(tags[3], -tags[i]);

                        if (interfaceElementMap.find(ownership) != interfaceElementMap.end())
                        {
                            for (auto const& nodeId : elementData.nodalConnectivity())
                            {
                                interfaceElementMap[ownership].emplace(nodeId);
                            }
                        }
                        else
                        {
                            std::set<int> interfaceNodes(elementData.nodalConnectivity().begin(),
                                                         elementData.nodalConnectivity().end());

                            interfaceElementMap.emplace(ownership, interfaceNodes);
                        }
                    }
                }
            }
        }
    }
    gmshFile.close();

    auto const end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Mesh data structure filled in " << elapsed_seconds.count() << "s\n";
}

int Reader::mapElementData(int const elementTypeId)
{
    // Return the number of local nodes per element
    switch (elementTypeId)
    {
        case LINE2: return 2; break;
        case TRIANGLE3: return 3; break;
        case QUADRILATERAL4: return 4; break;
        case TETRAHEDRON4: return 4; break;
        case HEXAHEDRON8: return 8; break;
        case PRISM6: return 6; break;
        case PYRAMID5: return 5; break;
        case LINE3: return 3; break;
        case TRIANGLE6: return 6; break;
        case QUADRILATERAL9: return 9; break;
        case TETRAHEDRON10: return 10; break;
        case HEXAHEDRON27: return 27; break;
        case PRISM18: return 18; break;
        case PYRAMID14: return 14; break;
        case POINT: return 1; break;
        case QUADRILATERAL8: return 8; break;
        case HEXAHEDRON20: return 20; break;
        case PRISM15: return 15; break;
        case PYRAMID13: return 13; break;
        case TRIANGLE9: return 19; break;
        case TRIANGLE10: return 10; break;
        case TRIANGLE12: return 12; break;
        case TRIANGLE15: return 15; break;
        case TRIANGLE15_IC: return 15; break;
        case TRIANGLE21: return 21; break;
        case EDGE4: return 4; break;
        case EDGE5: return 5; break;
        case EDGE6: return 6; break;
        case TETRAHEDRON20: return 20; break;
        case TETRAHEDRON35: return 35; break;
        case TETRAHEDRON56: return 56; break;
        case HEXAHEDRON64: return 64; break;
        case HEXAHEDRON125: return 125; break;
        default:
            throw GmshReaderException("The elementTypeId " + std::to_string(elementTypeId) +
                                      " is not implemented");
    }
    return -1;
}

void Reader::checkSupportedGmsh(float const gmshVersion)
{
    if (gmshVersion < 2.2)
    {
        throw std::runtime_error("GmshVersion " + std::to_string(gmshVersion) +
                                 " is not supported");
    }
}

void Reader::writeMeshToJson(bool const printIndices) const
{
    for (auto partition = 0; partition < number_of_partitions; ++partition)
    {
        std::map<StringKey, Value> localProcessMesh;

        Mesh process_mesh;

        for (auto const& mesh : gmshMesh)
        {
            for (auto const& element : mesh.second)
            {
                if (element.isOwnedByProcess(partition + 1))
                {
                    // Build a local copy of the elements to be offset later
                    localProcessMesh[mesh.first].push_back(element);
                }
            }
        }

        // Find all of the elements which belong to this process
        for (auto const& mesh : meshes)
        {
            auto const mesh_key = mesh.first;
            std::cout << "Searching " << mesh_key.first << std::endl;
            for (auto const& element : mesh.second)
            {
                if (element.isOwnedByProcess(partition + 1))
                {
                    // Copy the elements into the process mesh
                    process_mesh[mesh.first].push_back(element);
                }
            }
        }

        auto localToGlobalMapping = fillLocalToGlobalMap(localProcessMesh);
        auto local_global_mapping = fillLocalToGlobalMap(process_mesh);

        auto localNodes = fillLocalNodeList(localToGlobalMapping);

        if (useLocalNodalConnectivity)
        {
            reorderLocalMesh(localProcessMesh, localToGlobalMapping);
        }

        // Sort the elements based on the elementTypeId to output grouped
        // meshes for contiguous storage
        for (auto& mesh : localProcessMesh)
        {
            boost::sort(mesh.second,
                        [](auto const& a, auto const& b) { return a.typeId() < b.typeId(); });
        }

        // Check if this local mesh needs to be converted to zero based indexing
        // then correct the nodal connectivities, the mappings and the nodal and
        // element ids of the data structures
        if (useZeroBasedIndexing)
        {
            for (auto& l2g : localToGlobalMapping) --l2g;
            for (auto& localNode : localNodes) --localNode.id;
            for (auto& mesh : localProcessMesh)
            {
                for (auto& element : mesh.second)
                {
                    element.convertToZeroBasedIndexing();
                }
            }
        }

        writeInJsonFormat(localProcessMesh,
                          localToGlobalMapping,
                          localNodes,
                          partition,
                          number_of_partitions > 1,
                          printIndices);

        std::cout << std::string(2, ' ') << "Finished writing out JSON file for mesh partition "
                  << partition << "\n"
                  << std::flush;
    }
}

std::vector<int>
Reader::fillLocalToGlobalMap(std::map<StringKey, Value> const& localProcessMesh) const
{
    std::vector<int> localToGlobalMapping;

    for (auto const& mesh : localProcessMesh)
    {
        for (auto const& element : mesh.second)
        {
            boost::copy(element.nodalConnectivity(), std::back_inserter(localToGlobalMapping));
        }
    }

    // Sort and remove duplicates
    boost::sort(localToGlobalMapping);
    localToGlobalMapping.erase(std::unique(localToGlobalMapping.begin(),
                                           localToGlobalMapping.end()),
                               localToGlobalMapping.end());
    return localToGlobalMapping;
}

std::vector<int> fillLocalToGlobalMap(Mesh const& process_mesh) const
{
    std::vector<int> localToGlobalMapping;

    for (auto const& mesh : process_mesh)
    {
        for (auto const& element : mesh.second)
        {
            boost::copy(element.nodalConnectivity(), std::back_inserter(localToGlobalMapping));
        }
    }

    // Sort and remove duplicates
    boost::sort(localToGlobalMapping);
    localToGlobalMapping.erase(std::unique(localToGlobalMapping.begin(),
                                           localToGlobalMapping.end()),
                               localToGlobalMapping.end());
    return localToGlobalMapping;
}

void Reader::reorderLocalMesh(std::map<StringKey, Value>& localProcessMesh,
                              std::vector<int> const& localToGlobalMapping) const
{
    for (auto& mesh : localProcessMesh)
    {
        for (auto& element : mesh.second)
        {
            for (auto& node : element.nodalConnectivity())
            {
                auto found = boost::lower_bound(localToGlobalMapping, node);

                // Reset the node value to that inside the local ordering with
                // the default of one based ordering
                node = std::distance(localToGlobalMapping.begin(), found) + 1;
            }
        }
    }
}

std::vector<NodeData> Reader::fillLocalNodeList(std::vector<int> const& localToGlobalMapping) const
{
    std::vector<NodeData> localNodeList;
    for (auto const& map : localToGlobalMapping)
    {
        localNodeList.push_back(nodeList[map - 1]);
    }
    return localNodeList;
}

void Reader::writeInJsonFormat(std::map<StringKey, Value> const& localProcessMesh,
                               std::vector<int> const& localToGlobalMapping,
                               std::vector<NodeData> const& nodalCoordinates,
                               int const processId,
                               bool const isMeshDistributed,
                               bool const printIndices) const
{
    // Write out each file to Json format
    Json::Value event;

    std::string filename = fileName.substr(0, fileName.find_last_of(".")) + ".mesh";

    if (isMeshDistributed) filename += std::to_string(processId);

    std::fstream writer;
    writer.open(filename, std::ios::out);

    // Write out the nodal coordinates
    Json::Value nodeGroup;
    for (auto const& node : nodalCoordinates)
    {
        Json::Value coordinates(Json::arrayValue);
        for (auto const& xyz : node.coordinates)
        {
            coordinates.append(Json::Value(xyz));
        }
        nodeGroup["Coordinates"].append(coordinates);
        if (printIndices) nodeGroup["Indices"].append(node.id);
    }
    event["Nodes"].append(nodeGroup);

    for (auto const& mesh : localProcessMesh)
    {
        // Find all of the unique elementTypeIds
        std::set<int> elementTypeIds;
        for (auto const& element : mesh.second)
        {
            elementTypeIds.insert(element.typeId());
        }

        // Add in each of the element type groups into the json object
        for (auto const& elementTypeId : elementTypeIds)
        {
            Json::Value elementGroup;

            // Find the elements which are of the element type in the local mesh
            auto lower = boost::lower_bound(mesh.second, elementTypeId, [](auto a, auto b) {
                return a.typeId() < b;
            });
            auto upper = boost::upper_bound(mesh.second, elementTypeId, [](auto a, auto b) {
                return a < b.typeId();
            });

            std::for_each(lower, upper, [&](auto const& element) {
                Json::Value connectivity(Json::arrayValue);
                for (auto const& node : element.nodalConnectivity())
                {
                    connectivity.append(node);
                }
                elementGroup["NodalConnectivity"].append(connectivity);
                if (printIndices) elementGroup["Indices"].append(element.id());
            });

            elementGroup["Name"] = mesh.first;
            elementGroup["Type"] = elementTypeId;

            event["Elements"].append(elementGroup);
        }
    }

    if (isMeshDistributed)
    {
        for (auto const& l2g : localToGlobalMapping)
        {
            event["LocalToGlobalMap"].append(l2g);
        }

        int globalStartId = 0;

        for (auto const& interface : interfaceElementMap)
        {
            const auto masterId = interface.first.first;
            const auto slaveId  = interface.first.second;

            if (masterId < slaveId)
            {
                std::set<int> intersection;
                auto const& v1 = interface.second;
                auto const& v2 = interfaceElementMap.at(std::pair<int, int>(slaveId, masterId));

                std::set_intersection(v1.begin(),
                                      v1.end(),
                                      v2.begin(),
                                      v2.end(),
                                      std::inserter(intersection, intersection.begin()));

                if ((processId == masterId - 1 or processId == slaveId - 1))
                {
                    Json::Value interfaceGroup, nodeIds, globalIds;

                    for (auto const& nodeId : intersection)
                    {
                        nodeIds.append(nodeId);
                    }
                    interfaceGroup["Master"].append(interface.first.first);
                    interfaceGroup["Value"].append(processId == masterId - 1 ? 1 : -1);
                    interfaceGroup["Slave"].append(interface.first.second);
                    interfaceGroup["NodeIds"].append(nodeIds);
                    interfaceGroup["GlobalStartId"].append(globalStartId);

                    event["Interface"].append(interfaceGroup);
                }
                globalStartId += intersection.size();
            }
        }
        event["NumInterfaceNodes"].append(globalStartId);
    }
    Json::StyledWriter jsonwriter;
    writer << jsonwriter.write(event);
    writer.close();
}
}
