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

#include "Executor.hpp"
#include "FileManager.hpp"

class Map : public Executor{

    public:

    // class constructor
    Map();

    // tokenize string and write to disk
    int map(std::string, std::string);

    // returns a vector of strings, with each element being a new word
    std::vector<std::string> tokenize(std::string);

    // buffers memory and writes to disk when buffer is full
    void exportData(std::string);

    // Flush buffer to file
    void flushBuffer();

    // Overwrites pure virtual method from base class to return string saying mapper has finished
    std::string toString();
};
#endif
