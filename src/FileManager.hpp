
#include <boost/filesystem.hpp>
#include <vector>
#include <iostream>

class FileManager {
public:
    std::string input_directory;
    std::string output_directory;
    std::string temp_directory;
    FileManager(std::string input, std::string output, std::string temp) {
        input_directory = input;
        output_directory = output;
        temp_directory = temp;
    }
    std::string getInputDirectory();
    std::string getOutputDirectory();
    std::string getTempDirectory();
    void printDirectories();
    void readFile(std::string directory);
    void writeFile(std::string directory);
    std::vector<std::string> getDirectoryFileList(std::string directory);
    bool checkDirectoryExists(std::string directory);
};