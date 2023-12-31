/////////////////////////////////////////////////////////////////////
//  FileManager.hpp    -   header file for FileManager class       //
//  Language           -   C++                                     //
//  Author             -   Chandler Johns                          //
/////////////////////////////////////////////////////////////////////

/*
The FileManager class contains all the functionality to read and write files.
This class is created as a singleton so that every class can use the same instance of FileManager.
*/
#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <vector>

class FileManager {
   private:
    static FileManager *instance;  // pointer to class instance
    static std::mutex mutex_;      // mutex to keep class between multiple threads
   protected:
    std::string input_directory;
    std::string output_directory;
    std::string temp_directory;
    // Constructor based on arguments to create file manager class
    FileManager(std::string input, std::string output, std::string temp) {
        input_directory = input;
        output_directory = output;
        temp_directory = temp;
    }

   public:
    FileManager(FileManager &other) = delete;                                                                       // remove the ability to create a new instance
    void operator=(const FileManager &) = delete;                                                                   // can't assign class instance to new object
    static FileManager *GetInstance(const std::string &input, const std::string &output, const std::string &temp);  // constructor based on argument input
    static FileManager *GetInstance();                                                                              // constructor to get an already created file manager instance
    void printDirectories();                                                                                        // prints the input, output, and temporary directories for the class
    bool checkDirectoryExists(std::string directory);                                                               // checks that a directory exists
    void createDirectory(std::string directory);                                                                    // creates a directory
    std::vector<std::string> getDirectoryFileList(std::string directory);                                           // returns a list of files from a specified directory
    std::vector<std::string> readFile(std::string full_filepath);                                                   // reads a file given the full filepath
    std::vector<std::string> readFile(std::string filepath, std::string filename);                                  // reads a file given the filepath and filename separately
    void writeFile(std::string filepath, std::string filename, std::vector<std::string> file_lines);                // writes a file given filepath, filename, and lines in a vector
    void writeFile(std::string filepath, std::string filename, std::string all_file_data);                          // writes data to a file
    void appendToFile(std::string filepath, std::string filename, std::string file_line);                           // writes a file given filepath, filename, and a string to append to end of file
    void remove(std::string full_filepath);                                                                         // deletes a directory or file given full path
    void deleteFile(std::string filepath, std::string filename);                                                    // deletes a file
    std::string getFileStem(std::string filepath);                                                                  // gets the filename from a filepath
    std::string getInputDirectory();                                                                                // gets the input directory as specified by program and saved to class instance
    std::string getOutputDirectory();                                                                               // gets the output directory as specified by program and saved to class instance
    std::string getTempDirectory();                                                                                 // gets the temporary directory as specified by program and saved to class instance
};

#endif  // FILEMANAGER_H
