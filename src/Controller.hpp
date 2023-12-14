#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "FileManager.hpp"

class CircularPropertyTreeBuffer {
   private:
    int stub_id;
    std::vector<boost::property_tree::ptree> buffer;
    size_t currentIndex;
    bool _hasMessage;
    std::mutex& coutMutex;

    // thread-safe console writing
    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->coutMutex);
        std::string messageWithPrefix = "Stub ID: " + std::to_string(this->stub_id) + "; CircularPropertyTreeBuffer: ";
        outputStream << messageWithPrefix << message;
        outputStream.flush();
    };

   public:
    std::mutex bufferMutex;

    CircularPropertyTreeBuffer(std::mutex& coutMutex, int stub_id) : currentIndex(0), _hasMessage(false), coutMutex(coutMutex), stub_id(stub_id) {}

    void push(const boost::property_tree::ptree& message) {
        this->writeConsole(std::cout, "Pushing message to CircularBuffer\n");
        buffer.push_back(message);
        currentIndex = buffer.size() - 1;
        _hasMessage = true;
    }

    boost::property_tree::ptree pop() {
        if (!_hasMessage) {
            this->writeConsole(std::cout, "Warning: Tried to retrieve message from CircularBuffer but no message exists\n");
        }

        this->writeConsole(std::cout, "Retrieving message from CircularBuffer\n");

        boost::property_tree::ptree message = buffer[currentIndex];
        buffer.erase(buffer.begin() + currentIndex);
        if (buffer.empty()) {
            _hasMessage = false;
        } else {
            currentIndex = currentIndex % buffer.size();
        }
        return message;
    }

    bool hasMessage() const {
        return _hasMessage;
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

    // circular buffer for property_tree
    CircularPropertyTreeBuffer receivePTreeBuffer;
    CircularPropertyTreeBuffer sendPTreeBuffer;

    std::thread connectThread;
    std::thread receiveThread;
    std::thread sendThread;

    std::mutex& coutMutex;

    std::string filemanager_input_directory;
    std::string filemanager_output_directory;
    std::string filemanager_temp_directory;

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

                    // Place the pt object in the sendPTreeBuffer
                    boost::property_tree::ptree pt;
                    pt.put("message_type", "establish_connection");
                    pt.put("message", "Establish connection to stub");
                    pt.put("input_directory", this->filemanager_input_directory);
                    pt.put("output_directory", this->filemanager_output_directory);
                    pt.put("temp_directory", this->filemanager_temp_directory);
                    this->sendPTreeBuffer.bufferMutex.lock();
                    this->sendPTreeBuffer.push(pt);
                    this->sendPTreeBuffer.bufferMutex.unlock();
                    this->isAlive = true;
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

    // Function to continuously listen for messages on the socket and append them to receivePTreeBuffer
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
                    this->isAlive = false;
                    break;  // Connection closed cleanly by peer.
                } else if (error) {
                    this->writeConsole(std::cout, "Error: " + error.message() + "\n");
                    this->isAlive = false;
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

                // Append the received PropertyTree object to receivePTreeBuffer
                this->heartbeat.setTimeLastMessageReceived(std::chrono::system_clock::now());
                this->receivePTreeBuffer.bufferMutex.lock();
                receivePTreeBuffer.push(receivedPTree);
                this->receivePTreeBuffer.bufferMutex.unlock();
            }
            // Sleep for 1s if the connection is not alive
            this->writeConsole(std::cout, "!isAlive: receiveMessages sleeping\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        this->writeConsole(std::cout, "receiveThread ending\n");
    }

    // Function to continuously monitor sendPTreeBuffer and send PropertyTree objects on the socket
    void sendMessages() {
        this->writeConsole(std::cout, "sendThread started\n");
        while (true) {
            while (this->isAlive) {
                // Sleep for 100ms
                // this->writeConsole(std::cout, "sendMessages sleeping\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // Check if there are PropertyTree objects in sendPTreeBuffer
                this->sendPTreeBuffer.bufferMutex.lock();
                if (sendPTreeBuffer.hasMessage()) {
                    this->writeConsole(std::cout, "Sending message to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");

                    // Pop the PropertyTree object from sendPTreeBuffer
                    boost::property_tree::ptree pt = sendPTreeBuffer.pop();

                    // Convert the PropertyTree object to a JSON string
                    std::ostringstream buf;
                    boost::property_tree::write_json(buf, pt, false);
                    std::string jsonString = buf.str();

                    // Send the JSON string to the stub
                    try {
                        this->sendMessage(pt);
                    } catch (const boost::system::system_error& e) {
                        this->writeConsole(std::cerr, "Error sending message: " + std::string(e.what()) + "\n");
                        this->isAlive = false;
                    }
                }
                this->sendPTreeBuffer.bufferMutex.unlock();
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
        this->writeConsole(std::cout, "Writing heartbeat to Stub " + std::to_string(this->stub_id) + " sendPTreeBuffer. Unresponsive Count: " + std::to_string(this->heartbeat.getUnresponsiveHeartbeatsSent()) + "\n");
        std::lock_guard<std::mutex> lock(this->sendPTreeBuffer.bufferMutex);
        this->sendPTreeBuffer.push(pt);
    };

    // Send a message to the stub
    void sendMessage(const boost::property_tree::ptree pt) {
        // Convert the ptree to a JSON string
        std::ostringstream buf;
        boost::property_tree::write_json(buf, pt, false);
        std::string jsonString = buf.str();
        // jsonString += "\n";

        // Send the JSON string to the stub
        boost::asio::write(this->socket, boost::asio::buffer(jsonString));

        // Update the last message sent time
        this->heartbeat.setTimeLastMessageSent(std::chrono::system_clock::now());
    }

   public:
    StubConnection(int stub_id,
                   int port,
                   std::mutex& coutMutex,
                   std::string input_directory,
                   std::string output_directory,
                   std::string temp_directory,
                   int heartbeatCadenceSeconds = 0) : stub_id(stub_id),
                                                      port(port),
                                                      isAlive(false),
                                                      socket(io_context),
                                                      coutMutex(coutMutex),
                                                      receivePTreeBuffer(coutMutex, stub_id),
                                                      sendPTreeBuffer(coutMutex, stub_id),
                                                      filemanager_input_directory(input_directory),
                                                      filemanager_output_directory(output_directory),
                                                      filemanager_temp_directory(temp_directory),
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
};

// Enum for stages of processing
enum class ProcessingStage : int {
    Discovered,
    Map,
    Reduce,
};

// Enum for processing status
enum class ProcessingStatus : int {
    NotStarted,
    InProgress,
    Complete,
};

// Struct tracks the files that have been processed by the MapReduce process
struct ProcessFile {
    int fileId;
    std::string fileName;
    ProcessingStage stage;
};

// Struct tracks in progress tasks, the stub the task is assigned to, and the file being processed
struct Task {
    int taskId;
    std::string fileName;
    ProcessingStage stage;
    ProcessingStatus status;
};

// Struct tracks which tasks are assigned to which stubs
struct TaskAssignment {
    int taskId;
    int stubId;
};

// Controller class is responsible for managing the entire MapReduce process
class Controller {
   private:
    FileManager* fileManager;
    int port;
    std::vector<std::shared_ptr<StubConnection>> stubConnections;
    std::mutex coutMutex;

    std::vector<ProcessFile> processFiles;
    std::vector<Task> tasks;
    std::vector<TaskAssignment> taskAssignments;

   public:
    Controller(FileManager* fileManager, int port, std::vector<int>& ports) : fileManager(fileManager), port(port) {
        // Create socket connections to stubs
        for (int i = 0; i < ports.size(); i++) {
            this->createStubConnection(i, ports[i]);
        }

        // Build vector of ProcessFile
        for (const auto& fullPathFile : this->fileManager->getDirectoryFileList(fileManager->getInputDirectory())) {
            ProcessFile file;
            file.fileName = this->fileManager->getFileStem(fullPathFile);
            file.stage = ProcessingStage::Discovered;
            processFiles.push_back(file);
        }

        // Print the contents of the processFiles vector
        std::cout << "Processing Files:" << std::endl;
        for (const auto& file : processFiles) {
            std::cout << "File: " << file.fileName << "; Stage: " << static_cast<int>(file.stage) << std::endl;
        }

        // For each file in processFiles, build a Task for each stage of processing
        for (const auto& file : processFiles) {
            Task mapTask;
            mapTask.fileName = file.fileName;
            mapTask.stage = ProcessingStage::Map;
            tasks.push_back(mapTask);

            Task reduceTask;
            reduceTask.fileName = file.fileName;
            reduceTask.stage = ProcessingStage::Reduce;
            tasks.push_back(reduceTask);

            // taskId / fileName / stage (Enum: )
        }
    }

    // Create socket connection to Stub process which is listening on user-specified port
    void createStubConnection(int stub_id, int port) {
        // Create a new StubConnection object and add it to the stubConnections vector
        std::cout << "Creating connection to stub " << stub_id << " on port " << port << std::endl;
        stubConnections.push_back(std::make_shared<StubConnection>(stub_id, port, this->coutMutex, this->fileManager->getInputDirectory(), this->fileManager->getOutputDirectory(), this->fileManager->getTempDirectory(), 5));
        stubConnections.back()->start();
    }

    void execute() {
        // Run Map process on files
        // Run Reduce process on files
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

#endif  // CONTROLLER_HPP
