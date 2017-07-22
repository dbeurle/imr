
#pragma once

#include <vector>

namespace gmsh
{
struct ElementData
{
    ElementData(int numberOfNodes, int numberOfTags, int typeId, int id);

    int maxProcessId() const;

    bool isOwnedByProcess(int processId) const;

    /** @return true if this element is shared across multiple paritions */
    bool isSharedByMultipleProcesses() const { return tags.size() > 2 && tags[2] > 1; }

    /** Subtract one from each nodal connectivity index */
    void convertToZeroBasedIndexing();

    std::vector<int> tags; // Position in tag array
                           // 0 - Physical id
                           // 1 - Geometrical id
                           // 2 - Number of processes element belongs to
                           // 3 - If tags[2] > 1 then owner
                           // 4... - Ghost element processes (shared by processes)
    std::vector<int> nodalConnectivity;
    int typeId;
    int id;
};

inline void ElementData::convertToZeroBasedIndexing()
{
    for (auto& node : nodalConnectivity) --node;
    --id;
}
}
