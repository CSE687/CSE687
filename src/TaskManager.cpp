#include "TaskManager.hpp"

// Constructor takes a vector of files, vector of stubIds
TaskManager::TaskManager(std::vector<std::string> filePaths, std::vector<int> stubIds, std::mutex& coutMutex) : coutMutex(coutMutex) {
    // Build vector of File
    int fileId = 0;
    for (const auto& fullPathFile : filePaths) {
        this->addProcessFile(fileId, fullPathFile, ProcessingStage::Map, ProcessingStatus::NotStarted);
        fileId++;
    }

    // Create a Batch for each stubId and ProcessingStage
    int batchId = 0;
    for (const auto& stubId : stubIds) {
        this->addBatch(batchId, stubId, ProcessingStage::Map, ProcessingStatus::NotStarted);
        batchId++;
        this->addBatch(batchId, stubId, ProcessingStage::Reduce, ProcessingStatus::NotStarted);
        batchId++;
    }

    // For each file in files, build a Task for each stage of processing
    int taskId = 0;
    for (const auto& file : this->files) {
        this->addTask(taskId, file.fileId, -1, -1, ProcessingStage::Map, ProcessingStatus::NotStarted);
        taskId++;
        this->addTask(taskId, file.fileId, -1, -1, ProcessingStage::Reduce, ProcessingStatus::NotStarted);
        taskId++;
    }

    // Round-robin the Map tasks between the Map Batches
    int mapBatchIndex = 0;
    std::vector<Batch> mapBatches = this->getMapBatches();
    for (const auto& task : this->getMapTasks()) {
        this->assignTaskToBatch(mapBatches[mapBatchIndex].batchId, task.taskId);
        mapBatchIndex = (mapBatchIndex + 1) % mapBatches.size();
    }

    // Round-robin the Reduce tasks between the Reduce Batches
    int reduceBatchIndex = 0;
    std::vector<Batch> reduceBatches = this->getReduceBatches();
    for (const auto& task : this->getReduceTasks()) {
        this->assignTaskToBatch(reduceBatches[reduceBatchIndex].batchId, task.taskId);
        reduceBatchIndex = (reduceBatchIndex + 1) % reduceBatches.size();
    }

    this->print();
}

// Add a process file to the task manager
void TaskManager::addProcessFile(int fileId, const std::string& fileName, ProcessingStage stage, ProcessingStatus status = ProcessingStatus::NotStarted) {
    File file;
    file.fileId = fileId;
    file.fileName = fileName;
    file.stage = stage;
    file.status = status;
    files.push_back(file);
}

// Add a task to the task manager
void TaskManager::addTask(int taskId, const int fileId, const int stubId, const int batchId, ProcessingStage stage, ProcessingStatus status) {
    Task task;
    task.taskId = taskId;
    task.fileId = fileId;
    task.stubId = stubId;
    task.batchId = batchId;
    task.stage = stage;
    task.status = status;
    tasks.push_back(task);
}

// Add a batch to the task manager
void TaskManager::addBatch(int batchId, int stubId, ProcessingStage stage, ProcessingStatus status) {
    Batch batch;
    batch.batchId = batchId;
    batch.stubId = stubId;
    batch.stage = stage;
    batch.status = status;
    batches.push_back(batch);
}

void TaskManager::writeConsole(std::ostream& outputStream, const std::string& message) {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::string messageWithPrefix = "TaskManager: ";
    outputStream << messageWithPrefix << message;
    outputStream.flush();
};

// Set a taskId to a batch
void TaskManager::assignTaskToBatch(int batchId, int taskId) {
    // Check which batchId the task is already assigned to and remove it from that batch
    int existingBatchId = tasks[taskId].batchId;
    if (existingBatchId != -1) {
        batches[existingBatchId].taskIds.erase(taskId);
    }
    // Add the task to the specified batch
    batches[batchId].taskIds.insert(taskId);
    // Set the task's batchId
    tasks[taskId].batchId = batchId;
    tasks[taskId].stubId = batches[batchId].stubId;
}

// Sets a task's stub to the specified stub
void TaskManager::setTaskStub(int taskId, int stubId) {
    tasks[taskId].stubId = stubId;
}

// Set the stage of a file
void TaskManager::setFileStage(int fileId, ProcessingStage stage) {
    files[fileId].stage = stage;
}

// Set the status of a file
void TaskManager::setFileStatus(int fileId, ProcessingStatus status) {
    files[fileId].status = status;
}

// Set a task's stage to the specified stage
void TaskManager::setTaskStage(int taskId, ProcessingStage stage) {
    tasks[taskId].stage = stage;
    // Set the stage of the file associated with the task
    this->setFileStage(tasks[taskId].fileId, stage);
}

// Sets a task's status to the specified status
void TaskManager::setTaskStatus(int taskId, ProcessingStatus status) {
    tasks[taskId].status = status;
    // Set the status of the file associated with the task
    this->setFileStatus(tasks[taskId].fileId, status);
}

// Set the stage of a batch
void TaskManager::setBatchStage(int batchId, ProcessingStage stage) {
    batches[batchId].stage = stage;
    // Set the stage of all tasks in the batch
    for (const auto& taskId : batches[batchId].taskIds) {
        this->setTaskStage(taskId, stage);
    }
}

// Set the status of a batch
void TaskManager::setBatchStatus(int batchId, ProcessingStatus status) {
    // Check if batchId exists, if not - log and error and return
    if (batchId >= batches.size() || batchId < 0) {
        this->writeConsole(std::cout, "Error: batchId " + std::to_string(batchId) + " does not exist\n");
        return;
    }
    batches[batchId].status = status;
    // Set the status of all tasks in the batch
    for (const auto& taskId : batches[batchId].taskIds) {
        this->setTaskStatus(taskId, status);
    }
}

