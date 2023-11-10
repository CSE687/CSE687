#include "Reduce.hpp"

#include <boost/format.hpp>
#include <iostream>
#include <numeric>

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

void Reduce::execute(const std::string &key, const std::vector<int> &values) {
    int sum = Reduce::_sum_iterator(values);
    exportResult(key, sum);
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

void Reduce::toString() {
    std::cout << "Output filename: " << this->outputFilename << std::endl;
}
