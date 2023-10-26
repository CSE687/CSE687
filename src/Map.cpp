//////////////////////////////////////////////////////////////
//  Map.cpp     -   implementation file for Map class       //
//  Language    -   C++                                     //
//  Author      -   Joe Cranston                            //
//////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <boost/filesystem.hpp>
#include "Map.hpp"

// Initialize FileManager instance and reserve buffer space
Map::Map(std::string inputFile, size_t bufSize, size_t lineCount){
    this->fileManager = FileManager::GetInstance();
    tempFilename = inputFile;
    bufferSize = bufSize;
    buffer.reserve(bufferSize);
    numLines = lineCount;
}

// Tokenize the line from the input file and write each tokenized word to disk
void Map::map(std::string value, size_t currLine){
    std::vector<std::string> words = tokenize(value);
    for (std::string word : words){
        exportData(word, currLine);
    }
}

// Create a vector of strings, with each element being a new word
std::vector<std::string> Map::tokenize(std::string line){

    // Treat doubles dashes as spaces
    boost::algorithm::replace_all(line, "--", " ");
    std::vector<std::string> words;
    std::string thisWord;

    // Remove any invalid ASCII values
    for (int strInd = 0; strInd < line.length(); strInd++){
        if ((line[strInd] < 0) || (line[strInd] > 127)){
            line[strInd] = ' ';
        }
    }

    // For each character in the line
    for (char c : line){
        // If it's a valid ASCII letter, lowercase it and add it to a temp word
        if (isalpha(c)){
            thisWord += tolower(c);
        }
        // If punctuation, ignore
        else if (ispunct(c)){
            continue;
        }
        // Append temp word containing only lowercase letters to vector of words
        else{
            words.push_back(thisWord);
            thisWord.clear();
        }
    }

    // Append word to vector a final time if not empty
    if (thisWord != ""){
        words.push_back(thisWord);
    }

    // Remove any empty lines of data from the vector
    words.erase(std::remove_if(words.begin(), words.end(), [](const std::string& word) {
        return word.empty();
    }), words.end());

    return words;
}

// Buffer output in memory and write to disk when buffer is full or last line of input file is reached
void Map::exportData(std::string word, size_t currLine){
    buffer += "(" + word + ", 1)\n";
    if ((buffer.size() >= bufferSize) || (currLine == numLines - 1)){
        std::string tmpFile = boost::filesystem::path(tempFilename).stem().string() + "Output.txt";
        this->fileManager->appendToFile(this->fileManager->getTempDirectory(), tmpFile, buffer);
        buffer.clear();
    }
}