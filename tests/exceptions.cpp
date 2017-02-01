#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file
#include "GmshReader.hpp"
#include "GmshReaderException.hpp"
#include <catch.hpp>

using namespace gmsh;

TEST_CASE("Check for exceptions", "[exceptions]")
{
    SECTION("Throw a GmshReaderException for invalid mesh files")
    {
        REQUIRE_THROWS_AS(gmsh::Reader("invalid_file_name",
                                       Reader::NodalOrdering::Global,
                                       Reader::IndexingBase::One),
                          GmshReaderException);
    }
}
