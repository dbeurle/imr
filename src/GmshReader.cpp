/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"
#include "GmshReaderException.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>

#include <json/json.h>

namespace gmsh
{
Reader::Reader(std::string const& input_file_name,
               NodalOrdering const ordering,
               IndexingBase const base,
               distributed const distributed_option)
    : input_file_name(input_file_name),
      useZeroBasedIndexing(base == IndexingBase::Zero),
      useLocalNodalConnectivity(ordering == NodalOrdering::Local),
      is_feti_format(distributed_option == distributed::feti)
{
    fillMesh();
}

void Reader::fillMesh()
{
    auto const start = std::chrono::high_resolution_clock::now();

    std::fstream gmsh_file(input_file_name);

    if (!gmsh_file.is_open())
    {
        throw GmshReaderException("Input file " + input_file_name + " was not able to be opened");
    }

    std::string token, null;

    // Loop around file and read in keyword tokens
    while (!gmsh_file.eof())
    {
        gmsh_file >> token;

        if (token == "$MeshFormat")
        {
            float gmshVersion;     // File format version
            std::int32_t dataType; // Precision

            gmsh_file >> gmshVersion >> dataType >> null;
            checkSupportedGmsh(gmshVersion);
        }
        else if (token == "$PhysicalNames")
        {
            std::string physical_name;

            std::int32_t physicalIds;
            gmsh_file >> physicalIds;

            for (auto i = 0; i < physicalIds; ++i)
            {
                std::int32_t dimension, physicalId;
                gmsh_file >> dimension >> physicalId >> physical_name;

                // Extract the name from the quotes
                physical_name.erase(std::remove(physical_name.begin(), physical_name.end(), '\"'),
                                    physical_name.end());
                physicalGroupMap.emplace(physicalId, physical_name);
            }
            token.clear();
            gmsh_file >> token;
        }
        else if (token == "$Nodes")
        {
            std::int64_t number_of_nodes;
            gmsh_file >> number_of_nodes;
            nodal_data.resize(number_of_nodes);

            for (auto& node : nodal_data)
            {
                gmsh_file >> node.id >> node.coordinates[0] >> node.coordinates[1] >>
                    node.coordinates[2];
            }
        }
        else if (token == "$Elements")
        {
            int elementIds;
            gmsh_file >> elementIds;

            for (int elementId = 0; elementId < elementIds; elementId++)
            {
                int id = 0, numberOfTags = 0, elementTypeId = 0;

                gmsh_file >> id >> elementTypeId >> numberOfTags;

                auto const numberOfNodes = mapElementData(elementTypeId);

                list tags(numberOfTags, 0);
                list nodalConnectivity(numberOfNodes, 0);

                for (auto& tag : tags)
                {
                    gmsh_file >> tag;
                }
                for (auto& nodeId : nodalConnectivity)
                {
                    gmsh_file >> nodeId;
                }

                auto const physicalId = tags[0];

                ElementData elementData(std::move(nodalConnectivity), tags, elementTypeId, id);

                // Update the total number of partitions on the fly
                number_of_partitions = std::max(elementData.maxProcessId(), number_of_partitions);

                // Copy the element data into the mesh structure
                meshes[{physicalGroupMap[physicalId], elementTypeId}].push_back(elementData);

                if (elementData.isSharedByMultipleProcesses())
                {
                    for (int i = 4; i < tags[2] + 3; ++i)
                    {
                        auto const owner_sharer = std::make_pair(tags[3], std::abs(tags[i]));

                        auto const& connectivity = elementData.nodalConnectivity();

                        interfaceElementMap[owner_sharer].insert(std::begin(connectivity),
                                                                 std::end(connectivity));
                    }
                }
            }
        }
    }
    std::cout << std::string(2, ' ') << "A total number of " << number_of_partitions
              << " partitions was found\n";

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

void Reader::writeMeshToJson(bool const print_indices) const
{
    for (auto partition = 0; partition < number_of_partitions; ++partition)
    {
        Mesh process_mesh;

        // Find all of the elements which belong to this process
        for (auto const& mesh : meshes)
        {
            // Copy the elements into the process mesh
            for (auto const& element : mesh.second)
            {
                if (element.isOwnedByProcess(partition + 1))
                {
                    process_mesh[mesh.first].push_back(element);
                }
            }
        }

        auto local_global_mapping = fillLocalToGlobalMap(process_mesh);

        auto local_nodes = fillLocalNodeList(local_global_mapping);

        if (useLocalNodalConnectivity)
        {
            reorderLocalMesh(process_mesh, local_global_mapping);
        }

        // Check if this local mesh needs to be converted to zero based indexing
        // then correct the nodal connectivities, the mappings and the nodal and
        // element ids of the data structures
        if (useZeroBasedIndexing)
        {
            for (auto& l2g : local_global_mapping) --l2g;

            for (auto& localNode : local_nodes) --localNode.id;

            for (auto& mesh : process_mesh)
            {
                for (auto& element : mesh.second)
                {
                    element.convertToZeroBasedIndexing();
                }
            }
        }

        writeInJsonFormat(process_mesh,
                          local_global_mapping,
                          local_nodes,
                          partition,
                          number_of_partitions > 1,
                          print_indices);

        std::cout << std::string(2, ' ') << "Finished writing out JSON file for mesh partition "
                  << partition << "\n";
    }
}

list Reader::fillLocalToGlobalMap(Mesh const& process_mesh) const
{
    list local_global_mapping;

    for (auto const& mesh : process_mesh)
    {
        for (auto const& element : mesh.second)
        {
            auto const& nodes = element.nodalConnectivity();
            std::copy(std::begin(nodes), std::end(nodes), std::back_inserter(local_global_mapping));
        }
    }

    // Sort and remove duplicates
    std::sort(std::begin(local_global_mapping), std::end(local_global_mapping));

    local_global_mapping.erase(std::unique(std::begin(local_global_mapping),
                                           std::end(local_global_mapping)),
                               std::end(local_global_mapping));
    return local_global_mapping;
}

void Reader::reorderLocalMesh(Mesh& process_mesh, list const& local_global_mapping) const
{
    for (auto& mesh : process_mesh)
    {
        for (auto& element : mesh.second)
        {
            for (auto& node : element.nodalConnectivity())
            {
                auto const found = std::lower_bound(std::begin(local_global_mapping),
                                                    std::end(local_global_mapping),
                                                    node);

                // Reset the node value to that inside the local ordering with
                // the default of one based ordering
                node = std::distance(local_global_mapping.begin(), found) + 1;
            }
        }
    }
}

std::vector<NodeData> Reader::fillLocalNodeList(list const& local_global_mapping) const
{
    std::vector<NodeData> local_nodal_data;
    local_nodal_data.reserve(local_global_mapping.size());

    for (auto const& node_index : local_global_mapping)
    {
        local_nodal_data.emplace_back(nodal_data[node_index - 1]);
    }
    return local_nodal_data;
}

void Reader::writeInJsonFormat(Mesh const& process_mesh,
                               list const& localToGlobalMapping,
                               std::vector<NodeData> const& nodalCoordinates,
                               int const partition_number,
                               bool const is_decomposed,
                               bool const print_indices) const
{
    // Write out each file to Json format
    Json::Value event;

    std::string output_file_name =
        input_file_name.substr(0, input_file_name.find_last_of('.')) + ".mesh";

    if (is_decomposed) output_file_name += std::to_string(partition_number);

    std::fstream writer;
    writer.open(output_file_name, std::ios::out);

    // Write out the nodal coordinates
    Json::Value nodeGroup;
    auto& nodeGroupCoordinates = nodeGroup["Coordinates"];

    for (auto const& node : nodalCoordinates)
    {
        Json::Value coordinates(Json::arrayValue);
        for (auto const& xyz : node.coordinates)
        {
            coordinates.append(Json::Value(xyz));
        }
        nodeGroupCoordinates.append(coordinates);

        if (print_indices) nodeGroup["Indices"].append(node.id);
    }
    event["Nodes"].append(nodeGroup);

    for (auto const& mesh : process_mesh)
    {
        Json::Value elementGroup;
        auto& elementGroupNodalConnectivity = elementGroup["NodalConnectivity"];

        for (auto const& element_data : mesh.second)
        {
            Json::Value connectivity(Json::arrayValue);

            for (auto const& node : element_data.nodalConnectivity())
            {
                connectivity.append(node);
            }

            elementGroupNodalConnectivity.append(connectivity);

            if (print_indices) elementGroup["Indices"].append(element_data.id());
        }

        elementGroup["Name"] = mesh.first.first;
        elementGroup["Type"] = mesh.first.second;

        event["Elements"].append(elementGroup);
    }

    if (is_decomposed)
    {
        auto& eventLocalToGlobalMap = event["LocalToGlobalMap"];
        for (auto const& l2g : localToGlobalMapping)
        {
            eventLocalToGlobalMap.append(l2g);
        }

        if (is_feti_format)
        {
            long globalStartId{0l};

            for (auto const& interface : interfaceElementMap)
            {
                auto const master_partition = interface.first.first;
                auto const slave_partition  = interface.first.second;

                if (master_partition < slave_partition)
                {
                    std::set<std::int64_t> intersection;

                    auto const& v1 = interface.second;
                    auto const& v2 = interfaceElementMap.at({slave_partition, master_partition});

                    std::set_intersection(std::begin(v1),
                                          std::end(v1),
                                          std::begin(v2),
                                          std::end(v2),
                                          std::inserter(intersection, std::begin(intersection)));

                    if (partition_number == master_partition - 1 or
                        partition_number == slave_partition - 1)
                    {
                        Json::Value interface_group, nodal_numbers;

                        for (auto const& node_number : intersection)
                            nodal_numbers.append(node_number);

                        interface_group["NodeIds"].append(nodal_numbers);

                        interface_group["Master"] =
                            useZeroBasedIndexing ? master_partition - 1 : master_partition;

                        interface_group["Slave"] =
                            useZeroBasedIndexing ? slave_partition - 1 : slave_partition;

                        interface_group["Value"] =
                            partition_number == master_partition - 1 ? 1 : -1;

                        interface_group["GlobalStartId"] = globalStartId;

                        event["Interface"].append(interface_group);
                    }
                    globalStartId += intersection.size();
                }
            }
            event["NumInterfaceNodes"] = globalStartId;
        }
        else
        {
            for (auto const& interface : interfaceElementMap)
            {
                auto const master_partition = interface.first.first;
                auto const slave_partition  = interface.first.second;

                if (partition_number == slave_partition - 1)
                {
                    std::set<std::int64_t> intersection;

                    // Find the common indices between the master and the slave
                    // partition and print these out for each process interface
                    auto const& v1 = interface.second;
                    auto const& v2 = interfaceElementMap.at({slave_partition, master_partition});

                    std::set_intersection(std::begin(v1),
                                          std::end(v1),
                                          std::begin(v2),
                                          std::end(v2),
                                          std::inserter(intersection, std::begin(intersection)));

                    Json::Value interface_group, nodal_numbers;

                    for (auto const& node_number : intersection)
                    {
                        nodal_numbers.append(useZeroBasedIndexing ? node_number - 1 : node_number);
                    }
                    interface_group["Indices"] = nodal_numbers;

                    interface_group["Process"] =
                        useZeroBasedIndexing ? master_partition - 1 : master_partition;

                    event["Interface"].append(interface_group);
                }
            }
        }
    }
    Json::StyledWriter jsonwriter;
    writer << jsonwriter.write(event);
    writer.close();
}
}
