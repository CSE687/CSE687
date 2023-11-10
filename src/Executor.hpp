#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "FileManager.hpp"

class Executor {
   public:
    std::string outputFilename;
    FileManager *fileManager;

    virtual void toString() = 0;
};

#endif  // EXECUTOR_HPP
