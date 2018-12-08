
#include "gmsh_reader.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>

#include <json/json.h>

namespace imr
{
gmsh_reader::gmsh_reader(std::string const& filename,
                         nodal_order const ordering,
                         index_base const base,
                         distributed const distributed_option)
    : mesh_reader(filename),
      input_file_name(filename),
      useZeroBasedIndexing(base == index_base::zero),
      useLocalNodalConnectivity(ordering == nodal_order::local),
      is_feti_format(distributed_option == distributed::feti)
{
    this->parse();
}

void gmsh_reader::parse()
{
    auto const start = std::chrono::steady_clock::now();

    std::ifstream gmsh_file(input_file_name);

    if (!gmsh_file.is_open())
    {
        throw std::domain_error("Input file " + input_file_name + " was not able to be opened");
    }

    std::string token;

    // Loop around file and read in keyword tokens
    while (!gmsh_file.eof())
    {
        gmsh_file >> token;

        if (token == "$MeshFormat")
        {
            std::string null;

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

            for (std::int32_t i = 0; i < physicalIds; ++i)
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
                gmsh_file >> node.index >> node.coordinates[0] >> node.coordinates[1] >>
                    node.coordinates[2];
            }
        }
        else if (token == "$Elements")
        {
            std::int64_t element_count;
            gmsh_file >> element_count;

            for (std::int64_t count = 0; count < element_count; count++)
            {
                std::int64_t element_index = 0;

                int tags_size = 0, element_type = 0;

                gmsh_file >> element_index >> element_type >> tags_size;

                std::vector<std::int32_t> tags(tags_size, 0);
                std::vector<std::int64_t> node_indices(nodes_per_element(element_type), 0);

                for (auto& tag : tags)
                {
                    gmsh_file >> tag;
                }

                for (auto& node_index : node_indices)
                {
                    gmsh_file >> node_index;
                }

                auto const physical_index = tags[0];

                element element_data(std::move(node_indices), tags, element_type, element_index);

                // Update the total number of partitions on the fly
                m_partitions = std::max(element_data.maxProcessId(), m_partitions);

                if (element_data.is_shared())
                {
                    for (int i = 4; i < tags[2] + 3; ++i)
                    {
                        auto const owner_sharer = std::make_pair(tags[3], std::abs(tags[i]));

                        auto const& indices = element_data.node_indices();

                        interfaceElementMap[owner_sharer].insert(std::begin(indices),
                                                                 std::end(indices));
                    }
                }
                // Copy the element data into the mesh structure
                meshes[{physicalGroupMap[physical_index], element_type}].emplace_back(element_data);
            }
        }
    }
    std::cout << std::string(2, ' ') << "A total number of " << m_partitions
              << " partitions were found\n";

    auto const end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Mesh data structure filled in " << elapsed_seconds.count() << "s\n";
}

void gmsh_reader::checkSupportedGmsh(float const gmshVersion)
{
    if (gmshVersion < 2.2f || gmshVersion > 2.25f)
    {
        throw std::runtime_error("GmshVersion " + std::to_string(gmshVersion) +
                                 " is not supported");
    }
}

void gmsh_reader::write() const
{
    bool const print_indices = false;

    for (std::int32_t partition = 0; partition < m_partitions; ++partition)
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
            std::transform(begin(local_global_mapping),
                           end(local_global_mapping),
                           begin(local_global_mapping),
                           [](auto const value) { return value - 1; });

            for (auto& localNode : local_nodes)
            {
                --localNode.index;
            }

            for (auto& mesh : process_mesh)
            {
                for (auto& element : mesh.second)
                {
                    element.convertToZeroBasedIndexing();
                }
            }
        }

        write_json(process_mesh,
                   local_global_mapping,
                   local_nodes,
                   partition,
                   m_partitions > 1,
                   print_indices);

        std::cout << std::string(2, ' ') << "Finished writing out JSON file for mesh partition "
                  << partition << "\n";
    }
}

std::vector<std::int64_t> gmsh_reader::fillLocalToGlobalMap(Mesh const& process_mesh) const
{
    std::vector<std::int64_t> local_global_mapping;

    for (auto const& mesh : process_mesh)
    {
        for (auto const& element : mesh.second)
        {
            auto const& nodes = element.node_indices();
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

void gmsh_reader::reorderLocalMesh(Mesh& process_mesh,
                                   std::vector<std::int64_t> const& local_global_mapping) const
{
    for (auto& mesh : process_mesh)
    {
        for (auto& element : mesh.second)
        {
            for (auto& node : element.node_indices())
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

std::vector<node>
gmsh_reader::fillLocalNodeList(std::vector<std::int64_t> const& local_global_mapping) const
{
    std::vector<node> local_nodal_data;
    local_nodal_data.reserve(local_global_mapping.size());

    for (auto const& node_index : local_global_mapping)
    {
        local_nodal_data.emplace_back(nodal_data[node_index - 1]);
    }
    return local_nodal_data;
}

void gmsh_reader::write_json(Mesh const& process_mesh,
                             std::vector<std::int64_t> const& localToGlobalMapping,
                             std::vector<node> const& nodalCoordinates,
                             int const partition_number,
                             bool const is_decomposed,
                             bool const print_indices) const
{
    // Write out each file to Json format
    Json::Value event;

    std::string output_file_name = input_file_name.substr(0, input_file_name.find_last_of('.')) +
                                   ".mesh";

    if (is_decomposed)
    {
        output_file_name += std::to_string(partition_number);
    }

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

        if (print_indices)
        {
            nodeGroup["Indices"].append(node.index);
        }
    }
    event["Nodes"].append(nodeGroup);

    for (auto const& mesh : process_mesh)
    {
        Json::Value elementGroup;
        auto& elementGroupNodalConnectivity = elementGroup["NodalConnectivity"];

        for (auto const& element_data : mesh.second)
        {
            Json::Value connectivity(Json::arrayValue);

            for (auto const& node : element_data.node_indices())
            {
                connectivity.append(node);
            }

            elementGroupNodalConnectivity.append(connectivity);

            if (print_indices)
            {
                elementGroup["Indices"].append(element_data.index());
            }
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

                        interface_group["Master"] = useZeroBasedIndexing ? master_partition - 1
                                                                         : master_partition;

                        interface_group["Slave"] = useZeroBasedIndexing ? slave_partition - 1
                                                                        : slave_partition;

                        interface_group["Value"] = partition_number == master_partition - 1 ? 1
                                                                                            : -1;

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

                    interface_group["Process"] = useZeroBasedIndexing ? master_partition - 1
                                                                      : master_partition;

                    event["Interface"].append(interface_group);
                }
            }
        }
    }
    Json::StyledWriter jsonwriter;
    writer << jsonwriter.write(event);
    writer.close();
}
} // namespace imr
