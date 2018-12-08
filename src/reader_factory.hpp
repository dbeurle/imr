
#pragma once

#include <memory>
#include <string>

#include "mesh_reader.hpp"

namespace imr
{
std::unique_ptr<mesh_reader> make_reader(std::string const& filename,
                                         nodal_order ordering,
                                         index_base base,
                                         distributed distributed_option);
} // namespace imr
