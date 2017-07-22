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
TEST_CASE("Test suite for ElementData", "[ElementData]")
{
    // Setup the data arrays
    ElementData elementData(4, 3, 4, 1);

    SECTION("Data entry sanity check")
    {
        REQUIRE(elementData.typeId == 4);
        REQUIRE(elementData.id == 1);
        REQUIRE(elementData.nodalConnectivity.size() == 4);
    }
}
