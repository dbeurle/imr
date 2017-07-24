
#include "GmshReader.hpp"

#include <boost/program_options.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    using namespace gmsh;

    // Parse the command line options and process the mesh
    try
    {
        // Define and parse the program options
        namespace po = boost::program_options;

        po::options_description visible("Options");

        visible.add_options()("help", "Print help messages");
        visible.add_options()("zero-based",
                              "Use zero based indexing for elements and nodes");
        visible.add_options()("local-ordering",
                              "For distributed meshes, each processor has local indexing "
                              "and a local to global mapping");
        visible.add_options()("with-indices",
                              "Write out extra indices (results in file size increase)");

        po::options_description hidden("Hidden options");

        hidden.add_options()("input-file",
                             po::value<std::vector<std::string>>(),
                             "input file");

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;

        try
        {
            po::store(po::command_line_parser(argc, argv)
                          .options(cmdline_options)
                          .positional(p)
                          .run(),
                      vm);

            if (vm.count("help") || argc < 2)
            {
                std::cout << "\nA serial and parallel gmsh processing tool to convert "
                             ".msh to .json files\n\n"
                          << "gmshreader [Options] filename.msh\n\n"
                          << visible << std::endl;
                return 0;
            }
            po::notify(vm); // throws on error, so do after help in case
                            // there are any problems
        }
        catch (po::error& e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << visible << std::endl;
            return 1;
        }

        Reader::IndexingBase indexing = vm.count("zero-based") > 1
                                            ? Reader::IndexingBase::Zero
                                            : Reader::IndexingBase::One;

        Reader::NodalOrdering ordering = vm.count("local-ordering") > 1
                                             ? Reader::NodalOrdering::Local
                                             : Reader::NodalOrdering::Global;

        if (vm.count("input-file"))
        {
            for (auto const& input : vm["input-file"].as<std::vector<std::string>>())
            {
                Reader reader(input, ordering, indexing);
                reader.writeMeshToJson(vm.count("with-indices") > 1);
            }
        }
        else
        {
            throw std::runtime_error("Missing \".msh\" input file!\n");
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Unhandled Exception reached the top of main: " << e.what()
                  << ", application will now exit" << std::endl;
        return 1;
    }
    return 0;
}
