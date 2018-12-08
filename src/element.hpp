
#pragma once

#include <cstdint>
#include <vector>

namespace imr
{
class basic_element
{
public:
    explicit basic_element(std::int64_t const index, std::vector<std::int64_t> indices)
        : m_index(index), m_indices(indices)
    {
    }

    /// \return The integer index of the element
    auto index() const -> std::int64_t { return m_index; }

    /// \return A std::vector of integers containing the indices
    auto node_indices() const -> std::vector<std::int64_t> const& { return m_indices; }

    /// \return A std::vector of integers containing the indices
    auto node_indices() -> std::vector<std::int64_t>& { return m_indices; }

protected:
    /// Element index in mesh
    std::int64_t m_index;
    /// List of nodes that make up this element
    std::vector<std::int64_t> m_indices;
};

class element : public basic_element
{
public:
    enum property { physical = 0, geometric };

public:
    explicit element(std::vector<std::int64_t> node_indices,
                     std::vector<int> tags,
                     int const element_type,
                     int const element_index);

    auto type() const -> std::int32_t { return m_element_type; }

    [[deprecated("Use physical_index() instead")]] auto physicalId() const -> std::int32_t
    {
        return m_physical_index;
    }

    [[deprecated("Use geometric_index() instead")]] auto geometricId() const -> std::int32_t
    {
        return m_geometric_index;
    }

    [[deprecated("Use largest_process() instead")]] auto maxProcessId() const -> std::int32_t
    {
        return m_maxProcessId;
    }

    auto physical_index() const -> std::int32_t { return m_physical_index; }

    auto geometric_index() const -> std::int32_t { return m_geometric_index; }

    auto largest_process() const -> std::int32_t { return m_maxProcessId; }

    bool isOwnedByProcess(int const process_number) const noexcept
    {
        return process_number == m_processOwner;
    }

    auto owner_process() const noexcept { return m_processOwner; }

    /// \return True is the element is shared across multiple decompositions
    bool is_shared() const noexcept { return m_is_shared && m_partitionTags[0] > 1; }

    std::vector<int> const& partitionTags() const noexcept { return m_partitionTags; }

    /// Subtract one from each index
    void convertToZeroBasedIndexing();

private:
    std::vector<std::int32_t> m_partitionTags;

    std::int32_t m_physical_index;
    std::int32_t m_geometric_index;
    std::int32_t m_element_type;

    std::int32_t m_maxProcessId = 1;
    std::int32_t m_processOwner = 1;

    bool m_is_shared = false;
};

} // namespace imr
