///////////////////////////////////////////////////////////////////////
//  Workflow.cpp    -   manages the execution of the entire pipeline //
//  Language        -   C++                                          //
///////////////////////////////////////////////////////////////////////
#include "Workflow.hpp"

#include <vector>
using std::vector;

#include <ctime>
#include <map>
#include <regex>
#include <boost/thread/mutex.hpp>

#include "Map.hpp"
#include "Reduce.hpp"

boost::mutex cout_mutex;

using namespace std;

#ifdef DEBUG
#define DEBUG_MSG(str)                    \
    do {                                  \
        cout << "DEBUG: " << str << endl; \
    } while (false)
#else
#define DEBUG_MSG(str) void() 0  // treat DEBUG_MSG as a no-op if not in DEBUG mode
#endif

Workflow::Workflow(FileManager* filemgr) {
    fileManager = filemgr;
}

/*
void Workflow::execute() {
    input_files = fileManager->getDirectoryFileList(fileManager->getInputDirectory());

#ifdef DEBUG
    string debugmsg = "All Files in Input Directory:\n";
    for (string i : input_files) {
        debugmsg.append("\t");
        debugmsg.append(i);
        debugmsg.append("\n");
    }
    DEBUG_MSG(debugmsg);
#endif

    
    

    // Maps all the files:
    std::cout << "[+] Starting Mapper to parse input files..." << endl;

    Map mapper;
    for (string currfile : input_files) {
        vector<string> contents;
        try {
            contents = fileManager->readFile(currfile);
        } catch (exception& e) {
            cout << "File '" << currfile << "' could not be opened; skipping." << endl;
            skippedFiles.push_back(currfile);
            continue;
        }
        if (contents.size() == 0) {
            cout << "File '" << currfile << "' was empty; no mapping done." << endl;
            skippedFiles.push_back(currfile);
            continue;
        }
        int wordcount = 0;
        for (string currline : contents) {
            wordcount += mapper.map(fileManager->getFileStem(currfile + ".txt"), currline);
        }
#ifdef DEBUG
        DEBUG_MSG("Mapper tokenized " + to_string(wordcount) + " words from " + currfile);
#endif
        mapper.flushBuffer();
    }
    Executor* execPtr = &mapper;
    std::string message = execPtr->toString();
    cout << message << endl;


    std::cout << "[+] Sorting and reducing tokens in intermediate files..." << endl;

#ifdef DEBUG
    time_t start_time;
    time_t end_time;
#endif
    std::vector<std::string> input_files = this->fileManager->getDirectoryFileList(this->fileManager->getTempDirectory());
    for (int i = 0; i < input_files.size(); i++) {
#ifdef DEBUG
        DEBUG_MSG("Processing temporary file " + input_files[i]);
        time(&start_time);
#endif
        // Initialize Reducer with output file name
        Reduce reducer = Reduce(fileManager->getFileStem(input_files[i]) + ".txt");

        reducer.execute(input_files[i]);

        // Log the output filename
        Executor* executor = &reducer;
        cout << "Reducer finished processing " + executor->toString() << endl;

#ifdef DEBUG
        time(&end_time);
        DEBUG_MSG("File " + input_files[i] + " complete. (" + to_string(i + 1) + "/" + to_string(input_files.size()) + ") Time: " + to_string(end_time - start_time) + " sec");
#endif
    }
    std::cout << "[+] Completed sorting and reducing tokens." << endl;
    std::cout << "[+] Workflow complete." << endl;
}

*/


void Workflow::executeMap(std::string filename, int threadID){
    Map mapper;
    vector<string> contents;
    try {
        contents = fileManager->readFile(filename);
    } catch (exception& e) {
        cout_mutex.lock();
        std::cout << "File '" << filename << "' could not be opened; skipping." << endl;
        cout_mutex.unlock();
        skippedFiles.push_back(filename);
    }
    if (contents.size() == 0) {
        cout_mutex.lock();
        std::cout << "File '" << filename << "' was empty; no mapping done." << endl;
        cout_mutex.unlock();
        skippedFiles.push_back(filename);
    }
    cout_mutex.lock();
    std::cout << "Thread " << threadID << " is mapping file " << filename << std::endl;
    cout_mutex.unlock();
    int wordcount = 0;
    for (string currline : contents) {
        wordcount += mapper.map(fileManager->getFileStem(filename + ".txt"), currline);
    }
#ifdef DEBUG
        DEBUG_MSG("Mapper tokenized " + to_string(wordcount) + " words from " + currfile);
#endif
        mapper.flushBuffer();
}