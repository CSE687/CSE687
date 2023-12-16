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

class PropertyTreeQueue {
   private:
    int stub_id;
    std::queue<boost::property_tree::ptree> queue;
    std::mutex& coutMutex;

    // thread-safe console writing
    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::string messageWithPrefix = "Stub ID: " + std::to_string(this->stub_id) + "; PropertyTreeQueue: ";
        outputStream << messageWithPrefix << message;
        outputStream.flush();
    };

   public:
    std::mutex queueMutex;

    PropertyTreeQueue(std::mutex& coutMutex, int stub_id) : coutMutex(coutMutex), stub_id(stub_id) {}

    // push a message to the back of the queue
    void push(const boost::property_tree::ptree& message) {
        // LOGGING //
        // Convert the ptree to a string, including the child elements
        std::ostringstream buf;
        boost::property_tree::write_json(buf, message, false);
        std::string jsonString = buf.str();
        // write the message to the console
        this->writeConsole(std::cout, "push(): " + jsonString + "\n");
        // LOGGING //

        queue.push(message);
    }

    // pop the first message from the queue
    boost::property_tree::ptree pop() {
        if (this->queue.empty()) {
            this->writeConsole(std::cout, "Warning: Tried to retrieve message from PropertyTreeQueue but no message exists\n");
        }

        // Get the message from the front of the queue
        boost::property_tree::ptree message = this->queue.front();
        this->queue.pop();

        // LOGGING //
        // Convert the ptree to a string, including the child elements
        std::ostringstream buf;
        boost::property_tree::write_json(buf, message, false);
        std::string jsonString = buf.str();
        // write the message to the console
        this->writeConsole(std::cout, "pop(): " + jsonString + "\n");
        // LOGGING //

        return message;
    }

    // check if the queue has a message
    bool hasMessage() const {
        return !this->queue.empty();
    }

    // remove all messages from the queue
    void clear() {
        while (!this->queue.empty()) {
            this->queue.pop();
        }
    }
};

class StubHeartbeat {
   private:
    std::chrono::system_clock::time_point timeLastMessageSent;
    std::chrono::system_clock::time_point timeLastMessageReceived;
    std::chrono::system_clock::time_point timeLastHeartbeatSent;
    std::chrono::duration<double> heartbeatCadenceSeconds;

    int unresponsiveHeartbeatsSent;

    // calculate the time since the last heartbeat was sent
    std::chrono::duration<double> timeSinceLastMessageSent() {
        return std::chrono::system_clock::now() - this->timeLastMessageSent;
    }

    // calculate the time since the last heartbeat was received
    std::chrono::duration<double> timeSinceLastMessageReceived() {
        return std::chrono::system_clock::now() - this->timeLastMessageReceived;
    }

    // calculate the time since the last heartbeat was sent
    std::chrono::duration<double> timeSinceLastHeartbeatSent() {
        return std::chrono::system_clock::now() - this->timeLastHeartbeatSent;
    }

   public:
    std::mutex mutex;

    StubHeartbeat(int heartbeatCadenceSeconds) : unresponsiveHeartbeatsSent(0), heartbeatCadenceSeconds(std::chrono::seconds(heartbeatCadenceSeconds)) {
        // Initialize time points to the current time
        timeLastMessageSent = std::chrono::system_clock::now();
        timeLastMessageReceived = std::chrono::system_clock::now();
        timeLastHeartbeatSent = std::chrono::system_clock::now();
    }

    void setTimeLastMessageSent(const std::chrono::system_clock::time_point& time) {
        timeLastMessageSent = time;
    }

    void setTimeLastMessageReceived(const std::chrono::system_clock::time_point& time) {
        timeLastMessageReceived = time;
        this->resetUnresponsiveHeartbeatsSent();
    }

    void setTimeLastHeartbeatSent(const std::chrono::system_clock::time_point& time) {
        timeLastHeartbeatSent = time;
    }

    void setHeartbeatCadenceSeconds(const std::chrono::duration<double>& cadence) {
        heartbeatCadenceSeconds = cadence;
    }

    void incrementUnresponsiveHeartbeatsSent() {
        unresponsiveHeartbeatsSent++;
    }

    void resetUnresponsiveHeartbeatsSent() {
        unresponsiveHeartbeatsSent = 0;
    }

    std::chrono::system_clock::time_point getTimeLastMessageSent() const {
        return timeLastMessageSent;
    }

    std::chrono::system_clock::time_point getTimeLastMessageReceived() const {
        return timeLastMessageReceived;
    }

