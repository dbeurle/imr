/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <string>
#include <vector>
#include <valarray>
#include <array>
#include <fstream>
#include <map>

namespace neon
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
class GmshReader
{
public:

    using StringKey = std::string;
    using Value     = std::vector<ElementData>;

public:

    GmshReader(const std::string& fileName);

    ~GmshReader() = default;

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

    const std::map<StringKey, Value>& mesh() const {return gmshMesh;}

    void writeMesh(const std::string& fileName);

private:

    /**
     * Parse the gmsh file as provided by the filename with the appended file
     * extension (.msh)
     * @param file name of the mesh to open
     * @return true if the read was successful
     */
     bool parse(const std::string& fileName);

    /**
     * Provide a reference to the nodes and dimensions that will be populated
     * with the correct data based on the elementType
     * @param Gmsh element number
     * @return elementType number of nodes per element
     */
    int mapElementData(int elementType);

    /**
     * Check the version of gmsh is support otherwise print out a warning
     * @param gmshVersion
     */
    void checkSupportedGmsh(float gmshVersion);

    void fillMesh();

private:

    std::map<StringKey, Value> gmshMesh;
    std::map<int, std::string> physicalGroupMap;

    std::vector<NodeData> nodeList;

    std::string fileName;   //!< File name of gmsh file
    std::fstream gmshFile;  //!< Hold an object to the file stream

};
}
