#ifndef TASK_MANAGER_HPP
#define TASK_MANAGER_HPP

#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <vector>

enum class ProcessingStage {
    Map,
    Reduce
};

enum class ProcessingStatus {
    Error,
    NotStarted,
    SentToStub,
    InProgress,
    Complete
};

struct File {
    int fileId;
    std::string fileName;
    ProcessingStage stage;
    ProcessingStatus status;
};

struct Task {
    int taskId;
    int fileId;
    int stubId;
    int batchId;
    ProcessingStage stage;
    ProcessingStatus status;
};

struct Batch {
    int batchId;
    int stubId;
    ProcessingStage stage;
    ProcessingStatus status;
    std::set<int> taskIds;
};

class TaskManager {
   private:
    std::vector<File> files;
    std::vector<Task> tasks;
    std::vector<Batch> batches;
    std::mutex& coutMutex;

    void addProcessFile(int fileId, const std::string& fileName, ProcessingStage stage, ProcessingStatus status);
    void addTask(int taskId, int fileId, int stubId, int batchId, ProcessingStage stage, ProcessingStatus status);
    void addBatch(int batchId, int stubId, ProcessingStage stage, ProcessingStatus status);
    void writeConsole(std::ostream& outputStream, const std::string& message);

   public:
    TaskManager(std::vector<std::string> filePaths, std::vector<int> stubIds, std::mutex& coutMutex);

    void assignTaskToBatch(int batchId, int taskId);
    void setTaskStub(int taskId, int stubId);
    void setFileStage(int fileId, ProcessingStage stage);
    void setFileStatus(int fileId, ProcessingStatus status);
    void setTaskStage(int taskId, ProcessingStage stage);
    void setTaskStatus(int taskId, ProcessingStatus status);
    void setBatchStage(int batchId, ProcessingStage stage);
    void setBatchStatus(int batchId, ProcessingStatus status);

    std::vector<Task> getMapTasks();
    std::vector<Task> getReduceTasks();
    std::vector<Batch> getMapBatches();
    std::vector<Batch> getReduceBatches();

    const std::vector<Batch>& getBatches() const;
    const std::vector<File>& getProcessFiles() const;
    const std::vector<Task>& getTasks() const;

    bool allMapBatchesComplete();
    bool allReduceBatchesComplete();

    std::string getProcessingStageString(ProcessingStage stage);
    std::string getProcessingStatusString(ProcessingStatus status);

    void printFiles();
    void printTasks();
    void printBatches();
    void print();
};

#endif  // TASK_MANAGER_HPP
