#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include "GmshReader.hpp"
#include "GmshReaderException.hpp"

TEST_CASE( "Check for exceptions", "[exceptions]" )
{
    SECTION( "Throw a GmshReaderException for invalid mesh files" )
    {
        REQUIRE_THROWS_AS(neon::GmshReader reader("invalid_file_name"), GmshReaderException);
    }

}
