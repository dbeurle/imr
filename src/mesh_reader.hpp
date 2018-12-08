
#pragma once

#include <string>

namespace imr
{
/// Mesh partition nodal connectivity
enum class nodal_order { local, global };

/// Nodal numbering to be zero or one based
enum class index_base { zero, one };

/// Ordering for distribution of mshes
enum class distributed { serial, feti, interprocess };

/// Owner-sharer storage class for domain decomposed mesh
struct process
{
    std::int32_t owner;
    std::int32_t sharer;
};

class mesh_reader
{
public:
    mesh_reader(std::string filename) : filename(filename) {}

    virtual ~mesh_reader() = default;

    void virtual parse() = 0;

    void virtual write() const = 0;

protected:
    std::string filename;

    bool use_zero_indexing = true;
};
} // namespace imr
