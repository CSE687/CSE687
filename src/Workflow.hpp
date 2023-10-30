/////////////////////////////////////////////////////////////////////
//  Workflow.hpp    -   header file for Workflow class             //
//  Language        -   C++                                        //
/////////////////////////////////////////////////////////////////////

/*
 * The Workflow class manages the execution of the entire pipeline, from reading input files from the
 * user-provided input directory, to tokenizing each word and tallying their occurrence, to writing them
 * to the user-specified output directory.
 *
 * Required Files:
 * ---------------
 * FileManager.hpp, FileManager.cpp,
 * Reduce.hpp, Reduce.cpp, Map.hpp, Map.cpp,
 * Workflow.hpp, Workflow.cpp
 *
 * Maintenance History:
 * --------------------
 * - v1.0 : 29 Oct 2023
 *   Phase 1 release
 */

#ifndef WORKFLOW_H
#define WORKFLOW_H

#include <string>

#include "FileManager.hpp"

using namespace std;

typedef vector<int> counts;
class Workflow {
   private:
    FileManager* fileManager;  // The file manager for the workflow
    vector<string> input_files;
    vector<string> skippedFiles{};

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
