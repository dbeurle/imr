
#include "mesh_reader.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    std::string file = argv[1];

    std::cout << file << std::endl;

    imr::mesh_reader reader(file,
                            imr::NodalOrdering::Global,
                            imr::IndexingBase::One,
                            imr::distributed::feti);

    reader.write();

    std::cout << "This is my first example" << std::endl;
}
