
#pragma once

#include <vector>

namespace gmsh
{
class ElementData
{
public:
    explicit ElementData(std::vector<int> nodalConnectivity,
                         std::vector<int> tags,
                         int typeId,
                         int id);

    int id() const { return m_id; }

    int typeId() const { return m_typeId; }

    int physicalId() const { return m_physicalId; }

    int geometricId() const { return m_geometricId; }

    int maxProcessId() const { return m_maxProcessId; }

    bool isOwnedByProcess(int processId) const { return processId == m_processOwner; }

    bool isSharedByMultipleProcesses() const
    {
        return m_isElementShared && m_partitionTags[0] > 1;
    }

    std::vector<int> const& nodalConnectivity() const { return m_nodalConnectivity; }

    std::vector<int>& nodalConnectivity() { return m_nodalConnectivity; }

    std::vector<int> const& partitionTags() const { return m_partitionTags; }

    /** Subtract one from each nodal connectivity index */
    void convertToZeroBasedIndexing();

private:
    std::vector<int> m_partitionTags;
    std::vector<int> m_nodalConnectivity;

    int m_physicalId;
    int m_geometricId;
    int m_typeId;
    int m_id;
    int m_maxProcessId;
    int m_processOwner;

    bool m_isElementShared = false;
};

inline void ElementData::convertToZeroBasedIndexing()
{
    for (auto& node : m_nodalConnectivity) --node;
    --m_id;
}
}
