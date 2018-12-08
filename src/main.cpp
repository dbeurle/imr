
#include "reader_factory.hpp"

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

// Define and parse the program options
static po::options_description create_options()
{

    po::options_description visible("Options");

    visible.add_options()("help", "Print help messages");

    visible.add_options()("zero-based",
                          "Use zero based indexing for elements and nodes.  Default: "
                          "one-based.");

    visible.add_options()("local-ordering",
                          "For distributed meshes, each processor has local indexing "
                          "and a local to global mapping.  Default: global-ordering");

    visible.add_options()("with-indices",
                          "Write out extra indices (results in file size increase).  "
                          "Default without-indices");

    visible.add_options()("interprocess-format",
                          "Write out shared process interfaces (only for decomposed meshes).  "
                          "Default feti-format");

    return visible;
}

int main(int argc, char* argv[])
{
    using namespace imr;

    // Parse the command line options and process the mesh
    try
    {
        auto visible = create_options();

        po::options_description hidden("Hidden options");

        hidden.add_options()("input-file", po::value<std::vector<std::string>>(), "input file");

        po::options_description cmdline_options;
        cmdline_options.add(visible).add(hidden);

        po::positional_options_description p;
        p.add("input-file", -1);

        po::variables_map vm;

        try
        {
            po::store(
                po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(),
                vm);

            if (vm.count("help") || argc < 2)
            {
                std::cout << "\nA serial and parallel gmsh processing tool to convert "
                             ".msh to .json files\n\n"
                          << "imr [Options] filename.msh\n\n"
                          << visible << std::endl;
                return 0;
            }
            po::notify(vm); // throws on error, so do after help in case
                            // there are any problems
        }
        catch (po::error& error_message)
        {
            std::cerr << "ERROR: " << error_message.what() << "\n" << std::endl;
            std::cerr << visible << std::endl;
            return 1;
        }

        index_base const indexing = vm.count("zero-based") > 0 ? index_base::zero : index_base::one;

        nodal_order const ordering = vm.count("local-ordering") > 0 ? nodal_order::local
                                                                    : nodal_order::global;

        distributed const distributed_option = vm.count("interprocess-format") > 0
                                                   ? distributed::interprocess
                                                   : distributed::feti;

        std::cout << "\nPerforming mesh conversion with "
                  << (indexing == index_base::zero ? "zero" : "one")
                  << " based indexing for node indices\n\n";

        if (vm.count("input-file"))
        {
            for (auto const& input : vm["input-file"].as<std::vector<std::string>>())
            {
                auto const reader = make_reader(input, ordering, indexing, distributed_option);
                reader->parse();
                reader->write();
            }
        }
        else
        {
            throw std::runtime_error("Missing the input file!\n");
        }
    }
    catch (std::exception& error)
    {
        std::cerr << "Unhandled Exception reached the top of main: " << error.what()
                  << ", application will now exit" << std::endl;
        return 1;
    }
    return 0;
}
