#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("threePointBending.msh");
  reader.writeMurgeToJson();
  std::cout << "Done!" << std::endl;
}
