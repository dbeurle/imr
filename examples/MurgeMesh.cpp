
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    gmsh::Reader reader("basic.msh");

    reader.writeMurgeToJson();

    std::cout << "Done!\n";

}
