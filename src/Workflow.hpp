#ifndef WORKFLOW_H
#define WORKFLOW_H

#include <string>

#include "FileManager.hpp"

using namespace std;

typedef vector<int> counts;
class Workflow {
   private:
    FileManager* fileManager;
    vector<string> input_files;
    vector<string> skippedFiles{};  // The file manager for the workflow

   public:
    /**
     * @brief Construct a new Workflow object
     *
     * @param filemanager The filemanager to use for the workflow
     */
    Workflow(FileManager*);

    /**
     * @brief Runs the Map function and checks the status of the operation for any errors.
     *
     * @param file Overrides using the inputDir and sends just this file to Map.
     * @return int success or failure
     */
    int runMapper(string file);

    /**
     * @brief Runs the Reduce function and checks the status of the operation for any errors.
     *
     * @param file Overrides using the tempDir and sends just this file to Reduce.
     * @return int success or failure
     */
    int runReducer(string file);

    /**
     * @brief Puts the entire workflow together.
     *
     */
    void execute();
};

#endif  // WORKFLOW_H
