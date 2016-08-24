#include <iostream>
#include "GmshReader.hpp"
int main()
{
  neon::GmshReader reader("invalid mesh file");
  std::cout << "This is my first example" << std::endl;
}
