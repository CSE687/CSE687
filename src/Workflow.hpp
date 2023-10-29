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
     * @brief Puts the entire workflow together.
     *
     */
    void execute();
};

#endif  // WORKFLOW_H
