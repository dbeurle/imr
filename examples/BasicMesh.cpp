
#include "GmshReader.hpp"

#include <iostream>

int main()
{
    // Write out a basic mesh using zero based indexing and local numbering
    imr::Reader rectangleReader("basic.msh",
                                imr::Reader::NodalOrdering::Local,
                                imr::Reader::IndexingBase::Zero);
    rectangleReader.writeMeshToJson();

    imr::Reader cubeReader("cube.msh",
                           imr::Reader::NodalOrdering::Local,
                           imr::Reader::IndexingBase::Zero);
    cubeReader.writeMeshToJson();

    std::cout << "Done!\n";
}
