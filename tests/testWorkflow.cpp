#define BOOST_TEST_MODULE TestWorkflow
#include <boost/test/included/unit_test.hpp>
#include <string>

#include "../src/Workflow.hpp"

FileManager* fileManager = FileManager::GetInstance("tests/workdir/input", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(test_workflow__initializes) {
    Workflow workflow = Workflow(fileManager);

    BOOST_TEST(1 == 1);
}

BOOST_AUTO_TEST_CASE(test_workflow__executes) {
    Workflow workflow = Workflow(fileManager);
    workflow.execute();

    BOOST_TEST(1 == 1);
}
