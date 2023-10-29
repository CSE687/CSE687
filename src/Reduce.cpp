#include "Reduce.hpp"

#include <boost/format.hpp>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "FileManager.hpp"

Reduce::Reduce(std::string outputFilename) {
    this->fileManager = FileManager::GetInstance();
    this->outputFilename = outputFilename;
}

int Reduce::_sum_iterator(const std::vector<int> &values) {
    int sum = std::accumulate(values.begin(), values.end(), 0);
    return sum;
}

void Reduce::reduce(const std::string &key, const std::vector<int> &values) {
    int sum = Reduce::_sum_iterator(values);
    export_result(key, sum);
}

void Reduce::export_result(const std::string &key, int value) {
    // Write result to output file using fileManager
    fileManager->appendToFile(
        this->fileManager->getOutputDirectory(),
        this->outputFilename,
        "(" + key + ", " + std::to_string(value) + ")\n");
}
