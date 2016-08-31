/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include "GmshReaderException.hpp"

namespace gmsh
{
Reader::Reader(const std::string& fileName) : fileName(fileName)
{
    parse();
}

void Reader::parse()
{
    gmshFile.precision(sizeof(double));
    fillMesh();
}

void Reader::fillMesh()
{
    gmshFile.open(fileName.c_str());

    if (!gmshFile.is_open())
        throw GmshReaderException("Filename " + fileName + " is not valid");

    std::string token, null;

    std::vector<std::string> physicalNames;

    // Loop around file and read in keyword tokens
    while (!gmshFile.eof())
    {
        gmshFile >> token;

        if (token == "$MeshFormat")
        {
            float gmshVersion;  // File format version
            short dataType;     // Precision

            gmshFile >> gmshVersion >> dataType >> null;
            checkSupportedGmsh(gmshVersion);
        }
        else if (token == "$PhysicalNames")
        {
            std::string physicalName;
            int physicalIds = 0;
            gmshFile >> physicalIds;
            for (auto i = 0; i < physicalIds; ++i)
            {
                int dimension   = 0;
                int physicalId  = 0;
                gmshFile >> dimension >> physicalId >> physicalName;

                // Extract the name from the quotes
                physicalName.erase(remove(physicalName.begin(), physicalName.end(), '\"'),
                                   physicalName.end());
                physicalGroupMap.emplace(physicalId, physicalName);
            }
            token.clear();
            gmshFile >> token;
        }
        else if (token == "$Nodes")
        {
            int nodeIds = 0;
            gmshFile >> nodeIds;
            nodeList.resize(nodeIds);

            for (auto& node : nodeList)
                gmshFile >> node.id >> node.coordinates[0] >> node.coordinates[1] >> node.coordinates[2];

        }
        else if (token == "$Elements")
        {
            int elementIds = 0;
            gmshFile >> elementIds;
            for (int elementId = 0; elementId < elementIds; elementId++)
            {
                int id             = 0;
                int numberOfTags   = 0;
                int elementTypeNum = 0;

                gmshFile >> id >> elementTypeNum >> numberOfTags;

                int numberOfNodes = mapElementData(elementTypeNum);

                ElementData elementData(numberOfNodes,
                                        numberOfTags,
                                        elementTypeNum,
                                        id);

                for (auto& tag : elementData.tags)
                    gmshFile >> tag;

                for (auto& nodeId : elementData.nodalConnectivity)
                    gmshFile >> nodeId;

                int physicalId = elementData.tags[0];
                gmshMesh[physicalGroupMap[physicalId]].push_back(elementData);
            }
        }
    }
    gmshFile.close();
}

int Reader::mapElementData(int elementType)
{
    int lnodeIds = 0;
	switch (elementType)
	{
        case LINE2:          lnodeIds = 2;  break;
        case TRIANGLE3:      lnodeIds = 3;  break;
        case QUADRILATERAL4: lnodeIds = 4;  break;
        case TETRAHEDRON4:   lnodeIds = 4;  break;
        case HEXAHEDRON8:    lnodeIds = 8;  break;
        case PRISM6:         lnodeIds = 6;  break;
        case PYRAMID5:       lnodeIds = 5;  break;
        case LINE3:          lnodeIds = 3;  break;
        case TRIANGLE6:      lnodeIds = 6;	break;
        case QUADRILATERAL9: lnodeIds = 9;  break;
        case TETRAHEDRON10:  lnodeIds = 10;	break;
        case HEXAHEDRON27:   lnodeIds = 27; break;
        case PRISM18:        lnodeIds = 18; break;
        case PYRAMID14:      lnodeIds = 14; break;
        case POINT:          lnodeIds = 1;  break;
        case QUADRILATERAL8: lnodeIds = 8;  break;
        case HEXAHEDRON20:   lnodeIds = 20; break;
        case PRISM15:        lnodeIds = 15; break;
        case PYRAMID13:      lnodeIds = 13; break;
        case TRIANGLE9:      lnodeIds = 19; break;
        case TRIANGLE10:     lnodeIds = 10; break;
        case TRIANGLE12:     lnodeIds = 12; break;
        case TRIANGLE15:     lnodeIds = 15; break;
        case TRIANGLE15_IC:  lnodeIds = 15; break;
        case TRIANGLE21:     lnodeIds = 21; break;
        case EDGE4:          lnodeIds = 4;  break;
        case EDGE5:          lnodeIds = 5;  break;
        case EDGE6:          lnodeIds = 6;  break;
        case TETRAHEDRON20:  lnodeIds = 20; break;
        case TETRAHEDRON35:  lnodeIds = 35; break;
        case TETRAHEDRON56:  lnodeIds = 56; break;
        case HEXAHEDRON64:   lnodeIds = 64; break;
        case HEXAHEDRON125:  lnodeIds = 125;break;
	default:
		throw GmshReaderException("The elementTypeId "
                                  + std::to_string(elementType)
                                  + " is not implemented");
	}
    return lnodeIds;
}

void Reader::checkSupportedGmsh(float gmshVersion)
{
    if (gmshVersion < 2.2)
        throw GmshReaderException("GmshVersion " + std::to_string(gmshVersion)
                                  + " is not supported");
}

void Reader::writeMesh(const std::string& outputFileName)
{
    std::ofstream file(outputFileName);

    if(not file.is_open())
        throw GmshReaderException("Failed to open " + outputFileName);

    int numElements = 0;
    for (const auto& physicalGroup : gmshMesh)
        numElements += physicalGroup.second.size();

    file << "numNodes    \t" << nodeList.size()     << "\n";
    file << "numElements \t" << numElements         << "\n";

    file << "Nodes \n";
    for (const auto& node : nodeList)
    {
        file << std::setw(10) << std::right << node.id << "\t";
        file << std::setw(10) << std::right << node.coordinates[0] << "\t";
        file << std::setw(10) << std::right << node.coordinates[1] << "\t";
        file << std::setw(10) << std::right << node.coordinates[2] << "\n";
    }

    for (const auto& pairNameAndElements : gmshMesh)
    {
        file << "Physical group \t" << pairNameAndElements.first << "\n";
        file << "Elements \n";
        for (const auto& element : pairNameAndElements.second)
        {
            file << std::setw(10) << std::right << element.id << "\t";
            for (const auto& nodeId : element.nodalConnectivity)
                file << std::setw(10) << std::right << nodeId << "\t";

            file << "\n";
        }
     }
    file.close();
}

}
