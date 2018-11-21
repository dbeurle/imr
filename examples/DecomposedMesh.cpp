
#include "mesh_reader.hpp"

#include <iostream>

int main()
{
    imr::mesh_reader reader("decomposed.msh",
                       imr::NodalOrdering::Local,
                       imr::IndexingBase::Zero);
    reader.writeMeshToJson();

    std::cout << "Done!\n";
}
