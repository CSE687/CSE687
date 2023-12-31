#define BOOST_TEST_MODULE TestMap
#include <boost/test/included/unit_test.hpp>
#include "../src/Map.hpp"

FileManager* fileManager = FileManager::GetInstance("tests/workdir/input", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(TestMap_tokenize){

    Map myMap = Map();
    std::vector<std::string> token;
    token.push_back("this");
    token.push_back("is");
    token.push_back("a");
    token.push_back("test");
    token.push_back("is");
    token.push_back("the");
    token.push_back("code");
    token.push_back("working");
    token.push_back("okay");

    BOOST_TEST(token == myMap.tokenize("This is a test -- is the code wor-king okay?"));
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(TestMap_map){
    std::string inputFile = "testMap.txt";

    Map myMap = Map();

    std::vector<std::string> fileLines;
    std::string testLine = "Hello Map Class!";
    fileLines.push_back(testLine);
    fileManager->writeFile(fileManager->getInputDirectory(), inputFile, fileLines);


    std::vector<std::string> readLinesFromInput = fileManager->readFile(fileManager->getInputDirectory(), inputFile);

    std::string lineToTest = readLinesFromInput[0];

    int wordCount = 0;
    wordCount = myMap.map(inputFile, lineToTest);
    myMap.flushBuffer();

    std::vector<std::string> read_lines_from_output = fileManager->readFile(fileManager->getTempDirectory(), inputFile);

    BOOST_TEST(read_lines_from_output[0] == "(hello, 1)");
    BOOST_TEST(read_lines_from_output[1] == "(map, 1)");
    BOOST_TEST(read_lines_from_output[2] == "(class, 1)");
    BOOST_TEST(wordCount == 3);
    BOOST_TEST(true);
}