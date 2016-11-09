#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("threePointBending.msh");
  reader.writeMeshToJson();
  std::cout << "Done!" << std::endl;
}
