//////////////////////////////////////////////////////////////
//  Map.h       -   header file for Map class               //
//  Language    -   C++                                     //
//  Author      -   Joe Cranston                            //
//////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "Map.h"

// Initialize member variables, reserve buffer space, and open the output file
Map::Map(char* outFile, size_t bufSize){
    bufferSize = bufSize;
    buffer.reserve(bufferSize);
    outFilename = outFile;
    outputFile.open(outFilename);
}

// Tokenize the line from the input file and write each tokenized word to disk
void Map::map(std::string value){
    std::vector<std::string> words = tokenize(value);
    for (std::string word : words){
        exportData(word);
    }
}

// Create a vector of strings, with each element being a new word
std::vector<std::string> Map::tokenize(std::string line){
    std::vector<std::string> words;

    // Treat doubles dashes as spaces
    boost::algorithm::replace_all(line, "--", " ");

    // Remove any invalid ASCII values
    for (int strInd = 0; strInd < line.length(); strInd++){
        if ((line[strInd] < 0) || (line[strInd] > 127)){
            line[strInd] = ' ';
        }
    }

    // Split each word in the input line whenever a space or tab is reached
    boost::split(words, line, boost::is_any_of(" \t"));

    // Remove any empty lines of data from the vector
    words.erase(std::remove_if(words.begin(), words.end(), [](const std::string& word) {
        return word.empty();
    }), words.end());

    // FOr each word, remove punctuation and capitalization
    for (std::string& word : words){
        std::string result = "";
        for (char c : word){
            if (!std::ispunct(c)) {
                result += c;
            }
        }
        word = result;
        boost::to_lower(word);
    }
    return words;
}

// Buffer output in memory and write to disk when buffer is full
void Map::exportData(std::string word){
    buffer += "(" + word + ", 1)\n";
    if (buffer.size() >= bufferSize){
        outputFile << buffer;
        buffer.clear();
    }
}

// Write to disk a final time, flush the buffer, and close the file
Map::~Map(){
    outputFile << buffer;
    buffer.clear();
    outputFile.close();
}
