
#include "GmshReader.hpp"

#include <iostream>

int main()
{
    imr::Reader reader("fourPointBending.msh",
                       imr::Reader::NodalOrdering::Global,
                       imr::Reader::IndexingBase::One);

    reader.writeMeshToJson();
    std::cout << "Done!" << std::endl;
}
