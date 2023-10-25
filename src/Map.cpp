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
#include "Map.hpp"

// Initialize member variables, reserve buffer space, and open the output file
Map::Map(std::string inputDir, std::string outputDir, std::string tempDir, size_t bufSize, size_t lineCount){
    // Initialize FileManager
    this->fileManager = FileManager::GetInstance(inputDir, outputDir, tempDir);

    bufferSize = bufSize;
    buffer.reserve(bufferSize);
    numLines = lineCount;
}

// Tokenize the line from the input file and write each tokenized word to disk
void Map::map(std::string inputFile, std::string value, size_t currLine){
    tempFilename = inputFile;
    std::vector<std::string> words = tokenize(value);
    for (std::string word : words){
        exportData(word, currLine);
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
void Map::exportData(std::string word, size_t currLine){
    buffer.push_back("(" + word + ", 1)");
    if ((buffer.size() >= bufferSize) || (currLine == numLines - 1)){
        std::string tempFile = tempFilename.substr(14, tempFilename.size() - 18) + "Output.txt";
        this->fileManager->writeFile(this->fileManager->getTempDirectory(), tempFile, buffer);
        buffer.clear();
    }
}

// Destructor
Map::~Map(){}
