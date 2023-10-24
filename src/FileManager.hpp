#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <vector>

class FileManager {
   private:
    static FileManager *instance;
    static std::mutex mutex_;

   protected:
    std::string input_directory;
    std::string output_directory;
    std::string temp_directory;
    FileManager(std::string input, std::string output, std::string temp) {
        input_directory = input;
        output_directory = output;
        temp_directory = temp;
    }

   public:
    FileManager(FileManager &other) = delete;
    void operator=(const FileManager &) = delete;
    static FileManager *GetInstance(const std::string &input, const std::string &output, const std::string &temp);
    static FileManager *GetInstance();
    void setInputDirectory(const std::string &directory);
    void setOutputDirectory(const std::string &directory);
    void setTempDirectory(const std::string &directory);
    std::string getInputDirectory();
    std::string getOutputDirectory();
    std::string getTempDirectory();
    void printDirectories();
    std::vector<std::string> readFile(std::string filepath);
    void writeFile(std::string filepath, std::string filename, std::vector<std::string> file_lines);
    std::vector<std::string> getDirectoryFileList(std::string directory);
    bool checkDirectoryExists(std::string directory);
    void createDirectory(std::string directory);
};

#endif  // FILEMANAGER_H
