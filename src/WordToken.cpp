#include "WordToken.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string WordToken::getWord() const {
    return word;
}

vector<int>* WordToken::getCounts() {
    return &counts;
}

int WordToken::getCountsLen() const {
    return this->counts.size();
}

WordToken& WordToken::operator++(int) {
    counts.push_back(1);
    len++;
    return *this;
}

WordToken& WordToken::operator+=(int n) {
    counts.push_back(n);
    len++;
    return *this;
}

bool WordToken::operator<(const WordToken& wt) {
    return this->len < wt.getCountsLen();
}

bool WordToken::operator>(const WordToken& wt) {
    return this->len > wt.getCountsLen();
}
bool WordToken::operator<=(const WordToken& wt) {
    return this->len <= wt.getCountsLen();
}
bool WordToken::operator>=(const WordToken& wt) {
    return this->len >= wt.getCountsLen();
}

bool WordToken::operator==(const WordToken& wt) {
    return this->word == wt.getWord();
}
