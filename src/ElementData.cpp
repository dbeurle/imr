
#include "ElementData.hpp"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace imr
{
ElementData::ElementData(std::vector<int> nodalConnectivity,
                         std::vector<int> tags,
                         int const typeId,
                         int const id)
    : m_nodalConnectivity(nodalConnectivity), m_typeId(typeId), m_id(id)
{
    if (tags.empty())
    {
        throw std::runtime_error("Element tags vector not filled\n");
    }
    if (nodalConnectivity.empty())
    {
        throw std::runtime_error("Nodal connectivity vector not filled\n");
    }

    // Position in tag array
    // 0 - Physical id
    // 1 - Geometrical id
    // 2 - Number of processes element belongs to
    // 3 - If tags[2] > 1 then owner
    // 4... - Ghost element processes (shared by processes)
    m_physicalId  = tags[Property::Physical];
    m_geometricId = tags[Property::Geometric];

    m_isElementShared = tags.size() > 2;

    if (m_isElementShared)
    {
        // Pull out the ending tags and populate the partitionTags vector
        auto const shared_between = tags[2];
        m_partitionTags.reserve(shared_between);

        for (auto i = 2; i < tags.size(); i++) m_partitionTags.push_back(tags[i]);

        m_maxProcessId = std::abs(*std::max_element(m_partitionTags.begin(),
                                                    m_partitionTags.end(),
                                                    [](auto const a, auto const b) {
                                                        return std::abs(a) < std::abs(b);
                                                    }));

        m_processOwner = m_partitionTags[1];
    }
}
} // namespace imr
