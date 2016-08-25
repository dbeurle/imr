#include <iostream>
#include "GmshReader.hpp"
int main()
{
  neon::GmshReader reader("feti_beam.msh");
  std::cout << "This is my first example" << std::endl;
}
