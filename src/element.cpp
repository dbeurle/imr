
#include "element.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace imr
{
element::element(std::vector<std::int64_t> node_indices,
                 std::vector<std::int32_t> tags,
                 int const element_type,
                 int const element_index)
    : basic_element(element_index, std::move(node_indices)), m_element_type(element_type)
{
    if (tags.empty())
    {
        throw std::runtime_error("Element tags vector not filled\n");
    }

    if (m_indices.empty())
    {
        throw std::runtime_error("Node indices vector not filled\n");
    }

    // Position in tag array
    // 0 - Physical id
    // 1 - Geometrical id
    // 2 - Number of processes element belongs to
    // 3 - If tags[2] > 1 then owner
    // 4... - Ghost element processes (shared by processes)
    m_physical_index  = tags[property::physical];
    m_geometric_index = tags[property::geometric];

    m_is_shared = tags.size() > 2;

    if (m_is_shared)
    {
        // Pull out the ending tags and populate the partitionTags vector
        auto const shared_between = tags[2];
        m_partitionTags.reserve(shared_between);

        for (auto i = 2; i < tags.size(); i++)
        {
            m_partitionTags.push_back(tags[i]);
        }

        m_maxProcessId = std::abs(*std::max_element(begin(m_partitionTags),
                                                    end(m_partitionTags),
                                                    [](auto const a, auto const b) {
                                                        return std::abs(a) < std::abs(b);
                                                    }));

        m_processOwner = m_partitionTags[1];
    }
}

void element::convertToZeroBasedIndexing()
{
    std::transform(begin(m_indices), end(m_indices), begin(m_indices), [](auto const index) {
        return index - 1;
    });
    --m_index;
}

} // namespace imr
