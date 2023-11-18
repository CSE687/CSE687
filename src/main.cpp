#include <boost/log/trivial.hpp>
#include "Workflow.hpp"
#include <boost/thread.hpp>

using namespace std;

void help() {
    cout << "INPUT ERROR: Your inputs for the program should be as follows: program.out <1> <2> <3>\n";
    cout << "1 - Input Directory\n2 - Output Directory\n3 - Temp Directory for Intermediates\n";
    cout << "Please try running again in the following format.\n";
}

struct threadData {
    Workflow* myWorkflow;
    string filename;
    int threadID;
};

void consume(void* arg) {
    struct threadData* myStruct = (struct threadData*)arg;
    myStruct->myWorkflow->executeMap(myStruct->filename, myStruct->threadID);
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

        vector<string> input_files;
        input_files = filemanager->getDirectoryFileList(filemanager->getInputDirectory());

        // This code works for a single thread
        /*
        struct threadData td;
        td.myWorkflow = &workflow;
        td.filename = input_files[0];
        td.threadID = 1;

        boost::thread myThread(consume, &td);

        myThread.join();
        
        std::cout << "This line will be printed after thread ends.\n";
        */

        vector<boost::thread*> threadList;
        vector<threadData*> threadDataList;
        // Create threads for each input file
        for (int i = 0; i < input_files.size(); i++){
            struct threadData* td = new threadData;
            Workflow *workflow = new Workflow(filemanager);
            td->myWorkflow = workflow;
            td->filename = input_files[i];
            td->threadID = i+1;
            threadDataList.push_back(td);
            threadList.push_back(new boost::thread(consume, td));
        }

        for (int j = 0; j < threadList.size(); j++){
            threadList[j]->join();
        }
        
        for (int k = 0; k < threadDataList.size(); k++){
            delete threadDataList[k]->myWorkflow;
            delete threadDataList[k];
        }
    }
    return 0;
}
