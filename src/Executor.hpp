#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "FileManager.hpp"

template <typename... Args>
class Executor {
   public:
    std::string outputFilename;
    FileManager *fileManager;
    virtual void execute(const Args &...args) = 0;
};

#endif  // EXECUTOR_HPP
