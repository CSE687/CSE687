#define BOOST_TEST_MODULE TestFileManager
#include <boost/test/included/unit_test.hpp>
#include "../src/FileManager.hpp"

#include <string>
#include <boost/filesystem.hpp>
#include <vector>
#include <iostream>
#include <fstream>

BOOST_AUTO_TEST_CASE(test_filemanager__get_input_directory)
{
    FileManager* filemanager = FileManager::GetInstance("input", "output", "temporary");
    std::string expected = "input";
    BOOST_TEST(expected == filemanager->getInputDirectory());
    BOOST_TEST(true);
}

// BOOST_AUTO_TEST_CASE(test_filemanager__get_output_directory)
// {
//     FileManager* filemanager = FileManager::GetInstance("input", "output", "temporary");
//     std::string expected = "output";
//     BOOST_TEST(expected == filemanager->getOutputDirectory());
//     BOOST_TEST(true);
// }

// BOOST_AUTO_TEST_CASE(test_filemanager__get_temporary_directory)
// {
//     FileManager* filemanager = FileManager::GetInstance("input", "output", "temporary");
//     std::string expected = "temporary";
//     BOOST_TEST(expected == filemanager->getTempDirectory());
//     BOOST_TEST(true);
// }

// BOOST_AUTO_TEST_CASE(test_filemanager__print_directories)
// {
//     FileManager* filemanager = FileManager::GetInstance("input", "output", "temporary");
//     std::string expected_output = "Input Directory: input\nOutput Directory: output\nTemp Directory: temporary\n";

//     BOOST_TEST(true filemanager->printDirectories());
// }