    std::chrono::system_clock::time_point getTimeLastHeartbeatSent() const {
        return timeLastHeartbeatSent;
    }

    std::chrono::duration<double> getHeartbeatCadenceSeconds() const {
        return heartbeatCadenceSeconds;
    }

    int getUnresponsiveHeartbeatsSent() const {
        return unresponsiveHeartbeatsSent;
    }

    // determine if a heartbeat should be sent
    bool shouldSendHeartbeat() {
        // If the time since the last message sent exceeds the heartbeat cadence, send a heartbeat
        if (this->timeSinceLastMessageSent() > this->heartbeatCadenceSeconds && this->timeSinceLastHeartbeatSent() > this->heartbeatCadenceSeconds) {
            return true;
        }
        return false;
    }
};

class StubConnection {
   private:
    int stub_id;
    int port;

    // Heartbeat struct to track the last heartbeat sent and received from the stub
    StubHeartbeat heartbeat;
    bool isAlive;

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    std::thread connectThread;
    std::thread receiveThread;
    std::thread sendThread;

    std::string input_directory;
    std::string output_directory;
    std::string temp_directory;

    std::mutex& coutMutex;

    // Function to continuously try to establish a connection to the Stub
    void connect() {
        this->writeConsole(std::cout, "connectThread started\n");
        while (true) {
            if (!this->isAlive) {
                // Attempt to reconnect to the stub
                this->writeConsole(std::cout, "Attempting to establish connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");

                // Close the socket if it's open
                if (this->socket.is_open()) {
                    this->socket.close();
                }

                // Attempt to connect to the Stub
                boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), this->port);
                boost::system::error_code error;

                this->socket.connect(endpoint, error);

                if (!error) {
                    this->writeConsole(std::cout, "Established connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");

                    // Place the pt object in the sendPTreeQueue
                    boost::property_tree::ptree pt;
                    pt.put("message_type", "establish_connection");
                    pt.put("message", "Establish connection to stub");
                    pt.put("input_directory", input_directory);
                    pt.put("output_directory", output_directory);
                    pt.put("temp_directory", temp_directory);

                    // Add the ptree to the sendPTreeQueue
                    this->sendPTreeQueue.queueMutex.lock();
                    this->sendPTreeQueue.push(pt);
                    this->isAlive = true;
                    this->sendPTreeQueue.queueMutex.unlock();
                } else {
                    this->writeConsole(std::cerr, "Failed to establish connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n" + "Error: " + error.message() + "\n");
                    // Sleep before attempting to reconnect
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
            } else {
                // Send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
                if (this->heartbeat.shouldSendHeartbeat()) {
                    this->sendHeartbeat();
                    // Put this thread to sleep for 1 second
                    // this->writeConsole(std::cout, "connect sleeping\n");
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
        this->writeConsole(std::cout, "connectThread ending\n");
    }

    // Function to continuously listen for messages on the socket and append them to receivePTreeQueue
    void receiveMessages() {
        this->writeConsole(std::cout, "receiveThread started\n");
        while (true) {
            while (isAlive) {
                // Sleep for 100ms
                // this->writeConsole(std::cout, "receiveMessages sleeping\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // Receive and convert the message to PropertyTree object
                boost::array<char, 128> buf;
                boost::system::error_code error;
                size_t len = socket.read_some(boost::asio::buffer(buf), error);

                // If there is an error, print it to the console
                if (error == boost::asio::error::eof) {
                    this->writeConsole(std::cout, "Connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + " closed by peer\n");
                    this->setIsAliveFalse();
                    break;  // Connection closed cleanly by peer.
                } else if (error) {
                    this->writeConsole(std::cout, "Error: " + error.message() + "\n");
                    this->setIsAliveFalse();
                }

                // Convert the received message to a PropertyTree object
                std::string message(buf.data(), len);
                std::stringstream ss;
                ss << message;
                boost::property_tree::ptree receivedPTree;
                try {
                    boost::property_tree::read_json(ss, receivedPTree);
                } catch (const boost::property_tree::json_parser_error& e) {
                    // Handle the exception (e.g., print error message)
                    this->writeConsole(std::cerr, "Failed to read JSON: " + std::string(e.what()) + "\n");
                    continue;
                }

                // Safely read the message_type key from the received PropertyTree object
                try {
                    std::string message = receivedPTree.get<std::string>("message_type");
                    // If the message type is "ack", continue
                    if (message == "ack") {
                        this->writeConsole(std::cout, "Received ack from stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");
                        continue;
                    } else {
                        // Append the received PropertyTree object to receivePTreeQueue
                        this->writeConsole(std::cout, "Received message from stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");
                        this->writeConsole(std::cout, "Writing message to Stub " + std::to_string(this->stub_id) + " receivePTreeQueue\n");

                        this->receivePTreeQueue.queueMutex.lock();
                        receivePTreeQueue.push(receivedPTree);
                        this->receivePTreeQueue.queueMutex.unlock();
                    }
                } catch (const boost::property_tree::ptree_bad_path& e) {
                    // Handle the exception (e.g., print error message)
                    this->writeConsole(std::cerr, "Failed to read 'message_type' key from receivedPTree: " + std::string(e.what()) + "\n");
                    continue;
                }
            }
            // Sleep for 1s if the connection is not alive
            this->writeConsole(std::cout, "!isAlive: receiveMessages sleeping\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        this->writeConsole(std::cout, "receiveThread ending\n");
    }

    // Function to continuously monitor sendPTreeQueue and send PropertyTree objects on the socket
    void sendMessages() {
        this->writeConsole(std::cout, "sendThread started\n");
        while (true) {
            while (this->isAlive) {
                // Sleep for 100ms
                // this->writeConsole(std::cout, "sendMessages sleeping\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // Check if there are PropertyTree objects in sendPTreeQueue
                this->sendPTreeQueue.queueMutex.lock();
                if (sendPTreeQueue.hasMessage()) {
                    this->writeConsole(std::cout, "Sending message to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");

                    // Pop the PropertyTree object from sendPTreeQueue
                    boost::property_tree::ptree pt = sendPTreeQueue.pop();

                    // Convert the PropertyTree object to a JSON string
                    std::ostringstream buf;
                    boost::property_tree::write_json(buf, pt, false);
                    std::string jsonString = buf.str();

                    // Send the JSON string to the stub
                    try {
                        this->sendMessage(pt);
                    } catch (const boost::system::system_error& e) {
                        this->writeConsole(std::cerr, "Error sending message: " + std::string(e.what()) + "\n");
                        this->setIsAliveFalse();
                    }
                }
                this->sendPTreeQueue.queueMutex.unlock();
            }
            // Sleep for 1s if the connection is not alive
            this->writeConsole(std::cout, "!isAlive: sendMessages sleeping\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        this->writeConsole(std::cout, "sendThread ending\n");
    };

    // thread-safe console writing
    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::string messageWithPrefix = "Stub ID: " + std::to_string(this->stub_id) + "; StubConnection: ";
        outputStream << messageWithPrefix << message;
        outputStream.flush();
    };

    // send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
    void sendHeartbeat() {
        // Create a ptree object
        boost::property_tree::ptree pt;
        pt.put("message_type", "heartbeat");
        pt.put("message", "heartbeat");

        this->heartbeat.incrementUnresponsiveHeartbeatsSent();
        this->writeConsole(std::cout, "Writing heartbeat to Stub " + std::to_string(this->stub_id) + " sendPTreeQueue. Unresponsive Count: " + std::to_string(this->heartbeat.getUnresponsiveHeartbeatsSent()) + "\n");
        std::lock_guard<std::mutex> lock(this->sendPTreeQueue.queueMutex);
        this->sendPTreeQueue.push(pt);
    };

    // Send a message to the stub
    void sendMessage(const boost::property_tree::ptree pt) {
        // Convert the ptree to a JSON string
        std::ostringstream buf;
        boost::property_tree::write_json(buf, pt, false);
        std::string jsonString = buf.str();

        // LOGGING //
        this->writeConsole(std::cout, "sendMessage(): " + jsonString + "\n");
        // LOGGING //

        // Send the JSON string to the stub
        boost::asio::write(this->socket, boost::asio::buffer(jsonString));

        // Update the last message sent time
        this->heartbeat.setTimeLastMessageSent(std::chrono::system_clock::now());
    }

    // set isAlive to false
    void setIsAliveFalse() {
        this->writeConsole(std::cout, "Setting isAlive to false\n");
        this->isAlive = false;

        // Clear the send and receive buffers
        std::lock(this->sendPTreeQueue.queueMutex, this->receivePTreeQueue.queueMutex);

        // Clear the buffers
        this->sendPTreeQueue.clear();
        this->receivePTreeQueue.clear();

        // Place message in receivePTreeQueue to notify the Controller that the connection is no longer alive
        boost::property_tree::ptree pt;
        pt.put("message_type", "connection_closed");
        this->receivePTreeQueue.push(pt);

        // Unlock buffers
        this->sendPTreeQueue.queueMutex.unlock();
        this->receivePTreeQueue.queueMutex.unlock();
    }

   public:
    // circular buffer for property_tree
    PropertyTreeQueue receivePTreeQueue;
    PropertyTreeQueue sendPTreeQueue;

    StubConnection(int stub_id,
                   int port,
                   std::string input_directory,
                   std::string output_directory,
                   std::string temp_directory,
                   std::mutex& coutMutex,
                   int heartbeatCadenceSeconds = 0) : stub_id(stub_id),
                                                      port(port),
                                                      input_directory(input_directory),
                                                      output_directory(output_directory),
                                                      temp_directory(temp_directory),
                                                      isAlive(false),
                                                      socket(io_context),
                                                      coutMutex(coutMutex),
                                                      receivePTreeQueue(coutMutex, stub_id),
                                                      sendPTreeQueue(coutMutex, stub_id),
                                                      heartbeat(heartbeatCadenceSeconds) {
    }

    void start() {
        // Create and start the receive and send threads
        connectThread = std::thread(&StubConnection::connect, this);
        connectThread.detach();
        receiveThread = std::thread(&StubConnection::receiveMessages, this);
        receiveThread.detach();
        sendThread = std::thread(&StubConnection::sendMessages, this);
        sendThread.detach();
    };

    // Getter for stub_id
    int getStubId() const {
        return stub_id;
    }

    // Getter for port
    int getPort() const {
        return port;
    }

    // Getter for heartbeat
    const StubHeartbeat& getHeartbeat() const {
        return heartbeat;
    }

    // Getter for isAlive
    bool getIsAlive() const {
        return isAlive;
    }
};

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

    // Print the contents of the files vector
    void printFiles() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Files: " << std::endl;
        for (const auto& file : files) {
            std::cout << "File ID: " << file.fileId << "; File Name: " << file.fileName << "; Stage: " << static_cast<int>(file.stage) << "; Status: " << static_cast<int>(file.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the tasks vector
    void printTasks() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Tasks: " << std::endl;
        for (const auto& task : tasks) {
            std::cout << "Task ID: " << task.taskId << "; File ID: " << task.fileId << "; Stub ID: " << task.stubId << "; Batch ID: " << task.batchId << "; Stage: " << static_cast<int>(task.stage) << "; Status: " << static_cast<int>(task.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the batches vector
    void printBatches() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "Batches: " << std::endl;
        for (const auto& batch : batches) {
            std::cout << "Batch ID: " << batch.batchId << "; Stub ID: " << batch.stubId << "; Stage: " << static_cast<int>(batch.stage) << "; Status: " << static_cast<int>(batch.status) << std::endl;
        }
        std::cout << std::endl;
    }

    // Print the contents of the TaskManager
    void print() {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::cout << "TaskManager: " << std::endl;
        std::cout << "Files: " << std::endl;
        for (const auto& file : files) {
            std::cout << "File ID: " << file.fileId << "; File Name: " << file.fileName << "; Stage: " << static_cast<int>(file.stage) << "; Status: " << static_cast<int>(file.status) << std::endl;
        }
        std::cout << "Tasks: " << std::endl;
        for (const auto& task : tasks) {
            std::cout << "Task ID: " << task.taskId << "; File ID: " << task.fileId << "; Stub ID: " << task.stubId << "; Batch ID: " << task.batchId << "; Stage: " << static_cast<int>(task.stage) << "; Status: " << static_cast<int>(task.status) << std::endl;
        }
        std::cout << "Batches: " << std::endl;
        for (const auto& batch : batches) {
            std::string taskIdsStr;
            for (const auto& taskId : batch.taskIds) {
                taskIdsStr += std::to_string(taskId) + " ";
            }
            std::cout << "Batch ID: " << batch.batchId << "; Stub ID: " << batch.stubId << "; Stage: " << static_cast<int>(batch.stage) << "; Status: " << static_cast<int>(batch.status) << "; TaskIds: " << taskIdsStr << std::endl;
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
                            } else if (status == "Complete") {
                                this->taskManager->setBatchStatus(batchId, ProcessingStatus::Complete);
                            } else if (status == "Error") {
                                this->taskManager->setBatchStatus(batchId, ProcessingStatus::Error);
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
                    stubConnection->receivePTreeQueue.queueMutex.unlock();
                }
            }
        }
    };
};
#endif  // CONTROLLER_HPP
