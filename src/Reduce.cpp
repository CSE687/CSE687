#include "Reduce.hpp"

#include <boost/format.hpp>
#include <iostream>
#include <numeric>

/**
 * @brief Reduces the count that a word appears in a file to a single integer sum and writes it to output file.
 *
 * @param values The vector of integers to be reduced.
 */
extern "C" int reduce(const std::vector<int> &values) {
    int sum = std::accumulate(values.begin(), values.end(), 0);
    return sum;
}
