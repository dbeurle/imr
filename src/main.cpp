
#include "GmshReader.hpp"

#include <boost/program_options.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    // Parse the command line options and process the mesh
    try
    {
        // Define and parse the program options
        namespace po = boost::program_options;
        po::options_description desc("Options");

        desc.add_options()("help", "Print help messages");

        desc.add_options()("zero-based",
                           "Use zero based indexing for elements and nodes");

        desc.add_options()("local-ordering",
                           "For distributed meshes, each processor has local indexing "
                           "and a local to global mapping");

        desc.add_options()("no-indices", "Do not write out extra indices to save space");

        po::variables_map vm;

        try
        {
            po::store(po::parse_command_line(argc, argv, desc), vm);

            // --help option
            if (vm.count("help"))
            {
                std::cout << "\nA serial and parallel gmsh processing tool to convert "
                             ".msh to .json files for easy input\n\n"
                          << desc << std::endl;
                return 0;
            }
            po::notify(vm); // throws on error, so do after help in case
                            // there are any problems
        }
        catch (po::error& e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return 1;
        }

        // application code here //
    }
    catch (std::exception& e)
    {
        std::cerr << "Unhandled Exception reached the top of main: " << e.what()
                  << ", application will now exit" << std::endl;
        return 1;
    }
    return 0;
}
