/*
 * For licensing please refer to the LICENSE.md file
 */

#pragma once

#include <string>
#include <vector>
#include <valarray>
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
    enum { LINE2 = 1, TRI3, QUAD4, TETRA4, HEX8, PRISM6, TRI6 = 9, TETRA10 = 11};

    const std::map<StringKey, Value>& mesh() const {return gmshMesh;}

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
     * @return number of nodes per element
     */
    short mapElementData(int elementType);

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
