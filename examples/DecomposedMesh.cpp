
#include "GmshReader.hpp"

#include <iostream>

int main()
{
    using namespace gmsh;

    gmsh::Reader reader("decomposed.msh",
                        Reader::NodalOrdering::Local,
                        Reader::IndexingBase::Zero);
    reader.writeMeshToJson();

    std::cout << "Done!\n";
}
