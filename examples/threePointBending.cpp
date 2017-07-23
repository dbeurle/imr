
#include "GmshReader.hpp"

#include <iostream>

int main()
{
    using namespace gmsh;
    gmsh::Reader reader("threePointBending.msh",
                        Reader::NodalOrdering::Global,
                        Reader::IndexingBase::One);
    reader.writeMeshToJson();
    std::cout << "Done!" << std::endl;
}
