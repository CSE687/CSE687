#define BOOST_TEST_MODULE TestMap
#include <boost/test/included/unit_test.hpp>
#include "../src/Map.hpp"

FileManager* fileManager = FileManager::GetInstance("workdir/input", "workdir/output", "workdir/temp");

BOOST_AUTO_TEST_CASE(TestMap_tokenize){
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

    BOOST_TEST(token == Map::tokenize("This is a test -- is the code wor-king okay?"));
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_CASE(TestMap_map){

    size_t bufSize = 1024;
    size_t lineCount = 2;
    size_t currLine = 1;
    std::string inputFile = "tests/workdir/input/testMap.txt";

    Map myMap = Map(inputFile, bufSize, lineCount);

    std::vector<std::string> fileLines;
    std::string testLine = "Hello Map Class!";
    fileLines.push_back(testLine);
    fileManager->writeFile(fileManager->getInputDirectory(), "testMap.txt", fileLines);

    myMap.map(testLine, currLine);

    std::vector<std::string> read_lines = fileManager->readFile(fileManager->getTempDirectory(), "testMapOutput.txt");

    BOOST_TEST(read_lines[0] == "(hello, 1)");
    BOOST_TEST(read_lines[1] == "(map, 1)");
    BOOST_TEST(read_lines[2] == "(class, 1)");
    BOOST_TEST(true);
}
