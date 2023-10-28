#define BOOST_TEST_MODULE TestWordToken
#include <boost/test/included/unit_test.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "../src/WordToken.hpp"

using namespace std;

BOOST_AUTO_TEST_CASE(test_wordtoken__Counts) {
    vector<int> v = {1, 2, 3};
    WordToken wt = WordToken("a", v);
    // cout << "DEBUG counts: " << wt << endl;
    BOOST_TEST(wt.getCountsLen() == v.size());

    wt = WordToken("a", {});
    // cout << "DEBUG counts: " << wt << endl;
    BOOST_TEST(wt.getCountsLen() == 0);
}

BOOST_AUTO_TEST_CASE(test_wordtoken__Increments) {
    vector<int> v = {1, 2, 3};
    WordToken wt = WordToken("a", v);
    wt++;
    BOOST_TEST(wt.getCountsLen() == v.size() + 1);
    v.push_back(1);
    BOOST_TEST(*wt.getCounts() == v);

    wt += 4;
    v.push_back(4);
    BOOST_TEST(*wt.getCounts() == v);
    // cout << "DEBUG append4: " << wt << endl;
}
