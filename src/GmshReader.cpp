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
    parse(fileName);
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
            for (auto physicalId = 0; physicalId < physicalIds; ++physicalId)
            {
                int dimension = 0;
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
                gmshMesh[physicalNames[physicalId]].push_back(elementData);
            }
        }
    }
    gmshFile.close();
}

int GmshReader::mapElementData(int elementType)
{
	switch (elementType)
	{
	case LINE2:
		return 2;
		break;
	case TRIANGLE3:
		return 3;
		break;
	case QUADRILATERAL4:
		return 4;
		break;
	case TETRAHEDRON4:
		return 4;
		break;
	case HEXAHEDRON8:
		return 8;
		break;
	case PRISM6:
		return 6;
		break;
	case TRIANGLE6:
		return 6;
		break;
	case TETRAHEDRON10:
		return 10;
		break;
	default:
		throw GmshReaderException("The elementTypeId "
                                  + std::to_string(elementType)
                                  + " is not implemented");
	}
}

void GmshReader::checkSupportedGmsh(float gmshVersion)
{
    if (gmshVersion < 2.2)
        throw GmshReaderException("GmshVersion "
                                  + std::to_string(gmshVersion)
                                  + " is not supported");
}
}
