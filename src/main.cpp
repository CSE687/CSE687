#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>

#include "Stub.hpp"
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

        // create client socket
        Stub stub1(9000);
        // if (!stub1.fail_status()) {  // if status is 0, then we are clear to execute
        stub1();
        // }
    }
    return 0;
}
