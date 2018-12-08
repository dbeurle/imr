
#pragma once

#include <array>
#include <cstdint>

namespace imr
{
/// basic_node represents the geometry for a single node in three-dimensional space
struct node
{
    std::int64_t index;
    std::array<double, 3> coordinates;
};

/// basic_node represents the geometry for a single node in three-dimensional space
struct basic_node
{
    std::int64_t index;
    std::array<double, 3> coordinates;
};

} // namespace imr
