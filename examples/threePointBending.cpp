
#include "mesh_reader.hpp"

#include <iostream>

int main()
{

    imr::mesh_reader reader("threePointBending.msh",
                       imr::NodalOrdering::Global,
                       imr::IndexingBase::One);
    reader.writeMeshToJson();
    std::cout << "Done!" << std::endl;
}
