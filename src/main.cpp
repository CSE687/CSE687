#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>

#include "ThreadManager.hpp"
#include "Workflow.hpp"

using namespace std;

void help() {
    cout << "INPUT ERROR: Your inputs for the program should be as follows: program.out <1> <2> <3>\n";
    cout << "1 - Input Directory\n2 - Output Directory\n3 - Temp Directory for Intermediates\n";
    cout << "Please try running again in the following format.\n";
}

int main(int argc, char* argv[]) {
    if (argv[1] == "-h" || argc != 4) {
        help();
    } else {
        // Initialize File Manager
        FileManager* filemanager = nullptr;
        try {
            filemanager = FileManager::GetInstance(argv[1], argv[2], argv[3]);
        } catch (exception const& exc) {
            cerr << "Could not start FileManager: " << exc.what() << endl;
            help();
        }

        // Obtain vector of input files
        vector<string> input_files = filemanager->getDirectoryFileList(filemanager->getInputDirectory());

        // Initialize Thread Manager and launch map threads
        ThreadManager mapThreadMang(filemanager, &input_files);
        mapThreadMang.executeMapThreads();

        // Obtain vector of input files
        vector<string> temp_files = filemanager->getDirectoryFileList(filemanager->getTempDirectory());

        // Initialize Thread Manager and launch map threads
        ThreadManager reduceThreadMang(filemanager, &temp_files);
        reduceThreadMang.executeReduceThreads();
    }
    return 0;
}
