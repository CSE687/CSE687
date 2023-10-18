#include <vector>

#include <boost/log/trivial.hpp>

#include "Reduce.h"

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

    return 0;
}
