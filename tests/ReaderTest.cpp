#define CATCH_CONFIG_MAIN

#include "mesh_reader.hpp"

#include <catch2/catch.hpp>

using namespace imr;

TEST_CASE("Ensure exceptions are thrown")
{
    SECTION("Throw a GmshReaderException for invalid mesh files")
    {
        REQUIRE_THROWS_AS(mesh_reader("invalid_file_name",
                                      NodalOrdering::Global,
                                      IndexingBase::One,
                                      distributed::feti),
                          std::domain_error);
    }
}
TEST_CASE("Tests for basic ElementData")
{
    // gmsh element line definition is
    // 1 3 2 4 16 2 14 22 18
    std::vector<std::int64_t> node_indices{2, 14, 22, 18};
    std::vector<int> tags{4, 16};

    constexpr auto id = 1, typeId = 3;

    element elementData(node_indices, tags, typeId, id);

    SECTION("Data entry sanity check")
    {
        REQUIRE(elementData.id() == id);
        REQUIRE(elementData.typeId() == typeId);
        REQUIRE(elementData.node_indices().size() == node_indices.size());
        REQUIRE(elementData.physicalId() == 4);
        REQUIRE(elementData.geometricId() == 16);
    }
    SECTION("Check decomposition is disabled")
    {
        REQUIRE(!elementData.isSharedByMultipleProcesses());
        REQUIRE(elementData.isOwnedByProcess(1));
        REQUIRE(elementData.maxProcessId() == 1);
        REQUIRE(elementData.partitionTags().empty());
    }
    SECTION("Check zero based indexing method")
    {
        elementData.convertToZeroBasedIndexing();
        for (std::size_t i = 0; i < node_indices.size(); ++i)
        {
            REQUIRE(node_indices[i] - 1 == elementData.node_indices()[i]);
        }
        REQUIRE(elementData.id() == 0);
    }
}
TEST_CASE("Tests for decomposed ElementData")
{
    // 1 3 5 999 1 2 3 -4 402 233 450 197
    constexpr auto id = 1, typeId = 3;
    std::vector<std::int64_t> node_indices{402, 233, 450, 197};
    std::vector<int> tags{999, 1, 2, 3, -4};

    element elementData(node_indices, tags, typeId, id);

    REQUIRE(elementData.partitionTags().size() == 3);

    REQUIRE(elementData.isSharedByMultipleProcesses());

    REQUIRE(elementData.isOwnedByProcess(3));
    REQUIRE(elementData.maxProcessId() == 4);
}
TEST_CASE("Tests for Reader")
{
    mesh_reader reader("decomposed.msh",
                       NodalOrdering::Local,
                       IndexingBase::Zero,
                       distributed::feti);

    REQUIRE(reader.numberOfPartitions() == 4);

    // Check the physical names are in the map
    REQUIRE(reader.names().find(1) != end(reader.names()));
    // REQUIRE(reader.names().find(2) != reader.names().end());

    REQUIRE(reader.names().find(1)->second == "domain");
    // REQUIRE(reader.names().find(2)->second == "left_boundary");

    REQUIRE(reader.nodes().size() == 9);

    reader.write(false);
}
