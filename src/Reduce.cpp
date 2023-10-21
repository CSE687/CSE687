#include "Reduce.h"

#include <boost/format.hpp>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

int Reduce::_sum_iterator(const std::vector<int> &values) {
    int sum = std::accumulate(values.begin(), values.end(), 0);
    return sum;
}

void Reduce::reduce(const std::string &key, const std::vector<int> &values) {
    int sum = Reduce::_sum_iterator(values);
    export_result(key, sum);
}

void Reduce::export_result(const std::string &key, int value) {
    // Write result to output directory
    // Should use File Management Class
    std::cout << "Writing (" + key + ", " << value << ") to " + this->_output_dir << std::endl;

    // Write SUCCESS file to output directory
    // Should use File Management Class
    std::cout << "Writing SUCCESS file to " + this->_output_dir << std::endl;
}
