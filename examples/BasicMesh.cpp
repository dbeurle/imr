
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    gmsh::Reader reader("basic.msh", true);
    reader.writeMeshToJson();
    std::cout << "Done!\n";
}
