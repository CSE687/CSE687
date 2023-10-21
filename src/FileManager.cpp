
#include "FileManager.hpp"

void FileManager::printDirectories() {
    std::cout << "Input Directory: " << input_directory << std::endl;
    std::cout << "Output Directory: " << output_directory << std::endl;
    std::cout << "Temp Directory: " << temp_directory << std::endl;
}

bool FileManager::checkDirectoryExists(std::string directory) {
    return boost::filesystem::is_directory(directory);
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

std::string FileManager::getInputDirectory() { return input_directory; }
std::string FileManager::getOutputDirectory() { return output_directory; }
std::string FileManager::getTempDirectory() { return temp_directory; }