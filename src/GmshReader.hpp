/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <map>

namespace gmsh
{
struct ElementData
{
    ElementData(int numberOfNodes, int numberOfTags, int elementType, int id) :
        tags(numberOfTags, 0),
        nodalConnectivity(numberOfNodes, 0),
        elementType(elementType),
        id(id)
    {
    }
    std::vector<int> tags;
    std::vector<int> nodalConnectivity;
    int elementType;
    int id;
};

struct NodeData
{
    int id;
    std::array<double, 3> coordinates;
};

/*!
 * \class GmshReader
 * \brief Parses Gmsh format and returns the data structures of the mesh
 * in a format for neon to process
 */
class Reader
{
public:

    using StringKey = std::string;
    using Value     = std::vector<ElementData>;

public:

    /** Constructor that accepts a file name of the gmsh formatted mesh file */
    Reader(const std::string& fileName);

    ~Reader() = default;

    /** Gmsh element numbering scheme */
    enum ELEMENT_TYPE_ID {// Standard linear elements
                          LINE2=1,
                          TRIANGLE3,
                          QUADRILATERAL4,
                          TETRAHEDRON4,
                          HEXAHEDRON8,
                          PRISM6,
                          PYRAMID5,
                          // Quadratic elements
                          LINE3,
                          TRIANGLE6,
                          QUADRILATERAL9,  // 4 vertex, 4 edges and 1 face node
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
                          TRIANGLE15_IC,  // Incomplete 15 node triangle
                          TRIANGLE21 = 25,
                          EDGE4,
                          EDGE5,
                          EDGE6,
                          TETRAHEDRON20,
                          TETRAHEDRON35,
                          TETRAHEDRON56,
                          HEXAHEDRON64 = 92,
                          HEXAHEDRON125};

    /**
     * Return a map of the physical names and the element data.
     * The physicalIds and the names are given by names().
     * The value in the map is a list of ElementData objects
     */
    const std::map<StringKey, Value>& mesh() const {return gmshMesh;}

    /** Return a list of the coordinates and Ids of the nodes */
    const std::vector<NodeData>& nodes() const {return nodeList;}

    /** Return the physical names associated with the mesh */
    const std::map<int, std::string>& names() const {return physicalGroupMap;}

    void writeMesh(const std::string& outputFileName);

private:

    /**
     * Parse the gmsh file as provided by the filename with the appended file
     * extension (.msh)
     */
    void parse();

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

private:

    std::map<StringKey, Value> gmshMesh;
    std::map<int, std::string> physicalGroupMap;

    std::vector<NodeData> nodeList;

    std::string fileName;   //!< File name of gmsh file
    std::fstream gmshFile;  //!< Hold an object to the file stream

};
}
