
#include "mesh_reader.hpp"

#include <iostream>

int main()
{
    imr::mesh_reader reader("decomposed.msh",
                            imr::NodalOrdering::Local,
                            imr::IndexingBase::Zero,
                            imr::distributed::feti);
    reader.write();

    std::cout << "Done!\n";
}
