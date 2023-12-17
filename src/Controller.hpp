#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <chrono>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "FileManager.hpp"
#include "PropertyTreeQueue.hpp"
#include "StubConnection.hpp"
#include "StubHeartbeat.hpp"

// Enum for stages of processing
enum class ProcessingStage : int {
    Map,
    Reduce,
};

// Enum for processing status
enum class ProcessingStatus : int {
    Error,
    NotStarted,
    SentToStub,
    InProgress,
    Complete,
};

// Struct tracks the files that have been processed by the MapReduce process
struct File {
    int fileId;
    std::string fileName;
    ProcessingStage stage;
    ProcessingStatus status;
};

// Struct tracks in progress tasks, the stub the task is assigned to, and the file being processed
struct Task {
    int taskId;
    int fileId;
    int stubId;
    int batchId;
    ProcessingStage stage;
    ProcessingStatus status;
};

// Struct tracks the batches of tasks that are sent to stubs
struct Batch {
    int batchId;
    int stubId;
    std::set<int> taskIds;
    ProcessingStage stage;
    ProcessingStatus status;
};

// TaskManager class manages the tasks and their assignments
class TaskManager {
   private:
    std::vector<File> files;
    std::vector<Task> tasks;
    std::vector<Batch> batches;

    std::mutex& coutMutex;
    std::mutex taskManagerMutex;

    // Add a process file to the task manager
    void addProcessFile(int fileId, const std::string& fileName, ProcessingStage stage, ProcessingStatus status = ProcessingStatus::NotStarted) {
        File file;
        file.fileId = fileId;
        file.fileName = fileName;
        file.stage = stage;
        file.status = status;
        files.push_back(file);
    }

    // Add a task to the task manager
    void addTask(int taskId, const int& fileId, const int& stubId, const int& batchId, ProcessingStage stage, ProcessingStatus status) {
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
    void addBatch(int batchId, int stubId, ProcessingStage stage, ProcessingStatus status) {
        Batch batch;
        batch.batchId = batchId;
        batch.stubId = stubId;
        batch.stage = stage;
        batch.status = status;
        batches.push_back(batch);
    }

    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::string messageWithPrefix = "TaskManager: ";
        outputStream << messageWithPrefix << message;
        outputStream.flush();
    };

