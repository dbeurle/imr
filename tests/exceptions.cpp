#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include "GmshReader.hpp"
#include "GmshReaderException.hpp"
#include <catch.hpp>

using namespace gmsh;

TEST_CASE("Ensure exceptions are thrown", "[exceptions]")
{
    SECTION("Throw a GmshReaderException for invalid mesh files")
    {
        REQUIRE_THROWS_AS(gmsh::Reader("invalid_file_name",
                                       Reader::NodalOrdering::Global,
                                       Reader::IndexingBase::One),
                          GmshReaderException);
    }
}
TEST_CASE("Tests for serial ElementData", "[ElementData]")
{
    // Setup the data arrays
    std::vector<int> nodalConnectivity{1, 2, 3, 4};
    std::vector<int> tags{2, 4};

    constexpr auto id     = 1;
    constexpr auto typeId = 4;

    ElementData elementData(nodalConnectivity, tags, typeId, id);

    SECTION("Data entry sanity check")
    {
        REQUIRE(elementData.typeId() == typeId);
        REQUIRE(elementData.id() == id);
        REQUIRE(elementData.nodalConnectivity().size() == nodalConnectivity.size());
        REQUIRE(elementData.physicalId() == 2);
        REQUIRE(elementData.geometricId() == 4);
    }
}
TEST_CASE("Tests for decomposed ElementData", "[ElementData]")
{
    //
    REQUIRE(1 == 1);
}
