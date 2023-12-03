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
    string test_file = "tests/workdir/input/test.txt";
    Workflow workflow = Workflow(fileManager);

    // execute Map on the input file
    workflow.executeMap(test_file, 1);

    // check that the temp file was created
    ifstream tf(fileManager->getTempDirectory() + "/" + fileManager->getFileStem(test_file) + ".txt");
    BOOST_TEST(tf.good());
    tf.close();

    // execute Reduce on the temp file
    workflow.executeReduce(fileManager->getTempDirectory() + "/" + fileManager->getFileStem(test_file) + ".txt", 1);

    // check that the output file was created
    ifstream of(fileManager->getOutputDirectory() + "/" + fileManager->getFileStem(test_file) + ".txt");
    BOOST_TEST(of.good());

    // check that the output file contains the expected data
    string first_line;
    string expected = "(a, 2)";
    getline(of, first_line);
    BOOST_TEST(expected.compare(first_line) == 0);
    of.close();

    // check that the SUCCESS file was created
    ifstream sf(fileManager->getOutputDirectory() + "/" + fileManager->getFileStem(test_file) + ".txt-SUCCESS");
    BOOST_TEST(sf.good());
    sf.close();
}
