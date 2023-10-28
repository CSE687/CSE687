#ifndef WORDTOKEN_H
#define WORDTOKEN_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class WordToken {
   public:
    // constructors
    WordToken(const string w = "", vector<int> c = {}) {
        word = w;
        counts = c;
        len = c.size();
    }

    friend ostream& operator<<(ostream& os, WordToken const& wt) {
        os << "(\"" << wt.word << "\", {";
        for (auto i = wt.counts.begin(); i != wt.counts.end(); i++) {
            if (i != wt.counts.begin())
                os << ",";
            os << *i;
        }
        os << "})";
        return os;
    }

    // getters
    void print(ostream& osstr = cout) const;
    string getWord() const;
    int getCountsLen() const;
    vector<int>* getCounts();

    // Operator overloads sort/comparison functions
    bool operator<(const WordToken&);
    bool operator>(const WordToken&);
    bool operator<=(const WordToken&);
    bool operator>=(const WordToken&);
    bool operator==(const WordToken&);

    // Operator overloads addition functions
    WordToken& operator++(int);
    WordToken& operator+=(int);

   private:
    string word;
    vector<int> counts;
    int len;
};
#endif
