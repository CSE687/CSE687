
#include "FileManager.hpp"

FileManager* FileManager::instance{nullptr}; // pointer to class instance, initialized to a nullptr
std::mutex FileManager::mutex_; // mutex to keep class between multiple threads

// constructor based on argument input, creates the file manager class instance
// if the instance is already instantiated, it returns the instance
FileManager *FileManager::GetInstance(const std::string& input, const std::string& output, const std::string& temp) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance == nullptr) {
        instance = new FileManager(input, output, temp);
    }
    return instance;
}

// constructor to get an already created file manager instance that already exists, no constructor parameters necessary
FileManager *FileManager::GetInstance() {
    if (instance == nullptr) {
        std::cerr << "Unable to instantiate File Manager, need to input arguments.\n"; 
    }
    return instance;
}

// prints the input, output, and temporary directories for the class
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

// returns a list of files from a specified directory
std::vector<std::string> FileManager::getDirectoryFileList(std::string directory) {
    boost::filesystem::directory_iterator end_itr;  // ends the directory iterator
    std::vector<std::string> file_list;

    // cycle through the directory
    for (boost::filesystem::directory_iterator itr(directory); itr != end_itr; ++itr) {
        // If it's not a directory, list it. If you want to list directories too, just remove this check.
        if (boost::filesystem::is_regular_file(itr->path())) {
            // assign current file name to current_file and add it to our vector list.
            std::string current_file = itr->path().string();
            file_list.insert(file_list.end(), current_file);
        }
    }
    return file_list;
}

std::vector<std::string> FileManager::readFile(std::string full_filepath) {
    std::vector<std::string> file_lines;
    std::ifstream read_file(full_filepath);
    std::string line;
    // get each line from the file and insert it to our vector list as a string
    while (std::getline(read_file, line)) {
        file_lines.insert(file_lines.end(), line);
    }
    return file_lines;
}

std::vector<std::string> FileManager::readFile(std::string filepath, std::string filename) {
    std::string full_filepath = filepath + "/" + filename;
    return readFile(full_filepath);
}

void FileManager::writeFile(std::string filepath, std::string filename, std::vector<std::string> file_lines) {
    std::string allLines;
    // if the directory doesn't exist, we need to create the directory
    if (!checkDirectoryExists(filepath)) {
        createDirectory(filepath);
    }
    std::string full_file_path = filepath + "/" + filename;
    std::ofstream write_file(full_file_path); // create the output file object

    // open the file
    if (write_file.is_open()) {
        // write each line to the file
        for (std::string i: file_lines) {
            write_file << i << std::endl;
        }
    }
    // if the file can't be opened, return a cerr
    else { std::cerr << "Unable to open file\n"; }
    write_file.close(); // close the file
}

void FileManager::deleteFile(std::string filepath, std::string filename) {
    std::string full_file_path = filepath + "/" + filename;
    boost::filesystem::remove(full_file_path);
}

void FileManager::deleteAll(std::string full_filepath) {
    boost::filesystem::remove_all(full_filepath);
}

std::string FileManager::getInputDirectory() { return input_directory; }
std::string FileManager::getOutputDirectory() { return output_directory; }
std::string FileManager::getTempDirectory() { return temp_directory; }