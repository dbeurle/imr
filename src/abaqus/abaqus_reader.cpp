
#include "abaqus/abaqus_reader.hpp"

#include "node.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include <json/json.h>

namespace imr
{
abaqus_reader::abaqus_reader(std::string const& filename) : mesh_reader(filename) {}

void abaqus_reader::parse()
{
    auto const start = std::chrono::steady_clock::now();

    std::ifstream input(filename.c_str(), std::ios::in);

    if (!input)
    {
        throw std::domain_error("Cannot open file");
    }

    std::string line;

    while (input)
    {
        // extract the keyword from the
        std::string const keyword = abq::is_keyword(line) ? line.find(',') != std::string::npos
                                                                ? line.substr(1, line.find(',') - 1)
                                                                : line.substr(1, line.size())
                                                          : "";

        if (keyword == "Node")
        {
            std::cout << "reading nodes..." << std::flush;

            std::getline(input, line);

            while (abq::is_not_comment(line) && abq::is_not_keyword(line))
            {
                abq::remove_commas(line);

                // Store node information in matrix
                std::istringstream input_stream(line);

                basic_node node;

                input_stream >> node.index;

                for (auto& coordinate : node.coordinates)
                {
                    input_stream >> coordinate;
                }

                m_nodes.emplace_back(node);

                std::getline(input, line);
            }
            std::cout << "done!" << std::endl;
        }
        else if (keyword == "Element")
        {
            std::cout << "reading elements..." << std::flush;

            std::getline(input, line);

            while (abq::is_not_comment(line))
            {
                abq::remove_commas(line);

                // Store element information in matrix
                std::istringstream line_stream(line);

                std::vector<std::int64_t> node_indices;
                node_indices.reserve(20);

                std::int64_t index;
                line_stream >> index;

                std::copy(std::istream_iterator<int>(line_stream),
                          std::istream_iterator<int>(),
                          std::back_insert_iterator<decltype(node_indices)>(node_indices));

                m_elements.emplace_back(index, node_indices);

                std::getline(input, line);
            }
            std::cout << "done!" << std::endl;
        }
        else
        {
            std::getline(input, line);
        }
    }
    auto const end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << std::string(2, ' ') << "Parsing took " << elapsed_seconds.count() << "s\n";
}

void abaqus_reader::write() const
{
    auto const start = std::chrono::steady_clock::now();

    // Change suffix
    std::string output_name = filename;

    output_name.replace(filename.find("inp"), 3, "mesh");

    std::ofstream output(output_name.c_str(), std::ios::out);

    output << "Nodes"
           << "\n"
           << "   " << m_nodes.size() << "\n";

    output << "Elements"
           << "\n"
           << "   " << m_elements.size() << "\n";

    output << "Node\n";

    for (auto const& node : m_nodes)
    {
        for (auto const& coordinate : node.coordinates)
        {
            output << "  " << std::setw(13) << coordinate;
        }
        output << "\n";
    }
    output << "\n";

    output << "Element\n";

    for (auto const& element : m_elements)
    {
        for (auto const& index : element.node_indices())
        {
            output << "  " << std::setw(6) << index;
        }
        output << "\n";
    }
    output.close();

    auto const end = std::chrono::steady_clock::now();

    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << std::string(2, ' ') << "Writing took " << elapsed_seconds.count() << "s\n";
}
} // namespace imr
