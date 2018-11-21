
#pragma once

namespace imr
{
/// node represents the geometry for a single node
struct node
{
    std::int64_t id;
    std::array<double, 3> coordinates;
};
} // namespace imr
