
#include "element_types.hpp"

#include <stdexcept>

namespace imr
{
auto nodes_per_element(std::int32_t const element_type) noexcept -> std::int32_t
{
    // Return the number of local nodes per element
    switch (element_type)
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
        {
            throw std::domain_error("The elementTypeId " + std::to_string(element_type) +
                                    " is not implemented");
        }
    }
    return -1;
}

} // namespace imr
