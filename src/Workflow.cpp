///////////////////////////////////////////////////////////////////////
//  Workflow.cpp    -   manages the execution of the entire pipeline //
//  Language        -   C++                                          //
///////////////////////////////////////////////////////////////////////
#include "Workflow.hpp"

#include <vector>
using std::vector;

#include <boost/thread/mutex.hpp>
#include <ctime>
#include <map>
#include <regex>

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

void Workflow::executeMap(std::string filename, int threadID) {
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

void Workflow::executeReduce(std::string filename, int threadID) {
    // Initialize Reducer with output file name
    Reduce reducer = Reduce(fileManager->getFileStem(filename + ".txt"));

    cout_mutex.lock();
    std::cout << "Thread " << threadID << " is reducing file " << filename << std::endl;
    cout_mutex.unlock();

    reducer.execute(filename);
}
