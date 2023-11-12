#define BOOST_TEST_MODULE TestFileManager
#include <time.h>

#include <boost/filesystem.hpp>
#include <boost/test/included/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../src/FileManager.hpp"

FileManager* filemanager = FileManager::GetInstance("tests/workdir/input", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(test_filemanager__get_input_directory) {
    std::string expected = "tests/workdir/input";
    BOOST_TEST(expected == filemanager->getInputDirectory());
}

BOOST_AUTO_TEST_CASE(test_filemanager__get_output_directory) {
    std::string expected = "tests/workdir/output";
    BOOST_TEST(expected == filemanager->getOutputDirectory());
}

BOOST_AUTO_TEST_CASE(test_filemanager__get_temporary_directory) {
    std::string expected = "tests/workdir/temp";
    BOOST_TEST(expected == filemanager->getTempDirectory());
}

BOOST_AUTO_TEST_CASE(test_filemanager__create_directory) {
    filemanager->createDirectory("tests/workdir/unit");
    BOOST_TEST(true == filemanager->checkDirectoryExists("tests/workdir/unit"));
}

BOOST_AUTO_TEST_CASE(test_filemanager__delete_all) {
    filemanager->remove("tests/workdir/unit");
    BOOST_TEST(false == filemanager->checkDirectoryExists("tests/workdir/unit"));
}

BOOST_AUTO_TEST_CASE(test_filemanager__check_directory_exists) {
    BOOST_TEST(true == filemanager->checkDirectoryExists(filemanager->getInputDirectory()));
    BOOST_TEST(false == filemanager->checkDirectoryExists("tests/workdir/unit"));
}

BOOST_AUTO_TEST_CASE(test_filemanager__get_directory_file_list) {
    std::vector<std::string> file_list = filemanager->getDirectoryFileList(filemanager->getInputDirectory());

    BOOST_TEST(0 < file_list.size());
}

BOOST_AUTO_TEST_CASE(test_filemanager__check_write_and_read) {
    std::vector<std::string> file_lines = {"First", "Second", "Third"};
    filemanager->writeFile(filemanager->getOutputDirectory(), "unit.txt", file_lines);

    std::vector<std::string> read_lines = filemanager->readFile(filemanager->getOutputDirectory(), "unit.txt");

    BOOST_TEST(file_lines == read_lines);
}

BOOST_AUTO_TEST_CASE(test_filemanager__check_delete_file) {
    std::vector<std::string> file_lines = {"First", "Second", "Third"};
    filemanager->writeFile(filemanager->getOutputDirectory(), "unit.txt", file_lines);
    filemanager->deleteFile(filemanager->getOutputDirectory(), "unit.txt");

    std::vector<std::string> read_lines = filemanager->readFile(filemanager->getOutputDirectory(), "unit.txt");

    BOOST_TEST(file_lines != read_lines);
}

BOOST_AUTO_TEST_CASE(test_filemanager__check_append_to_file) {
    std::srand(time(NULL));
    int random_num = std::rand();
    std::string file_line = "Append Line " + std::to_string(random_num);
    filemanager->appendToFile(filemanager->getTempDirectory(), "unit.txt", file_line);
    std::vector<std::string> read_lines = filemanager->readFile(filemanager->getTempDirectory(), "unit.txt");

    std::string last_line = read_lines[read_lines.size() - 1];
    BOOST_TEST(file_line == last_line);
}
