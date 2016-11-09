
#include <numeric>
#include <algorithm>

#include "ElementData.hpp"

namespace gmsh
{
ElementData::ElementData(int numberOfNodes, int numberOfTags, int typeId, int id) :
    tags(numberOfTags, 0),
    nodalConnectivity(numberOfNodes, 0),
    typeId(typeId),
    id(id)
{
}

int ElementData::maxProcessId() const
{
    if (tags.size() == 2) return 1;
    return std::abs(*std::max_element(std::next(tags.begin(), 3),
                                      std::next(tags.begin(), 3+*(tags.begin()+2)),
                                      [](auto a, auto b)
                                      {
                                          return std::abs(a) < std::abs(b);
                                      }));
}

bool ElementData::isOwnedByProcess(int processId) const
{
    if (tags.size() == 2) return true;
    // Search the tags for the process value
    auto procHead = std::next(tags.begin(), 3);
    auto procTail = std::next(procHead, *(tags.begin()+2));
    return std::find(procHead, procTail, processId) != procTail;
}
}
