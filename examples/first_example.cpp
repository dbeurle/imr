
#include "GmshReader.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::string file = argv[1];

    std::cout << file << std::endl;

    imr::Reader reader(file, imr::Reader::NodalOrdering::Global, imr::Reader::IndexingBase::One);

    reader.writeMeshToJson();

    std::cout << "This is my first example" << std::endl;
}
