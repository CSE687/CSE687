#define BOOST_TEST_MODULE TestReduce
#include <boost/test/included/unit_test.hpp>
#include <iterator>
#include <string>
#include <vector>

#include "../src/Reduce.hpp"

FileManager* fileManager = FileManager::GetInstance("tests/workdir/input", "tests/workdir/output", "tests/workdir/temp");

BOOST_AUTO_TEST_CASE(test_reduce__sum_iterator) {
    std::string output_filename = "reduceTest.txt";
    Reduce reduce = Reduce(output_filename);

    BOOST_TEST(2 == reduce._sum_iterator(std::vector<int>{1, 1}));
    BOOST_TEST(6 == reduce._sum_iterator(std::vector<int>{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(test_reduce__reduce) {
    std::string output_filename = "reduceTest.txt";
    Reduce reduce = Reduce(output_filename);

    reduce.execute("hello", std::vector<int>{1, 1});
}
