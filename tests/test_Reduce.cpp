#define BOOST_TEST_MODULE TestReduce
#include <boost/test/included/unit_test.hpp>
#include <iterator>
#include <string>
#include <vector>

#include "../src/Reduce.hpp"

BOOST_AUTO_TEST_CASE(test_reduce__sum_iterator) {
    std::string output_dir = "asdf/asdf";
    Reduce reduce = Reduce(output_dir);

    BOOST_TEST(2 == reduce._sum_iterator(std::vector<int>{1, 1}));
    BOOST_TEST(6 == reduce._sum_iterator(std::vector<int>{1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(test_reduce__reduce) {
    std::string output_dir = "asdf/asdf";
    Reduce reduce = Reduce(output_dir);

    reduce.reduce("hello", std::vector<int>{1, 1});
}
