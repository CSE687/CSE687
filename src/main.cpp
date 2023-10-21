#include "FileManager/FileManager.hpp"

#include <boost/log/trivial.hpp>

#include "Reduce.h"

void help() {
    std::cout << "INPUT ERROR: Your inputs for the program should be as follows: program.out <1> <2> <3>\n";
    std::cout <<  "1 - Input Directory\n2 - Output Directory\n3 - Temp Directory for Intermediates\n";
    std::cout << "Please try running again in the following format.\n";
}

int main(int argc, char *argv[])
{
    if (argv[1] == "-h" || argc != 4) {
        help();
    }
    else {
        FileManager filemanager(argv[1], argv[2], argv[3]);

        if(filemanager.checkDirectoryExists(filemanager.getInputDirectory())) {
            std::vector<std::string> input_files = filemanager.getDirectoryFileList(filemanager.getInputDirectory());
            std::cout << "All Files in Input Directory:\n";
            for (std::string i: input_files) {
                std::cout << "\t" << i << std::endl;
            }
            std::cout << std::endl << std::endl;

        } else {
            std::cout << "Directory " << filemanager.getInputDirectory() << " does not exist.\n\n";
            help();
            std::cout << std::endl;
        }
    }


    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    // std::vector<int> values = {1, 2, 3};
    // std::string output_dir = "some/path/somewhere";
    // Executive executer;
    // executer.execute(argc, argv);

    // reduce.reduce("Hello", values);

    return 0;
}
