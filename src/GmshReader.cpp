/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"

#include <numeric>
#include <algorithm>
#include <boost/range/algorithm.hpp>
#include <boost/range/numeric.hpp>
#include <boost/container/flat_set.hpp>

#include <iostream>
#include <iomanip>
#include <json/json.h>
#include "GmshReaderException.hpp"

namespace gmsh
{
Reader::Reader(const std::string& fileName, bool useZeroBasedIndexing):
    fileName(fileName),
    useZeroBasedIndexing(useZeroBasedIndexing)
{
    parse();
}

void Reader::parse()
{
    gmshFile.precision(sizeof(double));
    fillMesh();
}

void Reader::fillMesh()
{
    gmshFile.open(fileName.c_str());

    if (!gmshFile.is_open())
        throw GmshReaderException("Filename " + fileName + " is not valid");

    std::string token, null;

    std::vector<std::string> physicalNames;

    // Loop around file and read in keyword tokens
    while (!gmshFile.eof())
    {
        gmshFile >> token;

        if (token == "$MeshFormat")
        {
            float gmshVersion;  // File format version
            short dataType;     // Precision

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
                int dimension  = 0;
                int physicalId = 0;
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
                gmshFile >> node.id >> node.coordinates[0] >> node.coordinates[1] >> node.coordinates[2];

        }
        else if (token == "$Elements")
        {
            int elementIds = 0;
            gmshFile >> elementIds;
            for (int elementId = 0; elementId < elementIds; elementId++)
            {
                int id            = 0;
                int numberOfTags  = 0;
                int elementTypeId = 0;

                gmshFile >> id >> elementTypeId >> numberOfTags;

                int numberOfNodes = mapElementData(elementTypeId);

                ElementData elementData(numberOfNodes,
                                        numberOfTags,
                                        elementTypeId,
                                        id);

                for (auto& tag : elementData.tags)
                {
                    gmshFile >> tag;
                }
                for (auto& nodeId : elementData.nodalConnectivity)
                {
                    gmshFile >> nodeId;
                }
                int physicalId = elementData.tags[0];
                gmshMesh[physicalGroupMap[physicalId]].push_back(elementData);

                if (elementData.isSharedByMultipleProcesses())
                {
                    for (int i = 4; i < elementData.tags[2] + 3; ++i)
                    {
                        std::pair<int,int> ownership(elementData.tags[3], -elementData.tags[i]);
                        if (interfaceElementMap.find(ownership) != interfaceElementMap.end())
                        {
                            for (auto const& nodeId : elementData.nodalConnectivity)
                                interfaceElementMap[ownership].emplace(nodeId);

                        }
                        else
                        {
                            std::set<int> interfaceNodes(elementData.nodalConnectivity.begin(),
                                                         elementData.nodalConnectivity.end());

                            interfaceElementMap.emplace(ownership, interfaceNodes);
                        }
                    }
                }
            }
        }
    }
    gmshFile.close();
}

int Reader::processIdsDecomposedMesh() const
{
    return boost::accumulate(gmshMesh, 0, [](auto i, auto const& mesh)
           {
               return std::max(i, boost::accumulate(mesh.second, 0, [](auto j, auto const& data)
                                  {
                                      return std::max(j, data.maxProcessId());
                                  }));
           });
}

int Reader::mapElementData(int elementTypeId)
{
    int lnodes = 0;
	switch (elementTypeId)
	{
        case LINE2:          lnodes = 2;  break;
        case TRIANGLE3:      lnodes = 3;  break;
        case QUADRILATERAL4: lnodes = 4;  break;
        case TETRAHEDRON4:   lnodes = 4;  break;
        case HEXAHEDRON8:    lnodes = 8;  break;
        case PRISM6:         lnodes = 6;  break;
        case PYRAMID5:       lnodes = 5;  break;
        case LINE3:          lnodes = 3;  break;
        case TRIANGLE6:      lnodes = 6;  break;
        case QUADRILATERAL9: lnodes = 9;  break;
        case TETRAHEDRON10:  lnodes = 10; break;
        case HEXAHEDRON27:   lnodes = 27; break;
        case PRISM18:        lnodes = 18; break;
        case PYRAMID14:      lnodes = 14; break;
        case POINT:          lnodes = 1;  break;
        case QUADRILATERAL8: lnodes = 8;  break;
        case HEXAHEDRON20:   lnodes = 20; break;
        case PRISM15:        lnodes = 15; break;
        case PYRAMID13:      lnodes = 13; break;
        case TRIANGLE9:      lnodes = 19; break;
        case TRIANGLE10:     lnodes = 10; break;
        case TRIANGLE12:     lnodes = 12; break;
        case TRIANGLE15:     lnodes = 15; break;
        case TRIANGLE15_IC:  lnodes = 15; break;
        case TRIANGLE21:     lnodes = 21; break;
        case EDGE4:          lnodes = 4;  break;
        case EDGE5:          lnodes = 5;  break;
        case EDGE6:          lnodes = 6;  break;
        case TETRAHEDRON20:  lnodes = 20; break;
        case TETRAHEDRON35:  lnodes = 35; break;
        case TETRAHEDRON56:  lnodes = 56; break;
        case HEXAHEDRON64:   lnodes = 64; break;
        case HEXAHEDRON125:  lnodes = 125;break;
	default:
		throw GmshReaderException("The elementTypeId "
                                  + std::to_string(elementTypeId)
                                  + " is not implemented");
	}
    return lnodes;
}

void Reader::checkSupportedGmsh(float gmshVersion)
{
    if (gmshVersion < 2.2)
        throw GmshReaderException("GmshVersion " + std::to_string(gmshVersion)
                                  + " is not supported");
}

void Reader::writeMesh(const std::string& outputFileName)
{
    std::ofstream file(outputFileName);

    if(not file.is_open())
        throw GmshReaderException("Failed to open " + outputFileName);

    int numElements = boost::accumulate(gmshMesh, 0, [](auto a, auto const& mesh)
                      {return a + mesh.second.size();});

    file << "numNodes    \t" << nodeList.size()     << "\n";
    file << "numElements \t" << numElements         << "\n";

    file << "Nodes \n";
    for (const auto& node : nodeList)
    {
        file << std::setw(10) << std::right << node.id << "\t";
        file << std::setw(10) << std::right << node.coordinates[0] << "\t";
        file << std::setw(10) << std::right << node.coordinates[1] << "\t";
        file << std::setw(10) << std::right << node.coordinates[2] << "\n";
    }

    for (const auto& pairNameAndElements : gmshMesh)
    {
        file << "Physical group \t" << pairNameAndElements.first << "\n";
        file << "Elements \n";
        for (const auto& element : pairNameAndElements.second)
        {
            file << std::setw(10) << std::right << element.id << "\t";
            for (const auto& nodeId : element.nodalConnectivity)
                file << std::setw(10) << std::right << nodeId << "\t";

            file << "\n";
        }
    }
    file.close();
}

void Reader::writeMeshToJson() const
{
    // Find the total number of processes associated with the mesh
    int processIds = processIdsDecomposedMesh();

    for (auto processId = 0; processId < processIds; ++processId)
    {
        std::map<StringKey, Value> localProcessMesh;

        for (auto const& mesh : gmshMesh)
        {
            for (auto const& element : mesh.second)
            {
                if (element.isOwnedByProcess(processId+1))
                {
                    // Build a local copy of the elements to be offset later
                    localProcessMesh[mesh.first].push_back(element);
                }
            }
        }
        auto localToGlobalMapping = fillLocalToGlobalMap(localProcessMesh);
        auto localNodes = fillLocalNodeList(localToGlobalMapping);

        // reorderLocalMesh(localProcessMesh, localToGlobalMapping);  <-this messes up the NodalConnectivity

        // Sort the elements based on the elementTypeId to output grouped
        // meshes for contiguous storage in the FEM program
        for (auto& mesh : localProcessMesh)
        {
            boost::sort(mesh.second, [](auto const& a, auto const& b)
            {
                return a.typeId < b.typeId;
            });
        }
        writeInJsonFormat( localProcessMesh,
                           localToGlobalMapping,
                           localNodes,
                           processId,
                           processIds > 1);
    }
}

std::vector<int> Reader::fillLocalToGlobalMap(std::map<StringKey, Value>& localProcessMesh) const
{
    std::vector<int> localToGlobalMapping;
    for (auto& mesh : localProcessMesh)
    {
        for (auto const& element : mesh.second)
        {
            boost::copy(element.nodalConnectivity, std::back_inserter(localToGlobalMapping));
        }
    }
    // Sort and remove duplicates
    boost::sort(localToGlobalMapping);
    localToGlobalMapping.erase( std::unique(localToGlobalMapping.begin(),
                                            localToGlobalMapping.end()),
                                localToGlobalMapping.end());
    return localToGlobalMapping;
}

void Reader::reorderLocalMesh( std::map<StringKey, Value>& localProcessMesh,
                               std::vector<int> const& localToGlobalMapping) const
{
    for (auto& mesh : localProcessMesh)
    {
        for (auto& element : mesh.second)
        {
            for (auto& node : element.nodalConnectivity)
            {
                auto found = boost::lower_bound(localToGlobalMapping, node);
                // Reset the node value to that inside the local ordering
                node = std::distance(localToGlobalMapping.begin(), found);
            }
        }
    }
}

std::vector<NodeData> Reader::fillLocalNodeList(std::vector<int> const& localToGlobalMapping) const
{
    std::vector<NodeData> localNodeList;
    for (auto const& map : localToGlobalMapping)
    {
        localNodeList.push_back(nodeList[map-1]);
    }
    return localNodeList;
}

void Reader::writeInJsonFormat( std::map<StringKey, Value> const& localProcessMesh,
                                std::vector<int> const& localToGlobalMapping,
                                std::vector<NodeData> const& nodalCoordinates,
                                int processId,
                                bool isMeshDistributed) const
{
    // Write out each file to Json format
    Json::Value event;

    std::string filename = fileName.substr(0, fileName.find_last_of("."))
                         + ".mesh";

    if (isMeshDistributed) filename += std::to_string(processId);

    std::fstream writer;
    writer.open(filename, std::ios::out);

    Json::Value nodeGroup;
    for (auto const& node : nodalCoordinates)
    {
        Json::Value coordinates(Json::arrayValue);
        for (auto const& xyz : node.coordinates)
        {
            coordinates.append(Json::Value(xyz));
        }
        nodeGroup["Coordinates"].append(coordinates);
        nodeGroup["Indices"].append(useZeroBasedIndexing ? node.id - 1 : node.id);
    }
    event["Nodes"].append(nodeGroup);

    for (auto const& mesh : localProcessMesh)
    {
        // Find all of the unique elementTypeIds
        boost::container::flat_set<int> elementTypeIds;
        for (auto const& element : mesh.second)
        {
            elementTypeIds.insert(element.typeId);
        }

        for (auto const& elementTypeId : elementTypeIds)
        {
            Json::Value elementGroup;

            auto lower = boost::lower_bound(mesh.second, elementTypeId,
                                            [](auto a, auto b) {return a.typeId < b;});
            auto upper = boost::upper_bound(mesh.second, elementTypeId,
                                            [](auto a, auto b) {return a < b.typeId;});

            std::for_each(lower, upper, [&](auto const& element)
            {
                Json::Value connectivity(Json::arrayValue);
                for (auto const& nodeId : element.nodalConnectivity)
                {
                    connectivity.append(useZeroBasedIndexing ? nodeId - 1 : nodeId);
                }
                elementGroup["NodalConnectivity"].append(connectivity);
                elementGroup["Indices"].append(useZeroBasedIndexing ? element.id - 1 : element.id);
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
            event["LocalToGlobalMap"].append(useZeroBasedIndexing ? l2g - 1 : l2g);
        }
        int globalStartId = 0;
        for (auto const& interface : interfaceElementMap)
        {
            const int masterId    = interface.first.first;
            const int slaveId     = interface.first.second;

            if (masterId < slaveId)
            {
                std::set<int> intersection;
                const std::set<int>& v1 = interface.second;
                const std::set<int>& v2 = interfaceElementMap.at(std::pair<int,int>(slaveId,masterId));

                std::set_intersection(v1.begin(), v1.end(),
                                      v2.begin(), v2.end(),
                                      std::inserter(intersection, intersection.begin()));

                if ((processId == masterId - 1 or processId == slaveId - 1))
                {
                    Json::Value interfaceGroup;
                    Json::Value nodeIds;
                    Json::Value globalIds;
                    for (auto const& nodeId : intersection)
                    {
                        nodeIds.append(nodeId);
                    }
                    interfaceGroup["Master"].append(interface.first.first);
                    interfaceGroup["Value"].append(processId == masterId -1 ? 1 : -1);
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
