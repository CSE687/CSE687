#include "Reduce.hpp"

#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <numeric>
#include <regex>

boost::mutex reduce_cout_mutex;

Reduce::Reduce(std::string outputFilename) {
    this->fileManager = FileManager::GetInstance();
    this->outputFilename = outputFilename;

    buffer.reserve(bufferSize);

    // creates a blank file and overwrites any pre-existing files with same name
    this->fileManager->writeFile(this->fileManager->getOutputDirectory(), this->outputFilename, "");
}

int Reduce::_sum_iterator(const std::vector<int> &values) {
    int sum = std::accumulate(values.begin(), values.end(), 0);
    return sum;
}

void Reduce::execute(const std::string &input_file) {
    Reduce::sort(input_file);

    for (auto &key : this->sorted_words) {
        int sum = Reduce::_sum_iterator(key.second);
        exportResult(key.first, sum);
    }
    Reduce::flushBuffer();

    // Write SUCCESS file to output directory
    this->fileManager->writeFile(fileManager->getOutputDirectory(), this->outputFilename + "-SUCCESS", "");
    // remove temp directory
    // fileManager->remove(input_file);
}

void Reduce::exportResult(const std::string &key, int value) {
    buffer += "(" + key + ", " + std::to_string(value) + ")\n";

    if ((buffer.size() >= bufferSize)) {
        // Write result to output file using fileManager
        this->flushBuffer();
    }
}

void Reduce::flushBuffer() {
    // Write result to output file using fileManager
    fileManager->appendToFile(
        this->fileManager->getOutputDirectory(),
        this->outputFilename,
        buffer);

    buffer.clear();
}

std::string Reduce::toString() {
    return this->outputFilename;
}

void Reduce::sort(const std::string &input_file) {
    // Sort & Reduce all of the files output by Mapper
    std::vector<std::string> file_lines = this->fileManager->readFile(input_file);

    for (std::string j : file_lines) {
        if (j.empty()) {
            continue;
        }
        std::regex key_rgx("^\\(([a-z]+),");
        std::regex value_rgx(", (\\d+)\\)");
        std::smatch key, value_match;
        regex_search(j, key, key_rgx);
        regex_search(j, value_match, value_rgx);
        if (key.size() < 2 || value_match.size() < 2) {
            std::cout << "SKIP: No matches found in " << j << std::endl;
            continue;
        }
        int value = stoi(value_match.str(1));
        std::map<std::string, std::vector<int>>::iterator find_iter = this->sorted_words.find(key.str(1));
        if (find_iter == this->sorted_words.end()) {
            this->sorted_words.insert({key.str(1), {value}});
        } else {
            std::vector<int> value_vector = find_iter->second;
            value_vector.insert(value_vector.end(), value);
            find_iter->second = value_vector;
        }
    }
}
