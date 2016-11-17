
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    using namespace gmsh;

    // Write out a basic mesh using zero based indexing and local numbering
    Reader rectangleReader( "basic.msh",
                            Reader::NodalOrdering::Local,
                            Reader::IndexingBase::Zero);
    rectangleReader.writeMeshToJson();

    Reader cubeReader( "cube.msh",
                       Reader::NodalOrdering::Local,
                       Reader::IndexingBase::Zero);
    cubeReader.writeMeshToJson();

    std::cout << "Done!\n";
}
