#include <boost/log/trivial.hpp>

#include "FileManager.hpp"
#include "Reduce.hpp"
#include "Workflow.hpp"

void help() {
    std::cout << "INPUT ERROR: Your inputs for the program should be as follows: program.out <1> <2> <3>\n";
    std::cout << "1 - Input Directory\n2 - Output Directory\n3 - Temp Directory for Intermediates\n";
    std::cout << "Please try running again in the following format.\n";
}

int main(int argc, char* argv[]) {
    if (argv[1] == "-h" || argc != 4) {
        help();
    } else {
        // Initialize File Manager
        FileManager* filemanager = FileManager::GetInstance(argv[1], argv[2], argv[3]);

        if (filemanager->checkDirectoryExists(filemanager->getInputDirectory())) {
            // Initialize & Execute Workflow
            Workflow workflow = Workflow(argv[1], argv[2], argv[3]);
            workflow.execute();
        } else {
            std::cout << "Directory " << filemanager->getInputDirectory() << " does not exist.\n\n";
            help();
            std::cout << std::endl;
        }
    }

    // std::vector<int> values = {1, 2, 3};
    // std::string output_dir = "some/path/somewhere";
    // reduce.reduce("Hello", values);

    return 0;
}
