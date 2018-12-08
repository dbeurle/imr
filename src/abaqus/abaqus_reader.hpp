
#pragma once

#include "mesh_reader.hpp"

#include "element.hpp"
#include "element_types.hpp"
#include "node.hpp"

#include <cstdint>
#include <string>
#include <vector>

/// \file abaqus_reader.hpp

namespace imr
{
namespace abq
{
/// Convert the ABAQUS element type to an integer
auto convert_element_type(std::string const& element_type) -> ELEMENT_TYPE_ID;

/// Delete commas in the string
inline void remove_commas(std::string& line)
{
    while (line.find(",", 0) != std::string::npos)
    {
        line.replace(line.find(","), 1, " ");
    }
}

inline bool is_comment(std::string const& line) { return line.substr(0, 2) == "**"; }

inline bool is_not_comment(std::string const& line) { return !is_comment(line); }

inline bool is_keyword(std::string const& line)
{
    return line.substr(0, 1) == "*" && is_not_comment(line);
}

inline bool is_not_keyword(std::string const& line) { return !is_keyword(line); }
} // namespace abq

/// abaqus_reader is a parser of the ABAQUS mesh format and attempts to sort these
/// into nodal sets and element sets for printing to a JSON mesh format
class abaqus_reader : public mesh_reader
{
public:
    abaqus_reader(std::string const& filename);

    virtual void parse() override final;

    virtual void write() const override final;

protected:
    std::vector<basic_node> m_nodes;

    std::vector<basic_element> m_elements;
};
} // namespace imr
