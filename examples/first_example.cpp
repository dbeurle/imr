#include <iostream>
#include "GmshReader.hpp"

using namespace gmsh;

int main(int argc, char** argv)
{


    std::string file = argv[1];

    std::cout << file << std::endl;

    gmsh::Reader reader( file,
                         Reader::NodalOrdering::Global,
                         Reader::IndexingBase::One);

    reader.writeMeshToJson();

    std::cout << "This is my first example" << std::endl;
}