// Get the list of Map Tasks
std::vector<Task> TaskManager::getMapTasks() {
    std::vector<Task> mapTasks;
    for (const auto& task : tasks) {
        if (task.stage == ProcessingStage::Map) {
            mapTasks.push_back(task);
        }
    }
    return mapTasks;
}

// Get the list of Reduce Tasks
std::vector<Task> TaskManager::getReduceTasks() {
    std::vector<Task> reduceTasks;
    for (const auto& task : tasks) {
        if (task.stage == ProcessingStage::Reduce) {
            reduceTasks.push_back(task);
        }
    }
    return reduceTasks;
}

// Get the list of Map Batches
std::vector<Batch> TaskManager::getMapBatches() {
    std::vector<Batch> mapBatches;
    for (const auto& batch : batches) {
        if (batch.stage == ProcessingStage::Map) {
            mapBatches.push_back(batch);
        }
    }
    return mapBatches;
}

// Get the list of Reduce Batches
std::vector<Batch> TaskManager::getReduceBatches() {
    std::vector<Batch> reduceBatches;
    for (const auto& batch : batches) {
        if (batch.stage == ProcessingStage::Reduce) {
            reduceBatches.push_back(batch);
        }
    }
    return reduceBatches;
}

// Get the list of Batches
const std::vector<Batch>& TaskManager::getBatches() const {
    return batches;
}

// Get the list of process files
const std::vector<File>& TaskManager::getProcessFiles() const {
    return files;
}

// Get the list of tasks
const std::vector<Task>& TaskManager::getTasks() const {
    return tasks;
}

// Boolean to determine if all map stage batches are complete
bool TaskManager::allMapBatchesComplete() {
    for (const auto& batch : this->getMapBatches()) {
        if (batch.status != ProcessingStatus::Complete) {
            return false;
        }
    }
    return true;
}

// Boolean to determine if all reduce stage batches are complete
bool TaskManager::allReduceBatchesComplete() {
    for (const auto& batch : this->getReduceBatches()) {
        if (batch.status != ProcessingStatus::Complete) {
            return false;
        }
    }
    return true;
}

// Return the string equivalent of the ProcessingStage
std::string TaskManager::getProcessingStageString(ProcessingStage stage) {
    switch (stage) {
        case ProcessingStage::Map:
            return "Map";
        case ProcessingStage::Reduce:
            return "Reduce";
        default:
            return "Unknown";
    }
}

// Return the string equivalent of the ProcessingStatus
std::string TaskManager::getProcessingStatusString(ProcessingStatus status) {
    switch (status) {
        case ProcessingStatus::Error:
            return "Error";
        case ProcessingStatus::NotStarted:
            return "NotStarted";
        case ProcessingStatus::SentToStub:
            return "SentToStub";
        case ProcessingStatus::InProgress:
            return "InProgress";
        case ProcessingStatus::Complete:
            return "Complete";
        default:
            return "Unknown";
    }
}

// Print the contents of the files vector
void TaskManager::printFiles() {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::cout << "Files: " << std::endl;
    for (const auto& file : files) {
        std::cout << "File ID: " << file.fileId << "; File Name: " << file.fileName << "; Stage: " << this->getProcessingStageString(file.stage) << "; Status: " << this->getProcessingStatusString(file.status) << std::endl;
    }
    std::cout << std::endl;
}

// Print the contents of the tasks vector
void TaskManager::printTasks() {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::cout << "Tasks: " << std::endl;
    for (const auto& task : tasks) {
        std::cout << "Task ID: " << task.taskId << "; File ID: " << task.fileId << "; Stub ID: " << task.stubId << "; Batch ID: " << task.batchId << "; Stage: " << this->getProcessingStageString(task.stage) << "; Status: " << this->getProcessingStatusString(task.status) << std::endl;
    }
    std::cout << std::endl;
}

// Print the contents of the batches vector
void TaskManager::printBatches() {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::cout << "Batches: " << std::endl;
    for (const auto& batch : batches) {
        std::cout << "Batch ID: " << batch.batchId << "; Stub ID: " << batch.stubId << "; Stage: " << this->getProcessingStageString(batch.stage) << "; Status: " << this->getProcessingStatusString(batch.status) << std::endl;
    }
    std::cout << std::endl;
}

// Print the contents of the TaskManager
void TaskManager::print() {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::cout << "TaskManager: " << std::endl;
    std::cout << "Files: " << std::endl;
    for (const auto& file : files) {
        std::cout << "File ID: " << file.fileId << "; File Name: " << file.fileName << "; Stage: " << this->getProcessingStageString(file.stage) << "; Status: " << this->getProcessingStatusString(file.status) << std::endl;
    }
    std::cout << "Tasks: " << std::endl;
    for (const auto& task : tasks) {
        std::cout << "Task ID: " << task.taskId << "; File ID: " << task.fileId << "; Stub ID: " << task.stubId << "; Batch ID: " << task.batchId << "; Stage: " << this->getProcessingStageString(task.stage) << "; Status: " << this->getProcessingStatusString(task.status) << std::endl;
    }
    std::cout << "Batches: " << std::endl;
    for (const auto& batch : batches) {
        std::string taskIdsStr;
        for (const auto& taskId : batch.taskIds) {
            taskIdsStr += std::to_string(taskId) + " ";
        }
        std::cout << "Batch ID: " << batch.batchId << "; Stub ID: " << batch.stubId << "; Stage: " << this->getProcessingStageString(batch.stage) << "; Status: " << this->getProcessingStatusString(batch.status) << "; TaskIds: " << taskIdsStr << std::endl;
    }
    std::cout << std::endl;
}
