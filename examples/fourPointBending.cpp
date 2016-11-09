#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("fourPointBending.msh");
  reader.writeMeshToJson();
  std::cout << "Done!" << std::endl;
}