   public:
    // Constructor takes a vector of files, vector of stubIds
    TaskManager(std::vector<std::string> filePaths, std::vector<int> stubIds, std::mutex& coutMutex) : coutMutex(coutMutex) {
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

    // Set a taskId to a batch
    void assignTaskToBatch(int batchId, int taskId) {
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
    void setTaskStub(int taskId, int stubId) {
        tasks[taskId].stubId = stubId;
    }

    // Set the stage of a file
    void setFileStage(int fileId, ProcessingStage stage) {
        files[fileId].stage = stage;
    }

    // Set the status of a file
    void setFileStatus(int fileId, ProcessingStatus status) {
        files[fileId].status = status;
    }

    // Set a task's stage to the specified stage
    void setTaskStage(int taskId, ProcessingStage stage) {
        tasks[taskId].stage = stage;
        // Set the stage of the file associated with the task
        this->setFileStage(tasks[taskId].fileId, stage);
    }

    // Sets a task's status to the specified status
    void setTaskStatus(int taskId, ProcessingStatus status) {
        tasks[taskId].status = status;
        // Set the status of the file associated with the task
        this->setFileStatus(tasks[taskId].fileId, status);
    }

    // Set the stage of a batch
    void setBatchStage(int batchId, ProcessingStage stage) {
        batches[batchId].stage = stage;
        // Set the stage of all tasks in the batch
        for (const auto& taskId : batches[batchId].taskIds) {
            this->setTaskStage(taskId, stage);
        }
    }

    // Set the status of a batch
    void setBatchStatus(int batchId, ProcessingStatus status) {
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
    std::vector<Task> getMapTasks() {
        std::vector<Task> mapTasks;
        for (const auto& task : tasks) {
            if (task.stage == ProcessingStage::Map) {
                mapTasks.push_back(task);
            }
        }
        return mapTasks;
    }

    // Get the list of Reduce Tasks
    std::vector<Task> getReduceTasks() {
        std::vector<Task> reduceTasks;
        for (const auto& task : tasks) {
            if (task.stage == ProcessingStage::Reduce) {
                reduceTasks.push_back(task);
            }
        }
        return reduceTasks;
    }

    // Get the list of Map Batches
    std::vector<Batch> getMapBatches() {
        std::vector<Batch> mapBatches;
        for (const auto& batch : batches) {
            if (batch.stage == ProcessingStage::Map) {
                mapBatches.push_back(batch);
            }
        }
        return mapBatches;
    }

    // Get the list of Reduce Batches
    std::vector<Batch> getReduceBatches() {
        std::vector<Batch> reduceBatches;
        for (const auto& batch : batches) {
            if (batch.stage == ProcessingStage::Reduce) {
                reduceBatches.push_back(batch);
            }
        }
        return reduceBatches;
    }

    // Get the list of Batches
    const std::vector<Batch>& getBatches() const {
        return batches;
    }

    // Get the list of process files
    const std::vector<File>& getProcessFiles() const {
        return files;
    }

    // Get the list of tasks
    const std::vector<Task>& getTasks() const {
        return tasks;
    }

    // Boolean to determine if all map stage batches are complete
    bool allMapBatchesComplete() {
        for (const auto& batch : this->getMapBatches()) {
            if (batch.status != ProcessingStatus::Complete) {
                return false;
            }
        }
        return true;
    }

    // Boolean to determine if all reduce stage batches are complete
    bool allReduceBatchesComplete() {
        for (const auto& batch : this->getReduceBatches()) {
            if (batch.status != ProcessingStatus::Complete) {
                return false;
            }
        }
        return true;
    }

    // Return the string equivalent of the ProcessingStage
    std::string getProcessingStageString(ProcessingStage stage) {
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
    std::string getProcessingStatusString(ProcessingStatus status) {
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
    void printFiles() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Files: " << std::endl;
        for (const auto& file : files) {
            std::cout << "File ID: " << file.fileId << "; File Name: " << file.fileName << "; Stage: " << this->getProcessingStageString(file.stage) << "; Status: " << this->getProcessingStatusString(file.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the tasks vector
    void printTasks() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Tasks: " << std::endl;
        for (const auto& task : tasks) {
            std::cout << "Task ID: " << task.taskId << "; File ID: " << task.fileId << "; Stub ID: " << task.stubId << "; Batch ID: " << task.batchId << "; Stage: " << this->getProcessingStageString(task.stage) << "; Status: " << this->getProcessingStatusString(task.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the batches vector
    void printBatches() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Batches: " << std::endl;
        for (const auto& batch : batches) {
            std::cout << "Batch ID: " << batch.batchId << "; Stub ID: " << batch.stubId << "; Stage: " << this->getProcessingStageString(batch.stage) << "; Status: " << this->getProcessingStatusString(batch.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the TaskManager
    void print() {
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
};

// Controller class is responsible for managing the entire MapReduce process
class Controller {
   private:
    FileManager* fileManager;
    int port;
    std::vector<std::shared_ptr<StubConnection>> stubConnections;
    std::mutex coutMutex;

    std::shared_ptr<TaskManager> taskManager;

    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::string messageWithPrefix = "Controller: ";
        outputStream << messageWithPrefix << message;
        outputStream.flush();
    };

   public:
    Controller(FileManager* fileManager, int port, std::vector<int>& ports) : fileManager(fileManager), port(port) {
        // Create socket connections to stubs
        for (int i = 0; i < ports.size(); i++) {
            this->createStubConnection(i, ports[i]);
        }
        // create a list of stubIds from the stubConnections vector
        std::vector<int> stubIds;
        for (const auto& stubConnection : stubConnections) {
            stubIds.push_back(stubConnection->getStubId());
        }

        // Create string vector of file names including .txt extension
        std::vector<std::string> fileNames;
        for (const auto& file : this->fileManager->getDirectoryFileList(this->fileManager->getInputDirectory())) {
            fileNames.push_back(this->fileManager->getFileStem(file) + ".txt");
        }

        // Initialize TaskManager
        this->taskManager = std::make_shared<TaskManager>(fileNames, stubIds, this->coutMutex);
    }

    // Create socket connection to Stub process which is listening on user-specified port
    void createStubConnection(int stub_id, int port) {
        // Create a new StubConnection object and add it to the stubConnections vector
        std::cout << "Creating connection to stub " << stub_id << " on port " << port << std::endl;
        stubConnections.push_back(std::make_shared<StubConnection>(stub_id, port, this->fileManager->getInputDirectory(), this->fileManager->getOutputDirectory(), this->fileManager->getTempDirectory(), this->coutMutex, 5));
        stubConnections.back()->start();
    }

    void execute() {
        // Run Map process on files
        // Run Reduce process on files
        while (true) {
            // Complete all Map Batches before moving on to Reduce Batches
            if (!this->taskManager->allMapBatchesComplete()) {
                // this->writeConsole(std::cout, "Map Batches not complete\n");
                // Place all Map Batch tasks with status NotStarted into the sendPTreeQueue
                for (const auto& batch : this->taskManager->getMapBatches()) {
                    // Verify that the batch status is NotStarted and that the stubId isAlive
                    if (batch.status == ProcessingStatus::NotStarted && this->stubConnections[batch.stubId]->getIsAlive()) {
                        // Create a ptree object
                        boost::property_tree::ptree pt;
                        pt.put("message_type", "map_task");
                        pt.put("batch_id", std::to_string(batch.batchId));
                        // create a string vector of file names from the taskIds in the batch
                        std::vector<std::string> fileNames;
                        for (const auto& taskId : batch.taskIds) {
                            fileNames.push_back(this->taskManager->getProcessFiles()[this->taskManager->getTasks()[taskId].fileId].fileName);
                        }
                        // put the fileNames vector into the ptree element called "files" as comma separated values
                        boost::property_tree::ptree files;
                        std::string fileNamesStr;
                        for (const auto& fileName : fileNames) {
                            fileNamesStr += fileName + ",";
                        }
                        if (!fileNamesStr.empty()) {
                            fileNamesStr.pop_back();  // Remove the trailing comma
                        }
                        files.put("", fileNamesStr);
                        pt.add_child("files", files);

                        // Convert the ptree to a string, including the child elements
                        std::ostringstream buf;
                        boost::property_tree::write_json(buf, pt, false);
                        std::string jsonString = buf.str();
                        // write the message to the console
                        this->writeConsole(std::cout, "Writing message to Stub " + std::to_string(batch.stubId) + " sendPTreeQueue: " + jsonString + "\n");

                        // Add the ptree to the assigned stubId's sendPTreeQueue
                        this->stubConnections[batch.stubId]->sendPTreeQueue.queueMutex.lock();
                        this->stubConnections[batch.stubId]->sendPTreeQueue.push(pt);
                        this->stubConnections[batch.stubId]->sendPTreeQueue.queueMutex.unlock();

                        // Set the batch status to SentToStub
                        this->taskManager->setBatchStatus(batch.batchId, ProcessingStatus::SentToStub);
                    }
                }
            } else if (!this->taskManager->allReduceBatchesComplete()) {
                // Place all Reduce Batch tasks with status NotStarted into the sendPTreeQueue
                for (const auto& batch : this->taskManager->getReduceBatches()) {
                    if (batch.status == ProcessingStatus::NotStarted && this->stubConnections[batch.stubId]->getIsAlive()) {
                        // Create a ptree object
                        boost::property_tree::ptree pt;
                        pt.put("message_type", "reduce_task");
                        pt.put("batch_id", std::to_string(batch.batchId));
                        // create a string vector of file names from the taskIds in the batch
                        std::vector<std::string> fileNames;
                        for (const auto& taskId : batch.taskIds) {
                            fileNames.push_back(this->taskManager->getProcessFiles()[this->taskManager->getTasks()[taskId].fileId].fileName);
                        }
                        // add the fileNames vector into the ptree
                        boost::property_tree::ptree files;
                        std::string fileNamesStr;
                        for (const auto& fileName : fileNames) {
                            fileNamesStr += fileName + ",";
                        }
                        if (!fileNamesStr.empty()) {
                            fileNamesStr.pop_back();  // Remove the trailing comma
                        }
                        files.put("", fileNamesStr);
                        pt.add_child("files", files);

                        // Add the ptree to the assigned stubId's sendPTreeQueue
                        this->stubConnections[batch.stubId]->sendPTreeQueue.queueMutex.lock();
                        this->stubConnections[batch.stubId]->sendPTreeQueue.push(pt);
                        this->stubConnections[batch.stubId]->sendPTreeQueue.queueMutex.unlock();

                        // Set the batch status to SentToStub
                        this->taskManager->setBatchStatus(batch.batchId, ProcessingStatus::SentToStub);
                    }
                }
            } else if (this->taskManager->allMapBatchesComplete() && this->taskManager->allReduceBatchesComplete()) {
                // All batches are complete, exit the loop - we done
                this->writeConsole(std::cout, "All batches complete, exiting\n");
                this->taskManager->print();
                break;
            }

            // Check for messages in message queues and act on them
            for (const auto& stubConnection : stubConnections) {
                // Check if there are messages in the receivePTreeQueue
                stubConnection->receivePTreeQueue.queueMutex.lock();
                if (stubConnection->receivePTreeQueue.hasMessage()) {
                    // Pop the message from the receivePTreeQueue
                    boost::property_tree::ptree pt = stubConnection->receivePTreeQueue.pop();
                    stubConnection->receivePTreeQueue.queueMutex.unlock();

                    // Convert the ptree to a JSON string
                    std::ostringstream buf;
                    boost::property_tree::write_json(buf, pt, false);
                    std::string jsonString = buf.str();
                    // write the message to the console
                    this->writeConsole(std::cout, "Received message from Stub " + std::to_string(stubConnection->getStubId()) + ": " + jsonString + "\n");

                    try {
                        // Check the message type
                        std::string messageType = pt.get<std::string>("message_type");
                        if (messageType == "batch_status") {
                            // Get the batch_id and status from the message
                            int batchId = std::stoi(pt.get<std::string>("batch_id"));
                            // convert message status to ProcessingStatus enum
                            std::string status = pt.get<std::string>("status");
                            //  set the batch status to the status from the message
                            if (status == "InProgress") {
                                this->taskManager->setBatchStatus(batchId, ProcessingStatus::InProgress);
                                this->taskManager->printBatches();
                            } else if (status == "Complete") {
                                this->taskManager->setBatchStatus(batchId, ProcessingStatus::Complete);
                                this->taskManager->printBatches();
                            } else if (status == "Error") {
                                this->taskManager->setBatchStatus(batchId, ProcessingStatus::Error);
                                this->taskManager->printBatches();
                            }
                        } else if (messageType == "connection_closed") {
                            // get the BatchIds which are running on the stub and set their status to NotStarted
                            for (const auto& batch : this->taskManager->getBatches()) {
                                if (batch.stubId == stubConnection->getStubId() && batch.status != ProcessingStatus::Complete) {
                                    this->taskManager->setBatchStatus(batch.batchId, ProcessingStatus::NotStarted);
                                }
                            }
                        } else {
                            this->writeConsole(std::cout, "Unknown message type: " + messageType + "\n");
                        }
                    } catch (const boost::property_tree::ptree_bad_path& e) {
                        this->writeConsole(std::cout, "Property tree node does not exist: " + std::string(e.what()) + "\n");
                    }
                }
                stubConnection->receivePTreeQueue.queueMutex.unlock();
            }
        }
    };
};
#endif  // CONTROLLER_HPP
