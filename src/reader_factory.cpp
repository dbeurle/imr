
#include "reader_factory.hpp"

#include "abaqus/abaqus_reader.hpp"
#include "gmsh/gmsh_reader.hpp"

#include <iostream>
#include <stdexcept>

namespace imr
{
std::unique_ptr<mesh_reader> make_reader(std::string const& filename,
                                         nodal_order const ordering,
                                         index_base const base,
                                         distributed const distributed_option)
{
    // extract file extension
    std::string const extension = filename.substr(filename.find_last_of('.') + 1, filename.size());

    if (extension == "inp")
    {
        std::cout << "Detected an ABAQUS input file\n\n";
        return std::make_unique<abaqus_reader>(filename);
    }
    else if (extension == "msh")
    {
        std::cout << "Detected a Gmsh input file\n\n";
        return std::make_unique<gmsh_reader>(filename, ordering, base, distributed_option);
    }
    throw std::domain_error("Input extension is not recognised.  Try .msh or .inp");

    return nullptr;
}
} // namespace imr
