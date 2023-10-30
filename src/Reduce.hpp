/*
  The Reduce class will have a method `reduce()` that will take a string with one key
  and an iterator of integers.

    The `reduce` function should sum all the values in the iterator and then call an export function (different from the Map clas).
    The `export` function will take two parameters, the key and resulting reduced value, and write the result out to the output directory.

    Upon success an additional empty file SUCCESS will be written out to the output directory.

    The reduce method itself should not directly deal with any File IO.
*/

#include <algorithm>
#include <string>
#include <vector>

#include "FileManager.hpp"

#ifndef REDUCE_H
#define REDUCE_H

/**
 * @brief A class that reduces a vector of integers to a single integer sum and writes it to disk.
 */
class Reduce {
   public:
    Reduce(std::string outputFilename);
    FileManager *fileManager;

    /**
     * @brief Exports the result of the reduction to a file.
     *
     * @param key The key associated with the vector of integers.
     * @param value The reduced sum of the vector of integers.
     */
    void export_result(const std::string &key, int value);

    // stores name of output file
    std::string outputFilename;

    /**
     * @brief Reduces the supplied vector of integers to a single integer sum and writes it to output file.
     *
     * @param key The key associated with the vector of integers.
     * @param values The vector of integers to be reduced.
     */
    void reduce(const std::string &key, const std::vector<int> &values);

    /**
     * @brief Returns the sum of the supplied vector of integers.
     *
     * @param values The vector of integers to be summed.
     * @return The sum of the supplied vector of integers.
     */
    int _sum_iterator(const std::vector<int> &values);
};
#endif
