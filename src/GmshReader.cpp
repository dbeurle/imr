/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"

#include <iostream>

#include "GmshReaderException.hpp"

namespace neon
{
GmshReader::GmshReader(const std::string& fileName)
{
    if (!this->parse(fileName)) throw GmshFileParseException(fileName);
}

bool GmshReader::parse(const std::string& inputFileName)
{
    fileName = inputFileName;

    gmshFile.precision(sizeof(double));

    this->fillMesh();

    return true;
}

void GmshReader::fillMesh()
{
    gmshFile.open(fileName.c_str());

    if (!gmshFile.is_open()) throw GmshFileOpenException(fileName);

    std::string token, null;

    std::vector<std::string> physicalNames;

    // Loop around file and read in lines //
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
            int dimension, physicalId, physicalIds;
            std::string physicalName;

            gmshFile >> physicalIds;

            for (auto physicalId = 0; physicalId < physicalIds; physicalId++)
            {
                gmshFile >> dimension >> physicalId >> physicalName;

                // Extract the name from the quotes //
                physicalName.erase(remove(physicalName.begin(), physicalName.end(), '\"'),
                                   physicalName.end());
                physicalNames.push_back(physicalName);
            }
            token.clear();
            gmshFile >> token;
        }
        else if (token == "$Nodes")
        {
            auto globalNodeIds = 0u;
            gmshFile >> globalNodeIds;

            //mesh.setNumberOfNodes(globalNodeIds);

            for (auto globalNodeId = 0; globalNodeId < globalNodeIds; globalNodeId++)
            {
                // Need to come up with a nodal coordinate format too
                // maybe a vector / valarray of tuples?

                // gmshFile >> null >> mesh.setNodeCoordinates(0, globalNodeId)
                //                  >> mesh.setNodeCoordinates(1, globalNodeId)
                //                  >> mesh.setNodeCoordinates(2, globalNodeId);
            }
        }
        else if (token == "$Elements")
        {
            auto elementIds = 0u;
            gmshFile >> elementIds;
            for (auto elementId = 0; elementId < elementIds; elementId++)
            {
                auto physicalId = 0u, nodeIds = 0u, numberOfTags = 0u;
                auto elementTypeNum = 0;

                gmshFile >> null >> elementTypeNum >> numberOfTags;

                ElementData elementData(mapElementData(elementTypeNum),
                                        numberOfTags,
                                        elementTypeNum);

                for (auto tag = 0; tag < numberOfTags; tag++)
                {
                    gmshFile >> elementData.tags[tag];
                }
                for (auto nodeId = 0; nodeId < nodeIds; nodeId++)
                {
                    // Read-in the nodal connectivity data and convert to
                    // zero-based indexing
                    gmshFile >> elementData.nodalConnectivity[nodeId];
                    elementData.nodalConnectivity[nodeId]--;
                }
                gmshMesh[physicalNames[--elementData.tags[0]]].push_back(elementData);
            }
        }
    }
    gmshFile.close();
}

short GmshReader::mapElementData(int elementType)
{
	switch (elementType)
	{
	case LINE2:
		return 2;
		break;
	case TRI3:
		return 3;
		break;
	case QUAD4:
		return 4;
		break;
	case TETRA4:
		return 4;
		break;
	case HEX8:
		return 8;
		break;
	case PRISM6:
		return 6;
		break;
	case TRI6:
		return 6;
		break;
	case TETRA10:
		return 10;
		break;
	default:
		throw GmshElementCodeException(elementType);
	}
}

void GmshReader::checkSupportedGmsh(float gmshVersion)
{
    if (gmshVersion < 2.2) Warn<< "WARNING: gmsh unsupported\n";
}
}
