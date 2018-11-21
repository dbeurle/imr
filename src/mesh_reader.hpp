/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "element.hpp"
#include "node.hpp"

namespace imr
{
/// Mesh partition nodal connectivity
enum class NodalOrdering { Local, Global };

/// Nodal numbering to be zero or one based
enum class IndexingBase { Zero, One };

/// Ordering for distribution of mshes
enum class distributed { feti, interprocess };

/// mesh_reader parses Gmsh format and returns the data structures of the mesh
/// in a json format for easier processing
class mesh_reader
{
public:
    using Value = std::vector<element>;
    using Mesh  = std::map<std::pair<std::string, std::int32_t>, Value>;

    using owner_sharer_t = std::pair<std::int32_t, std::int32_t>;

public:
    /** Gmsh element numbering scheme */
    enum ELEMENT_TYPE_ID {
        // Standard linear elements
        LINE2 = 1,
        TRIANGLE3,
        QUADRILATERAL4,
        TETRAHEDRON4,
        HEXAHEDRON8,
        PRISM6,
        PYRAMID5,
        // Quadratic elements
        LINE3,
        TRIANGLE6,
        QUADRILATERAL9, // 4 vertex, 4 edges and 1 face node
        TETRAHEDRON10,
        HEXAHEDRON27,
        PRISM18,
        PYRAMID14,
        POINT = 15,
        QUADRILATERAL8,
        HEXAHEDRON20,
        PRISM15,
        PYRAMID13,
        TRIANGLE9 = 20,
        TRIANGLE10,
        TRIANGLE12,
        TRIANGLE15,
        TRIANGLE15_IC, // Incomplete 15 node triangle
        TRIANGLE21 = 25,
        EDGE4,
        EDGE5,
        EDGE6,
        TETRAHEDRON20,
        TETRAHEDRON35,
        TETRAHEDRON56,
        HEXAHEDRON64 = 92,
        HEXAHEDRON125
    };

public:
    /**
     * @param File name of gmsh mesh
     * @param Flag to use local processor ordering or retain global ordering.
              If this is true, then each of the output meshes will have be ordered
              locally and there will be a local to global mapping provided in the
              the mesh file in addition to the nodal connectivity
     * @param Flag for zero based indexing in nodal coordinates
     */
    mesh_reader(std::string const& input_file_name,
                NodalOrdering const ordering,
                IndexingBase const base,
                distributed const distributed_option = distributed::feti);

    ~mesh_reader() = default;

    /**
     * Return a map of the physical names and the element data.
     * The physicalIds and the names are given by names().
     * The value in the map is a list of ElementData objects
     */
    auto const& mesh() const { return meshes; }

    /** Return a list of the coordinates and Ids of the nodes */
    std::vector<node> const& nodes() const { return nodal_data; }

    /** Return the physical names associated with the mesh */
    std::map<std::int32_t, std::string> const& names() const { return physicalGroupMap; }

    /**
     * Write out a distributed mesh in the Murge format which requires a
     * local to global mapping for the distributed matrices from a finite
     * element discretization.  This involves performing a reordering of
     * each of the element nodal connectivity arrays from the global view
     * that gmsh outputs and the local processor view that Murge expects.
     */
    void writeMeshToJson(bool const printIndices = true) const;

    /** Return the number of decompositions in the mesh */
    auto numberOfPartitions() const { return number_of_partitions; }

private:
    /**
     * Provide a reference to the nodes and dimensions that will be populated
     * with the correct data based on the elementType
     * @param elementTypeId gmsh element number
     * @return number of nodes for the element
     */
    int mapElementData(int const elementTypeId);

    /// Check the version of gmsh is support otherwise print out a warning
    /// @param gmshVersion
    void checkSupportedGmsh(float const gmshVersion);

    /** This method fills the datastructures \sa ElementData \sa node */
    void fillMesh();

    /** Return the local to global mapping for the nodal connectivities */
    std::vector<std::int64_t> fillLocalToGlobalMap(Mesh const& process_mesh) const;

    /** Reorder the mesh to for each process */
    void reorderLocalMesh(Mesh& processMesh,
                          std::vector<std::int64_t> const& local_global_mapping) const;

    /// Gather the local process nodal coordinates using the local to global mapping.
    /// This is required to reduce the number of coordinates for each process.
    /// \sa writeInJsonFormat
    std::vector<node>
    fillLocalNodeList(std::vector<std::int64_t> const& local_global_mapping) const;

    void writeInJsonFormat(Mesh const& process_mesh,
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

    int number_of_partitions = 1;
};
} // namespace imr
