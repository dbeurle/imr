
#pragma once

#include <cstdint>
#include <vector>

namespace imr
{
class element
{
public:
    enum Property { Physical = 0, Geometric };

public:
    explicit element(std::vector<std::int64_t>&& node_indices,
                     std::vector<int> tags,
                     int const typeId,
                     int const id);

    int id() const noexcept { return m_id; }

    int typeId() const noexcept { return m_typeId; }

    int physicalId() const noexcept { return m_physicalId; }

    int geometricId() const noexcept { return m_geometricId; }

    int maxProcessId() const noexcept { return m_maxProcessId; }

    bool isOwnedByProcess(int const processId) const noexcept
    {
        return processId == m_processOwner;
    }

    auto owner_process() const noexcept { return m_processOwner; }

    bool isSharedByMultipleProcesses() const noexcept
    {
        return m_isElementShared && m_partitionTags[0] > 1;
    }

    std::vector<std::int64_t> const& nodalConnectivity() const noexcept { return m_indices; }

    std::vector<std::int64_t>& nodalConnectivity() noexcept { return m_indices; }

    std::vector<int> const& partitionTags() const noexcept { return m_partitionTags; }

    /// Subtract one from each index
    void convertToZeroBasedIndexing();

private:
    std::vector<int> m_partitionTags;
    std::vector<std::int64_t> m_indices;

    int m_physicalId;
    int m_geometricId;
    int m_typeId;
    int m_id;

    int m_maxProcessId = 1;
    int m_processOwner = 1;

    bool m_isElementShared = false;
};

inline void element::convertToZeroBasedIndexing()
{
    for (auto& node : m_indices)
    {
        --node;
    }
    --m_id;
}
} // namespace imr
