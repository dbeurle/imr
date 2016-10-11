#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("fourPointBending.msh");
  reader.writeMurgeToJson();
  std::cout << "Done!" << std::endl;
}
