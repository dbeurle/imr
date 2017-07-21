/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <array>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "ElementData.hpp"

namespace gmsh
{

struct NodeData
{
    int id;
    std::array<double, 3> coordinates;
};

/**
 * Reader parses Gmsh format and returns the data structures of the mesh
 * in a json format for easier processing
 */
class Reader
{
public:
    using StringKey = std::string;
    using Value     = std::vector<ElementData>;

public:
    enum class NodalOrdering { Local, Global };
    enum class IndexingBase { Zero, One };

    /** Gmsh element numbering scheme */
    enum ELEMENT_TYPE_ID { // Standard linear elements
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
    Reader(std::string const& fileName, NodalOrdering, IndexingBase);

    ~Reader() = default;

    /**
     * Return a map of the physical names and the element data.
     * The physicalIds and the names are given by names().
     * The value in the map is a list of ElementData objects
     */
    const std::map<StringKey, Value>& mesh() const { return gmshMesh; }

    /** Return a list of the coordinates and Ids of the nodes */
    const std::vector<NodeData>& nodes() const { return nodeList; }

    /** Return the physical names associated with the mesh */
    const std::map<int, std::string>& names() const { return physicalGroupMap; }

    void writeMesh(const std::string& outputFileName);

    /**
     * Write out a distributed mesh in the Murge format which requires a
     * local to global mapping for the distributed matrices from a finite
     * element discretization.  This involves performing a reordering of
     * each of the element nodal connectivity arrays from the global view
     * that gmsh outputs and the local processor view that Murge expects.
     */
    void writeMeshToJson() const;

private:
    /**
     * Provide a reference to the nodes and dimensions that will be populated
     * with the correct data based on the elementType
     * @param Gmsh element number
     * @return elementType number of nodes per element
     */
    int mapElementData(int elementTypeId);

    /**
     * Check the version of gmsh is support otherwise print out a warning
     * @param gmshVersion
     */
    void checkSupportedGmsh(float gmshVersion);

    /** This method fills the datastructures \sa ElementData \sa NodeData */
    void fillMesh();

    /** Return the number of decompositions in the mesh */
    int numberOfPartitions() const;

    /** Return the local to global mapping for the nodal connectivities */
    std::vector<int> fillLocalToGlobalMap(std::map<StringKey, Value>& processMesh) const;

    /** Reorder the mesh to for each processor */
    void reorderLocalMesh(std::map<StringKey, Value>& processMesh,
                          std::vector<int> const& localToGlobalMapping) const;

    /** Gather the local process nodes using the local to global mapping */
    std::vector<NodeData>
    fillLocalNodeList(std::vector<int> const& localToGlobalMapping) const;

    void writeInJsonFormat(std::map<StringKey, Value> const& processMesh,
                           std::vector<int> const& localToGlobalMapping,
                           std::vector<NodeData> const& nodalCoordinates,
                           int processId,
                           bool isMeshDistributed) const;

private:
    std::map<StringKey, Value> gmshMesh;
    std::map<int, std::string> physicalGroupMap;
    /**
     * Key:
     * pair.first:  process that owns the element
     * pair.second: process that shares the element
     * Value:       node ids of the interface element
     *
     */
    std::map<std::pair<int, int>, std::set<int>> interfaceElementMap;

    std::vector<NodeData> nodeList;

    std::string fileName;  //!< File name of gmsh file
    std::fstream gmshFile; //!< Hold an object to the file stream

    bool useZeroBasedIndexing;
    bool useLocalNodalConnectivity;
};
}
