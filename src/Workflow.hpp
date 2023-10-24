#ifndef WORKFLOW_H
#define WORKFLOW_H

#include <string>

#include "FileManager.hpp"

class Workflow {
   private:
    std::string inputDir;   // The input directory for the workflow
    std::string tempDir;    // The temporary directory for the workflow
    std::string outputDir;  // The output directory for the workflow

    FileManager* fileManager;  // The file manager for the workflow

   public:
    /**
     * @brief Construct a new Workflow object
     *
     * @param inputDir The input directory for the workflow
     * @param tempDir The temporary directory for the workflow
     * @param outputDir The output directory for the workflow
     */
    Workflow(std::string inputDir, std::string tempDir, std::string outputDir);

    /**
     * @brief Get the input directory for the workflow
     *
     * @return std::string The input directory
     */
    std::string getInputDir() const;

    /**
     * @brief Set the input directory for the workflow
     *
     * @param inputDir The input directory
     */
    void setInputDir(std::string inputDir);

    /**
     * @brief Get the temporary directory for the workflow
     *
     * @return std::string The temporary directory
     */
    std::string getTempDir() const;

    /**
     * @brief Set the temporary directory for the workflow
     *
     * @param tempDir The temporary directory
     */
    void setTempDir(std::string tempDir);

    /**
     * @brief Get the output directory for the workflow
     *
     * @return std::string The output directory
     */
    std::string getOutputDir() const;

    /**
     * @brief Set the output directory for the workflow
     *
     * @param outputDir The output directory
     */
    void setOutputDir(std::string outputDir);

    /**
     * @brief Execute the workflow
     *
     */
    void execute();
};

#endif  // WORKFLOW_H
