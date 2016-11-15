#include <iostream>
#include "GmshReader.hpp"
int main()
{
    using namespace gmsh;

    gmsh::Reader reader( "feti_beam_fine.msh",
                         Reader::NodalOrdering::Local,
                         Reader::IndexingBase::Zero);
    reader.writeMeshToJson();

    std::cout << "This is my first example" << std::endl;
}
