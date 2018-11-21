#define CATCH_CONFIG_MAIN

#include "mesh_reader.hpp"
#include "GmshReaderException.hpp"

#include <catch2/catch.hpp>

using namespace imr;

TEST_CASE("Ensure exceptions are thrown")
{
    SECTION("Throw a GmshReaderException for invalid mesh files")
    {
        REQUIRE_THROWS_AS(Reader("invalid_file_name",
                                 Reader::NodalOrdering::Global,
                                 Reader::IndexingBase::One),
                          std::domain_error);
    }
}
TEST_CASE("Tests for basic ElementData")
{
    // gmsh element line definition is
    // 1 3 2 4 16 2 14 22 18
    std::vector<int> nodalConnectivity{2, 14, 22, 18};
    std::vector<int> tags{4, 16};

    constexpr auto id = 1, typeId = 3;

    ElementData elementData(nodalConnectivity, tags, typeId, id);

    SECTION("Data entry sanity check")
    {
        REQUIRE(elementData.id() == id);
        REQUIRE(elementData.typeId() == typeId);
        REQUIRE(elementData.nodalConnectivity().size() == nodalConnectivity.size());
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
        for (int i = 0; i < nodalConnectivity.size(); ++i)
        {
            REQUIRE(nodalConnectivity[i] - 1 == elementData.nodalConnectivity()[i]);
        }
        REQUIRE(elementData.id() == 0);
    }
}
TEST_CASE("Tests for decomposed ElementData")
{
    // 1 3 5 999 1 2 3 -4 402 233 450 197
    constexpr auto id = 1, typeId = 3;
    std::vector<int> nodalConnectivity{402, 233, 450, 197};
    std::vector<int> tags{999, 1, 2, 3, -4};

    ElementData elementData(nodalConnectivity, tags, typeId, id);

    REQUIRE(elementData.partitionTags().size() == 3);

    REQUIRE(elementData.isSharedByMultipleProcesses());

    REQUIRE(elementData.isOwnedByProcess(3));
    REQUIRE(elementData.maxProcessId() == 4);
}
TEST_CASE("Tests for Reader")
{
    Reader reader("decomposed.msh", Reader::NodalOrdering::Local, Reader::IndexingBase::Zero);

    REQUIRE(reader.numberOfPartitions() == 4);

    // Check the physical names are in the map
    REQUIRE(reader.names().find(1) != reader.names().end());
    // REQUIRE(reader.names().find(2) != reader.names().end());

    REQUIRE(reader.names().find(1)->second == "domain");
    // REQUIRE(reader.names().find(2)->second == "left_boundary");

    REQUIRE(reader.nodes().size() == 9);

    reader.writeMeshToJson(false);
}
