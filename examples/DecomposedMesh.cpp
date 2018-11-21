
#include "GmshReader.hpp"

#include <iostream>

int main()
{
    imr::Reader reader("decomposed.msh",
                       imr::Reader::NodalOrdering::Local,
                       imr::Reader::IndexingBase::Zero);
    reader.writeMeshToJson();

    std::cout << "Done!\n";
}
