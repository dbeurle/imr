#define CATCH_CONFIG_RUNNER
#include <catch.hpp>
#include "GmshReader.hpp"
#include "GmshReaderException.hpp"


int main()
{
    using namespace gmsh;
    Reader reader("basic.msh", Reader::NodalOrdering::Global, Reader::IndexingBase::One);
}
