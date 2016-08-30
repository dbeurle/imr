#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("feti_beam.msh");
  reader.writeMesh("mesh.neon");
  std::cout << "This is my first example" << std::endl;
}
