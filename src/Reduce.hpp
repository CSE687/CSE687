/*
  The Reduce class will have a method `execute()` that will take a string with one key
  and an iterator of integers.

    The `execute` function should sum all the values in the iterator and then call an export function (different from the Map clas).
    The `export` function will take two parameters, the key and resulting reduced value, and write the result out to the output directory.

    Upon success an additional empty file SUCCESS will be written out to the output directory.

    The execute method itself should not directly deal with any File IO.
*/
#ifndef REDUCE_H
#define REDUCE_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Executor.hpp"
#include "FileManager.hpp"

/**
 * @brief A class that reduces a vector of integers to a single integer sum and writes it to disk.
 */
class Reduce : public Executor {
   private:
    std::map<std::string, std::vector<int>> sorted_words;

   public:
    Reduce(std::string outputFilename);

    /**
     * @brief Exports the result of the reduction to a file.
     *
     * @param key The key associated with the vector of integers.
     * @param value The reduced sum of the vector of integers.
     */
    void exportResult(const std::string &key, int value);

    /**
     * @brief Flushes the buffer to the output file.
     */
    void flushBuffer();

    /**
     * @brief Reduces the supplied vector of integers to a single integer sum and writes it to output file.
     *
     * @param key The key associated with the vector of integers.
     * @param values The vector of integers to be reduced.
     */
    void execute(const std::string &input_file);

    /**
     * @brief Returns the sum of the supplied vector of integers.
     *
     * @param values The vector of integers to be summed.
     * @return The sum of the supplied vector of integers.
     */
    int _sum_iterator(const std::vector<int> &values);

    /**
     * @brief Prints the output filename.
     */
    std::string toString();

    /**
     * @brief Sorts the words in a file into a vector
     */
    void sort(const std::string &input_file);
};
#endif
