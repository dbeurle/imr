
#pragma once

#include <cstdint>

namespace imr
{
/// Gmsh element numbering scheme
enum ELEMENT_TYPE_ID {
    // Standard linear elements
    LINE2 = 1,
    TRIANGLE3,
    QUADRILATERAL4,
    TETRAHEDRON4,
    HEXAHEDRON8,
    PRISM6,
    PYRAMID5,
    // Quadratic elements
    LINE3,
    TRIANGLE6,
    QUADRILATERAL9, // 4 vertex, 4 edges and 1 face node
    TETRAHEDRON10,
    HEXAHEDRON27,
    PRISM18,
    PYRAMID14,
    POINT = 15,
    QUADRILATERAL8,
    HEXAHEDRON20,
    PRISM15,
    PYRAMID13,
    TRIANGLE9 = 20,
    TRIANGLE10,
    TRIANGLE12,
    TRIANGLE15,
    TRIANGLE15_IC, // Incomplete 15 node triangle
    TRIANGLE21 = 25,
    EDGE4,
    EDGE5,
    EDGE6,
    TETRAHEDRON20,
    TETRAHEDRON35,
    TETRAHEDRON56,
    HEXAHEDRON64 = 92,
    HEXAHEDRON125
};

/// Provide the number of nodes in the element \sa ELEMENT_ID_TYPE
auto nodes_per_element(std::int32_t const element_type) noexcept -> std::int32_t;

} // namespace imr
