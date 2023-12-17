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
#include "TaskManager.hpp"

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
