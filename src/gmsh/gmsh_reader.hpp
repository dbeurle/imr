
#pragma once

#include "element.hpp"
#include "element_types.hpp"
#include "mesh_reader.hpp"
#include "node.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace imr
{
/// gmsh_reader parses Gmsh format and returns the data structures of the mesh
/// in a json format for easier processing
class gmsh_reader : public mesh_reader
{
public:
    using Mesh = std::map<std::pair<std::string, std::int32_t>, std::vector<element>>;

    using owner_sharer_t = std::pair<std::int32_t, std::int32_t>;

public:
    /// \param File name of gmsh mesh
    /// \param Flag to use local processor ordering or retain global ordering.
    ///        If this is true, then each of the output meshes will have be ordered
    ///        locally and there will be a local to global mapping provided in the
    ///        the mesh file in addition to the nodal connectivity
    /// \param Flag for zero based indexing in nodal coordinates
    explicit gmsh_reader(std::string const& filename,
                         nodal_order const ordering,
                         index_base const base,
                         distributed const distributed_option);

    ~gmsh_reader() = default;

    virtual void parse() override final;

    virtual void write() const override final;

    /// Return a map of the physical names and the element data.
    /// The physicalIds and the names are given by names().
    /// The value in the map is a list of ElementData objects
    auto const& mesh() const { return meshes; }

    /// Return a list of the coordinates and Ids of the nodes
    std::vector<node> const& nodes() const { return nodal_data; }

    /// Return the physical names associated with the mesh
    std::map<std::int32_t, std::string> const& names() const { return physicalGroupMap; }

    /// Return the number of decompositions in the mesh
    auto numberOfPartitions() const -> std::int32_t { return m_partitions; }

private:
    /// Provide a reference to the nodes and dimensions that will be populated
    /// with the correct data based on the elementType
    /// \param elementTypeId gmsh element number
    /// \return number of nodes for the element
    int mapElementData(int const elementTypeId);

    /// Check the version of gmsh is support otherwise print out a warning
    /// \param gmshVersion
    void checkSupportedGmsh(float const gmshVersion);

    /// Return the local to global mapping for the nodal connectivities
    std::vector<std::int64_t> fillLocalToGlobalMap(Mesh const& process_mesh) const;

    /// Reorder the mesh to for each process
    void reorderLocalMesh(Mesh& processMesh,
                          std::vector<std::int64_t> const& local_global_mapping) const;

    /// Gather the local process nodal coordinates using the local to global mapping.
    /// This is required to reduce the number of coordinates for each process.
    /// \sa writeInJsonFormat
    std::vector<node>
    fillLocalNodeList(std::vector<std::int64_t> const& local_global_mapping) const;

    void write_json(Mesh const& process_mesh,
                    std::vector<std::int64_t> const& local_global_mapping,
                    std::vector<node> const& nodalCoordinates,
                    int const process_number,
                    bool const is_distributed,
                    bool const printIndices) const;

private:
    std::vector<node> nodal_data;

    Mesh meshes;

    /**
     * Key:
     * pair.first:  process that owns the element
     * pair.second: process that shares the element
     * Value:       node ids of the interface element
     */
    std::map<owner_sharer_t, std::set<std::int64_t>> interfaceElementMap;

    std::map<std::int32_t, std::string> physicalGroupMap;

    /// File name of gmsh file
    std::string input_file_name;

    bool useZeroBasedIndexing;
    bool useLocalNodalConnectivity;

    /// Output in FETI format
    bool is_feti_format = true;

    int m_partitions = 1;
};
} // namespace imr
