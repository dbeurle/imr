
#include "mesh_reader.hpp"

#include <iostream>

int main()
{
    // Write out a basic mesh using zero based indexing and local numbering
    imr::mesh_reader rectangleReader("basic.msh",
                                     imr::NodalOrdering::Local,
                                     imr::IndexingBase::Zero,
                                     imr::distributed::feti);
    rectangleReader.write();

    imr::mesh_reader cubeReader("cube.msh",
                                imr::NodalOrdering::Local,
                                imr::IndexingBase::Zero,
                                imr::distributed::feti);
    cubeReader.write();

    std::cout << "Done!\n";
}
