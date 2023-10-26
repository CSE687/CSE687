#ifndef MAP_H
#define MAP_H

//////////////////////////////////////////////////////////////
//  Map.hpp     -   header file for Map class               //
//  Language    -   C++                                     //
//  Author      -   Joe Cranston                            //
//////////////////////////////////////////////////////////////

/*
The map class contains a method map() that takes a string containing a line from an input file, calls the 
tokenize() method to remove all punctuation, capitalization, and whitespace, and finally calls the exportData()
 method to buffer output in memory and write to disk when the buffer is full.
*/

#include <string>
#include <vector>
#include <fstream>
#include "FileManager.hpp"

class Map{

    public:

    // class constructor
    Map(std::string, size_t, size_t);
    
    // tokenize string and write to disk
    void map(std::string, size_t);

    // returns a vector of strings, with each element being a new word
    static std::vector<std::string> tokenize(std::string);

    // buffers memory and writes to disk when buffer is full
    void exportData(std::string, size_t);

    private:

    FileManager* fileManager;
    std::string tempFilename;
    size_t bufferSize;
    std::string buffer;
    size_t numLines;
};
#endif