//////////////////////////////////////////////////////////////
//  Map.cpp     -   implementation file for Map class       //
//  Language    -   C++                                     //
//  Author      -   Joe Cranston                            //
//////////////////////////////////////////////////////////////

#include "Map.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>

extern "C" std::vector<std::string> map(std::string filename, std::string line) {
    std::vector<std::string> words = tokenize(line);
    return words;
}

// Create a vector of strings, with each element being a new word
std::vector<std::string> tokenize(std::string line) {
    // Treat doubles dashes as spaces
    boost::algorithm::replace_all(line, "--", " ");
    std::vector<std::string> words;
    std::string thisWord;

    // Remove any invalid ASCII values
    for (int strInd = 0; strInd < line.length(); strInd++) {
        if ((line[strInd] < 0) || (line[strInd] > 127)) {
            line[strInd] = ' ';
        }
    }

    // For each character in the line
    for (char c : line) {
        // If it's a valid ASCII letter, lowercase it and add it to a temp word
        if (isalpha(c)) {
            thisWord += tolower(c);
        }
        // If punctuation, ignore
        else if (ispunct(c)) {
            continue;
        }
        // Append temp word containing only lowercase letters to vector of words
        else {
            words.push_back(thisWord);
            thisWord.clear();
        }
    }

    // Append word to vector a final time if not empty
    if (thisWord != "") {
        words.push_back(thisWord);
    }

    // Remove any empty lines of data from the vector
    words.erase(std::remove_if(words.begin(), words.end(), [](const std::string& word) {
                    return word.empty();
                }),
                words.end());

    return words;
}
