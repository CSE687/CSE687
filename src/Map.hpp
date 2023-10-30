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

#include "FileManager.hpp"

class Map{

    public:

    // class constructor
    Map();

    // tokenize string and write to disk
    int map(std::string, std::string, int, int);

    // returns a vector of strings, with each element being a new word
    static std::vector<std::string> tokenize(std::string);

    // buffers memory and writes to disk when buffer is full
    void exportData(std::string, int, int);

    private:
    // pointer to instance of fileManager class
    FileManager* fileManager;

    // stores name of input file
    std::string inputFilename;

    // constant buffer size for each instance of Map
    const int bufferSize = 1024;

    // Buffer to store data before being written to disk
    std::string buffer;

    // Current line number of file that is being mapped
    int lineNum;
};
#endif
