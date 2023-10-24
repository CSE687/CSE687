#include "Workflow.hpp"

#include <string>

#include "FileManager.hpp"
// #include "Map.hpp"
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
    std::cout << "All Files in Input Directory:\n";
    for (std::string i : input_files) {
        std::cout << "\t" << i << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Reading first file in directory: " << input_files[0] << std::endl;
    std::vector<std::string> file_lines = this->fileManager->readFile(input_files[0]);
    for (std::string i : file_lines) {
        std::cout << i << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Copying file to write: " << input_files[0] << std::endl;
    this->fileManager->writeFile(this->fileManager->getOutputDirectory(), "file_copy.txt", file_lines);

    // Execute Map class
    // Execute Sorting class
    // Execute Reduce class
}
