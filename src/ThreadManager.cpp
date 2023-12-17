#include "ThreadManager.hpp"

// Assigns arugments to member variables containing pointer to a FileManager instance
// and a vector of input files
ThreadManager::ThreadManager(vector<string>* inputFiles) {
    fileManager = FileManager::GetInstance();
    input_files = inputFiles;
}

// Spawns a new map thread for each input file
// Calls join() on each thread to wait for each to terminate
// Deletes any dynamically allocated memory
void ThreadManager::executeMapThreads() {
    for (int i = 0; i < input_files->size(); i++) {
        spawnMapThreads(&input_files->at(i), i + 1);
    }
    joinThreadListThreads();
    deleteThreadListThreads();
}

// Spawns a new Reduce thread for each input file
// Calls join() on each thread to wait for each to terminate
// Deletes any dynamically allocated memory
void ThreadManager::executeReduceThreads() {
    for (int i = 0; i < input_files->size(); i++) {
        spawnReduceThreads(&input_files->at(i), i + 1);
    }
    joinThreadListThreads();
    deleteThreadListThreads();
}

// Creates a new struct threadData pointer for each thread, which stores a unique
// Workflow instance, filename, and threadID. Each thread operates on the static
// threadMap() method, with this struct threadData pointer passed in as an argument.
void ThreadManager::spawnMapThreads(std::string* filename, int threadID) {
    struct threadData* td = new threadData;
    Workflow* workflow = new Workflow(fileManager);
    td->myWorkflow = workflow;
    td->filename = *filename;
    td->threadID = threadID;
    threadDataList.push_back(td);
    threadList.push_back(new boost::thread(threadMap, td));
}

// Creates a new struct threadData pointer for each thread, which stores a unique
// Workflow instance, filename, and threadID. Each thread operates on the static
// threadReduce() method, with this struct threadData pointer passed in as an argument.
void ThreadManager::spawnReduceThreads(std::string* filename, int threadID) {
    struct threadData* td = new threadData;
    Workflow* workflow = new Workflow(fileManager);
    td->myWorkflow = workflow;
    td->filename = *filename;
    td->threadID = threadID;
    threadDataList.push_back(td);
    threadList.push_back(new boost::thread(threadReduce, td));
}

// Each thread will execute this method, which calls Workflow::executeMap()
void ThreadManager::threadMap(void* arg) {
    struct threadData* myStruct = (struct threadData*)arg;
    myStruct->myWorkflow->executeMap(myStruct->filename, myStruct->threadID);
}

// Each thread will execute this method, which calls Workflow::executeReduce()
void ThreadManager::threadReduce(void* arg) {
    struct threadData* myStruct = (struct threadData*)arg;
    myStruct->myWorkflow->executeReduce(myStruct->filename, myStruct->threadID);
}

// Joins all threads so the code waits for each to complete processing
void ThreadManager::joinThreadListThreads() {
    for (int j = 0; j < threadList.size(); j++) {
        threadList[j]->join();
    }
}

// Deletes all dynamically allocated memory from class
void ThreadManager::deleteThreadListThreads() {
    for (int k = 0; k < threadDataList.size(); k++) {
        delete threadDataList[k]->myWorkflow;
        delete threadDataList[k];
    }

    for (int y = 0; y < threadList.size(); y++) {
        delete threadList[y];
    }
}
