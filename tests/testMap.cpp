#define BOOST_TEST_MODULE mytests
#include <boost/test/included/unit_test.hpp>
#include "../src/Map.hpp"

BOOST_AUTO_TEST_CASE(myTestCase){
    size_t bufSize = 1024;
    char argv[] = {'O', 'u', 't', 'p', 'u', 't', '.', 't', 'x', 't', 0};
    char* outputFile = argv;
    Map myMap(outputFile, bufSize);

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