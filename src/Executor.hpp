#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "FileManager.hpp"

class Executor {
   public:
    std::string outputFilename;
    FileManager *fileManager;

    // constant buffer size for each instance of Executor
    const int bufferSize = 1024;

    // Buffer to store data before being written to disk
    std::string buffer;

    virtual void toString() = 0;
};

#endif  // EXECUTOR_HPP
