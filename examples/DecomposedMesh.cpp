
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    gmsh::Reader reader("decomposed.msh");
    reader.writeMeshToJson();
    std::cout << "Done!\n";
}
