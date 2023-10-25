#define BOOST_TEST_MODULE TestMap
#include <boost/test/included/unit_test.hpp>
#include "../src/Map.hpp"

FileManager* fileManager = FileManager::GetInstance("/workspaces/CSE687_Project/tests/workdir/input", "/workspaces/CSE687_Project/tests/workdir/output", "/workspaces/CSE687_Project/tests/workdir/temp");

BOOST_AUTO_TEST_CASE(TestMap_tokenize){
    size_t bufSize = 1024;
    size_t numLines = 1;
    Map myMap = Map(bufSize, numLines);

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
