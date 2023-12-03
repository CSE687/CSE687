#define BOOST_TEST_MODULE TestThreadManager
#include <boost/test/included/unit_test.hpp>
#include "../src/ThreadManager.hpp"
#include "../src/FileManager.hpp"
#include "../src/Workflow.hpp"

FileManager* fileManager = FileManager::GetInstance("tests/workdir/input/testMapThreads", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(TestThreadManager_executeMapThreads){

    std::vector<std::string> input_files = fileManager->getDirectoryFileList(fileManager->getInputDirectory());

    // Initialize 
    ThreadManager threadMang(fileManager, &input_files);
    threadMang.executeMapThreads();

    std::vector<std::string> output_files = fileManager->getDirectoryFileList(fileManager->getTempDirectory());
    
    std::vector<std::string> output1 = fileManager->readFile(fileManager->getTempDirectory(), "test1.txt");
    std::vector<std::string> testOutput1;
    testOutput1.push_back("(thread, 1)");
    testOutput1.push_back("(one, 1)");

    std::vector<std::string> output2 = fileManager->readFile(fileManager->getTempDirectory(), "test2.txt");
    std::vector<std::string> testOutput2;
    testOutput2.push_back("(thread, 1)");
    testOutput2.push_back("(two, 1)");

    BOOST_TEST(output1 == testOutput1);
    BOOST_TEST(output2 == testOutput2);
}