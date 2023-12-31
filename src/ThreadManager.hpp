#ifndef ThreadManager_H
#define ThreadManager_H

/*
The ThreadManager class abstracts all thread operations into a class. The constructor takes a pointer to a FileManager
instance and a pointer to a vector containing all input files to be mapped. The executeMapThreads() method loops over
this vector and spawns a map thread for each input file. The threads all operate on the executeMap() method of the
Workflow class. Once each thread is spawned, the joinThreadListThreads() waits for each to complete before the deleteThreadListThreads()
method frees up all dynamically allocated memory.
*/

#include <boost/thread.hpp>

#include "Executor.hpp"
#include "FileManager.hpp"
#include "Workflow.hpp"

class ThreadManager {
   public:
    // class constructor
    ThreadManager(vector<string>*);

    // Loops over input file vector, spawning a new map thread for each file. Also
    // calls join on each thread and frees up all dynamic memory
    void executeMapThreads();

    // Calls threadMap for each thread created
    void spawnMapThreads(std::string*, int threadID);

    // Each thread executes the Workflow::executeMap method
    static void threadMap(void* arg);

    // Calls join() on each thread to wait for each to complete
    void joinThreadListThreads();

    // Frees up any dynamically allcoated memory
    void deleteThreadListThreads();

    // Loops over input file vector, spawning a new Reduce thread for each file. Also
    // calls join on each thread and frees up all dynamic memory
    void executeReduceThreads();

    // Calls threadReduce for each thread created
    void spawnReduceThreads(std::string*, int threadID);

    // Each thread executes the Workflow::executeReduce method
    static void threadReduce(void* arg);

    // Unique data for each thread
    struct threadData {
        Workflow* myWorkflow;
        string filename;
        int threadID;
    };

    // Singleton instance of File Manager class
    FileManager* fileManager;

    // Pointer to vector of input files, stored as strings
    vector<string>* input_files;

    // Vector of thread pointers
    vector<boost::thread*> threadList;

    // Vector of pointers to threadData structs
    vector<ThreadManager::threadData*> threadDataList;
};
#endif
