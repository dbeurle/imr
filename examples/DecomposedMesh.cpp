
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    gmsh::Reader reader("decomposed.msh");
    reader.writeMurgeToJson();
    std::cout << "Done!\n";
}
