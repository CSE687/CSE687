/*
  The Reduce class will have a method `execute()` that will take a string with one key
  and an iterator of integers.

    The `execute` function should sum all the values in the iterator and then call an export function (different from the Map clas).
    The `export` function will take two parameters, the key and resulting reduced value, and write the result out to the output directory.

    Upon success an additional empty file SUCCESS will be written out to the output directory.

    The execute method itself should not directly deal with any File IO.
*/

#include <algorithm>
#include <string>
#include <vector>

#ifndef REDUCE_H
#define REDUCE_H

/**
 * @brief Reduces the count that a word appears in a file to a single integer sum and writes it to output file.
 *
 * @param values The vector of integers to be reduced.
 */
extern "C" int reduce(const std::vector<int> &values);
#endif
