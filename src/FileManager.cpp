
#include "FileManager.hpp"

FileManager* FileManager::instance{nullptr};
std::mutex FileManager::mutex_;

FileManager *FileManager::GetInstance(const std::string& input, const std::string& output, const std::string& temp) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance == nullptr) {
        instance = new FileManager(input, output, temp);
    }
    return instance;
}

FileManager *FileManager::GetInstance() {
    if (instance == nullptr) {
        std::cerr << "Unable to instantiate File Manager, need to input arguments.\n"; 
    } else {
        return instance;
    }
}

void FileManager::printDirectories() {
    std::cout << "Input Directory: " << input_directory << std::endl;
    std::cout << "Output Directory: " << output_directory << std::endl;
    std::cout << "Temp Directory: " << temp_directory << std::endl;
}

bool FileManager::checkDirectoryExists(std::string directory) {
    return boost::filesystem::is_directory(directory);
}

void FileManager::createDirectory(std::string directory) {
    boost::filesystem::create_directory(directory);
}

std::vector<std::string> FileManager::getDirectoryFileList(std::string directory) {
    boost::filesystem::directory_iterator end_itr;
    std::vector<std::string> file_list;

    // cycle through the directory
    for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr) {
        // If it's not a directory, list it. If you want to list directories too, just remove this check.
        if (boost::filesystem::is_regular_file(itr->path())) {
            // assign current file name to current_file and echo it out to the console.
            std::string current_file = itr->path().string();
            file_list.insert(file_list.end(), current_file);
        }
    }
    return file_list;
}

std::vector<std::string> FileManager::readFile(std::string filepath) {
    std::vector<std::string> file_lines;
    std::ifstream read_file(filepath);
    std::string line;
    while (std::getline(read_file, line)) {
        file_lines.insert(file_lines.end(), line);
    }
    return file_lines;
}

void FileManager::writeFile(std::string filepath, std::string filename, std::vector<std::string> file_lines) {
    std::string allLines;
    if (!checkDirectoryExists(filepath)) {
        createDirectory(filepath);
    }
    std::string full_file_path = filepath + "/" + filename;
    std::ofstream write_file(full_file_path);

    if (write_file.is_open()) {
        for (std::string i: file_lines) {
            write_file << i << std::endl;
        }
    }
    else { std::cerr << "Unable to open file\n"; }
    write_file.close();
}

std::string FileManager::getInputDirectory() { return input_directory; }
std::string FileManager::getOutputDirectory() { return output_directory; }
std::string FileManager::getTempDirectory() { return temp_directory; }