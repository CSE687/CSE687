// #include <iostream>
// using std::cout;
// #include <boost/log/trivial.hpp>

// int main()
// {
//     // cout << "Hello World!";
//     BOOST_LOG_TRIVIAL(info) << "Hello, Boost.Log!";
//     return 0;
// }

// #include <iostream>
// #include <iterator>
// #include <algorithm>

// #include <boost/lambda/lambda.hpp>
// #include <boost/log/core.hpp>
// #include <boost/log/trivial.hpp>
// #include <boost/log/utility/setup/file.hpp>
// #include <boost/log/expressions.hpp>

// namespace logging = boost::log;

// void init_logging(const char *logger_filename)
// {
//     logging::add_file_log(logger_filename);

//     logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
// }

// int main(int, char *[])
// {
//     init_logging("titan_logger.log");
//     BOOST_LOG_TRIVIAL(trace) << "This is a trace severity message";
//     BOOST_LOG_TRIVIAL(debug) << "This is a debug severity message";
//     BOOST_LOG_TRIVIAL(info) << "This is an informational severity message";
//     BOOST_LOG_TRIVIAL(warning) << "This is a warning severity message";
//     BOOST_LOG_TRIVIAL(error) << "This is an error severity message";
//     BOOST_LOG_TRIVIAL(fatal) << "and this is a fatal severity message";
//     std::cin.get();
//     return 0;
// }

// int main()
// {
//     using namespace boost::lambda;
//     typedef std::istream_iterator<int> in;

//     std::for_each(
//         in(std::cin), in(), std::cout << (_1 * 3) << " ");
// }

#include <boost/log/trivial.hpp>

int mult(int a, int b)
{
    return a * b;
}

int main(int, char *[])
{
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    return 0;
}
