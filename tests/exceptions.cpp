#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include "GmshReader.hpp"
#include "GmshReaderException.hpp"

TEST_CASE( "Check for exceptions", "[exceptions]" )
{
    SECTION( "Throw a GmshReaderException for invalid mesh files" )
    {
        using namespace gmsh;
        REQUIRE_THROWS_AS(gmsh::Reader reader("invalid_file_name",
                                              Reader::NodalOrdering::Global,
                                              Reader::IndexingBase::One),
                          GmshReaderException);
    }
}
