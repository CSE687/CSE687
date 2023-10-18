/*
  The Reduce class will have a method `reduce()` that will take a string with one key
  and an iterator of integers.

    The `reduce` function should sum all the values in the iterator and then call an export function (different from the Map clas).
    The `export` function will take two parameters, the key and resulting reduced value, and write the result out to the output directory.

    Upon success an additional empty file SUCCESS will be written out to the output directory.

    The reduce method itself should not directly deal with any File IO.
*/

#include <string>
#include <vector>
#include <algorithm>

class Reduce
{
public:
    Reduce(const std::string &output_dir) : _output_dir(output_dir) {}

    // Sum vector and write to disk
    void reduce(const std::string &key, const std::vector<int> &values);

    // Returns the sum of the supplied vector
    int _sum_iterator(const std::vector<int> &values);

private:
    void export_result(const std::string &key, int value);

    std::string _output_dir;
};
