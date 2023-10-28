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
     * @brief Get the input directory for the workflow
     *
     * @return std::string The input directory
     */
    string getInputDir() const;

    /**
     * @brief Set the input directory for the workflow
     *
     * @param inputDir The input directory
     */
    void setInputDir(string inputDir);

    /**
     * @brief Get the temporary directory for the workflow
     *
     * @return std::string The temporary directory
     */
    string getTempDir() const;

    /**
     * @brief Set the temporary directory for the workflow
     *
     * @param tempDir The temporary directory
     */
    void setTempDir(string tempDir);

    /**
     * @brief Get the output directory for the workflow
     *
     * @return std::string The output directory
     */
    string getOutputDir() const;

    /**
     * @brief Set the output directory for the workflow
     *
     * @param outputDir The output directory
     */
    void setOutputDir(string outputDir);

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