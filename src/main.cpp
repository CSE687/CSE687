#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <cstring>
#include <vector>

#include "Controller.hpp"
#include "Stub.hpp"
#include "ThreadManager.hpp"
#include "Workflow.hpp"

using namespace std;

void help() {
    cout << "INPUT ERROR: Your input for the program should be one of the following: \n";
    cout << "stub <port>\n";
    cout << "controller <controller-port> <1> <2> <3> <port, port, ...>\n";
    cout << "  - <1>: Input Directory\n";
    cout << "  - <2>: Output Directory\n";
    cout << "  - <3>: Temp Directory for Intermediates\n";
    cout << "  - List of ports for stub connections\n";
    cout << "Please try running again with an above format.\n";
}

int main(int argc, char* argv[]) {
    // Check for correct number of arguments
    bool CLICheckController = strcmp(argv[1], "controller") == 0 && argc >= 7;
    bool CLICheckStub = strcmp(argv[1], "stub") == 0 && argc == 3;

    // Check for help flag or incorrect input
    if (strcmp(argv[1], "-h") == 0 || !(CLICheckController || CLICheckStub)) {
        help();
        exit(1);
    }

    if (strcmp(argv[1], "stub") == 0) {
        cout << "Starting Program: " << argv[1] << endl;
        cout << "Starting Listen Server on Port " << argv[2] << endl;
        Stub stub(atoi(argv[2]));
        stub.startListening();
    } else if (strcmp(argv[1], "controller") == 0) {
        cout << "Starting Program: " << argv[1] << endl;
        // Initialize File Manager
        FileManager* filemanager = nullptr;
        try {
            filemanager = FileManager::GetInstance(argv[3], argv[4], argv[5]);
        } catch (exception const& exc) {
            cerr << "Could not start FileManager: " << exc.what() << endl;
            help();
            exit(1);
        }

        // Collect argv[5] and beyond as ports
        std::vector<int> ports;
        for (int i = 6; i < argc; i++) {
            ports.push_back(atoi(argv[i]));
        }

        // Start Controller
        Controller controller(filemanager, atoi(argv[2]), ports);
        controller.execute();
    }

    return 0;
}
