#include <iostream>
#include "GmshReader.hpp"
int main()
{
  gmsh::Reader reader("feti_beam_fine.msh");
  reader.writeMurgeToJson();
  std::cout << "This is my first example" << std::endl;
}
