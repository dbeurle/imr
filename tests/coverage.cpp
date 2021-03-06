#define CATCH_CONFIG_RUNNER

#include <catch.hpp>

#include "mesh_reader.hpp"
#include "GmshReaderException.hpp"

int main()
{
    using namespace imr;
    Reader reader("basic.msh", Reader::NodalOrdering::Global, Reader::IndexingBase::One);
}
