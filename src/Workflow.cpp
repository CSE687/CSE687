#include "Workflow.hpp"

#include <string>

#include "FileManager.hpp"
#include "Map.hpp"
#include "Reduce.hpp"
// #include "Sort.hpp"

Workflow::Workflow(std::string inputDir, std::string tempDir, std::string outputDir) {
    this->inputDir = inputDir;
    this->tempDir = tempDir;
    this->outputDir = outputDir;

    // Initialize FileManager
    this->fileManager = FileManager::GetInstance(inputDir, outputDir, tempDir);
}

std::string Workflow::getInputDir() const {
    return inputDir;
}

void Workflow::setInputDir(std::string inputDir) {
    this->inputDir = inputDir;
    this->fileManager->setInputDirectory(this->inputDir);
}

std::string Workflow::getTempDir() const {
    return tempDir;
}

void Workflow::setTempDir(std::string tempDir) {
    this->tempDir = tempDir;
    this->fileManager->setTempDirectory(this->tempDir);
}

std::string Workflow::getOutputDir() const {
    return outputDir;
}

void Workflow::setOutputDir(std::string outputDir) {
    this->outputDir = outputDir;
    this->fileManager->setOutputDirectory(this->outputDir);
}

void Workflow::execute() {
    std::vector<std::string> input_files = this->fileManager->getDirectoryFileList(this->fileManager->getInputDirectory());

    // Execute Map class
    size_t bufSize = 1024; 
    for (int i = 0; i < input_files.size(); i++){
        std::vector<std::string> file_lines = this->fileManager->readFile(input_files[i]);
        size_t numLines = file_lines.size();
        Map myMap = Map(input_files[i], bufSize, numLines);
        for (size_t j = 0; j < numLines; j++){
            myMap.map(file_lines[j], j);
        }
    }

    // Execute Sorting class
    // Execute Reduce class
}
