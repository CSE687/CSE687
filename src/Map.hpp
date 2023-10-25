#ifndef MAP_H
#define MAP_H

//////////////////////////////////////////////////////////////
//  Map.h       -   header file for Map class               //
//  Language    -   C++                                     //
//  Author      -   Joe Cranston                            //
//////////////////////////////////////////////////////////////

/*
The map class contains a method map() that takes a string containing a line from an input file.
The map() method calls the tokenize() method which removes all punctuation, capitalization, and
whitespace and returns a vector of strings, with each element being a new word. The map() method
then calls the exportData() method which buffers output in memory and writes to disk when the
buffer is full.
*/

#include <string>
#include <vector>
#include <fstream>
#include "FileManager.hpp"

class Map{
    public:

    // class constructor
    Map(std::string, std::string, std::string, size_t, size_t);
    
    // tokenize string and write to disk
    void map(std::string, std::string, size_t);

    // returns a vector of strings, with each element being a new word
    std::vector<std::string> tokenize(std::string);

    // buffers memory and writes to disk when buffer is full
    void exportData(std::string, size_t);

    // class destructor
    ~Map();

    private:
    FileManager* fileManager;
    std::string tempFilename;
    size_t bufferSize;
    std::vector<std::string> buffer;
    size_t numLines;
    
    
    
};
#endif