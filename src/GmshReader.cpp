/*
 * For licensing please refer to the LICENSE.md file
 */

#include "GmshReader.hpp"

#include <iostream>
#include <iomanip>
#include <json/json.h>
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
                int dimension  = 0;
                int physicalId = 0;
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
                int id            = 0;
                int numberOfTags  = 0;
                int elementTypeId = 0;

                gmshFile >> id >> elementTypeId >> numberOfTags;

                int numberOfNodes = mapElementData(elementTypeId);

                ElementData elementData(numberOfNodes,
                                        numberOfTags,
                                        elementTypeId,
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

int Reader::mapElementData(int elementTypeId)
{
    int lnodeIds = 0;
	switch (elementTypeId)
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
                                  + std::to_string(elementTypeId)
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

    int numElements = std::accumulate(gmshMesh.begin(), gmshMesh.end(), 0,
                                      [](auto a, auto const& mesh)
                                      {return a + mesh.second.size();});

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

void Reader::writeMurgeToJson() const
{
    // Find the total number of processes associated with the mesh
    int processIds = processIdsDecomposedMesh();

    for (auto processId = 0; processId < processIds; ++processId)
    {
        std::map<StringKey, Value> processMesh;

        std::cout << "Building process mesh..." << std::flush;

        for (auto const& mesh : gmshMesh)
        {
            for (auto const& element : mesh.second)
            {
                if (element.isOwnedByProcess(processId+1))
                {
                    // Build a local copy of the elements to be offset later
                    processMesh[mesh.first].push_back(element);
                }
            }
        }

        std::cout << "done\nFilling the local to global mapping..." << std::flush;

        auto localToGlobalMapping = fillLocalMap(processMesh);

        std::cout << "done\nFilling the local node list for this proc..." << std::flush;

        auto localNodes = fillLocalNodeList(localToGlobalMapping);

        std::cout << "done\nReordering the local mesh..." << std::flush;

        reorderLocalMesh(processMesh, localToGlobalMapping);

        std::cout << "done\nSorting the mesh..." << std::flush;

        // Sort the elements based on the elementTypeId to output grouped
        // meshes for contiguous storage in the FEM program
        for (auto& mesh : processMesh)
        {
            std::sort( mesh.second.begin(), mesh.second.end(),
                       [](auto const& a, auto const& b)
                       {
                           return a.typeId < b.typeId;
                       });
        }

        std::cout << "done.\nWriting to json format..." << std::flush;

        writeInJsonFormat( processMesh,
                           localToGlobalMapping,
                           localNodes,
                           processIds != 1 ? fileName+std::to_string(processId)+".mesh"
                                           : fileName+".mesh");
        std::cout << "done!\n" << std::flush;
    }
}

std::vector<int> Reader::fillLocalMap(std::map<StringKey, Value>& processMesh) const
{
    std::vector<int> localToGlobalMapping;
    for (auto& mesh : processMesh)
    {
        for (auto const& element : mesh.second)
        {
            std::copy( element.nodalConnectivity.begin(),
                       element.nodalConnectivity.end(),
                       std::back_inserter(localToGlobalMapping));
        }
    }
    // Sort and remove duplicates
    std::sort(localToGlobalMapping.begin(), localToGlobalMapping.end());
    localToGlobalMapping.erase( std::unique( localToGlobalMapping.begin(),
                                             localToGlobalMapping.end()),
                                localToGlobalMapping.end());
    return localToGlobalMapping;
}

void Reader::reorderLocalMesh( std::map<StringKey, Value>& processMesh,
                               std::vector<int> const& localToGlobalMapping) const
{
    for (auto& mesh : processMesh)
    {
        for (auto& element : mesh.second)
        {
            for (auto& node : element.nodalConnectivity)
            {
                auto found = std::lower_bound( localToGlobalMapping.begin(),
                                               localToGlobalMapping.end(),
                                               node);
                // Reset the node value to that inside the
                node = std::distance(localToGlobalMapping.begin(), found);
            }
        }
    }
}

std::vector<NodeData> Reader::fillLocalNodeList(std::vector<int> const& localToGlobalMapping) const
{
    std::vector<NodeData> localNodeList;
    for (auto const& map : localToGlobalMapping)
    {
        localNodeList.push_back(nodeList[map-1]);
    }
    return localNodeList;
}

void Reader::writeInJsonFormat( std::map<StringKey, Value> const& processMesh,
                                std::vector<int> const& localToGlobalMapping,
                                std::vector<NodeData> const& nodalCoordinates,
                                std::string const& filename,
                                bool isZeroBased) const
{
    // Write out each file to Json format
    Json::Value event;

    std::fstream writer;
    writer.open(filename, std::ios::out);

    for (auto const& node : nodalCoordinates)
    {
        Json::Value coordinates(Json::arrayValue);
        for (auto const& xyz : node.coordinates)
        {
            coordinates.append(Json::Value(xyz));
        }
        event["Nodes"].append(coordinates);
    }

    for (auto const& mesh : processMesh)
    {
        // Find all of the unique elementTypeIds
        std::vector<int> elementTypeIds;
        for (auto const& element : mesh.second)
        {
            elementTypeIds.push_back(element.typeId);
        }
        std::sort(elementTypeIds.begin(), elementTypeIds.end());
        elementTypeIds.erase( std::unique(elementTypeIds.begin(), elementTypeIds.end()),
                              elementTypeIds.end());

        for (auto const& elementTypeId : elementTypeIds)
        {
            Json::Value elementGroup;
            // Print out the lower and upper bounds of this particular typeId
            auto lower = std::lower_bound(mesh.second.begin(), mesh.second.end(),
                                          elementTypeId,
                                          [](auto a, auto b) {return a.typeId < b;});
            auto upper = std::upper_bound(mesh.second.begin(), mesh.second.end(),
                                          elementTypeId,
                                          [](auto a, auto b) {return a < b.typeId;});

            std::for_each(lower, upper, [&](auto const& element)
            {
                Json::Value connectivity(Json::arrayValue);
                for (auto const& nodeId : element.nodalConnectivity)
                {
                    connectivity.append(nodeId);
                }
                elementGroup["NodalConnectivity"].append(connectivity);
            });
            elementGroup["Name"] = mesh.first;
            elementGroup["Type"] = elementTypeId;

            event["Elements"]["Group"].append(elementGroup);
        }
    }

    for (auto const& l2g : localToGlobalMapping)
    {
        event["LocalToGlobalMap"].append(isZeroBased ? l2g - 1 : l2g);
    }
    writer << event;
    writer.close();
}

}
