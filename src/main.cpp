#include <vector>
#include <iostream>
#include <fstream>
#include <string>

#include <boost/log/trivial.hpp>

#include "Reduce.h"
#include "Map.h"

// Input Dir
// Output Dir
// Temp Dir
int main(int argc, char *argv[])
{
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    std::vector<int> values = {1, 2, 3};
    std::string output_dir = "some/path/somewhere";
    Reduce reduce = Reduce(output_dir);
    reduce.reduce("Hello", values);

    // start of JC's main.cpp
    std::ifstream inputFile(argv[1], std::ios::in);

    size_t bufSize = 1024;
    Map myMap(argv[2], bufSize);

    if (inputFile.is_open()){
        std::string line;
        while(inputFile.good()){
            getline(inputFile, line);
            myMap.map(line);
        }
        inputFile.close();
    }

    else{
        std::cout << "Unable to open file.";
    }

    return 0;
}
