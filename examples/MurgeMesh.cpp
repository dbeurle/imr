
#include <iostream>
#include "GmshReader.hpp"

int main()
{
    gmsh::Reader reader("feti_beam.msh");

    reader.writeMurgeToJson();

    std::cout << "Done!\n";

}
