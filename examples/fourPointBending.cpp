#include <iostream>
#include "GmshReader.hpp"
int main()
{
    using namespace gmsh;
    Reader reader( "fourPointBending.msh",
                   Reader::NodalOrdering::Global,
                   Reader::IndexingBase::One);

    reader.writeMeshToJson();
    std::cout << "Done!" << std::endl;
}
