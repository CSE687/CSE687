#define BOOST_TEST_MODULE TestWorkflow
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <string>

#include "../src/Workflow.hpp"

FileManager* fileManager = FileManager::GetInstance("tests/workdir/input", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(test_workflow__initializes) {
    Workflow workflow = Workflow(fileManager);
}

BOOST_AUTO_TEST_CASE(test_workflow__executes) {
    string test_file = "/test.txt";
    Workflow workflow = Workflow(fileManager);
    workflow.execute();
    ifstream f(fileManager->getOutputDirectory() + test_file + "-SUCCESS");
    BOOST_TEST(f.good());
    f.close();

    ifstream of(fileManager->getOutputDirectory() + test_file + "");
    string first_line;
    string expected = "(a, 2)";
    getline(of, first_line);
    BOOST_TEST(expected.compare(first_line) == 0);
}
