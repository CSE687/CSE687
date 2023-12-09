#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "FileManager.hpp"

// cout mutex
// std::mutex cout_mutex_controller;

// Struct tracks the last heartbeat sent and received from a stub
struct StubHeartbeat {
    std::chrono::system_clock::time_point lastMessageSent;
    std::chrono::system_clock::time_point lastMessageReceived;
};

class StubConnection {
   private:
    int stub_id;
    int port;
    StubHeartbeat heartbeat;
    bool isAlive;
    bool destructionRequested;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

   public:
    StubConnection(int stub_id, int port) : stub_id(stub_id), port(port), isAlive(false), destructionRequested(false), socket(io_context) {}

    void start(std::string input_directory, std::string output_directory, std::string temp_directory) {
        std::cout << "Start connection to stub " << this->stub_id << " on port " << this->port << std::endl;
        // Place the connection logic in a thread
        std::thread connectionThread([this, input_directory, output_directory, temp_directory]() {
            // Connect to the port and keep the connection alive
            while (true) {
                if (!this->isAlive) {
                    // Attempt to reconnect to the stub
                    std::cout << "Attempting to establish connection to stub " << this->stub_id << " on port " << this->port << std::endl;
                    establishConnection(input_directory, output_directory, temp_directory);
                }
                // Sleep for a certain duration before attempting to reconnect
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });

        // Detach the thread to allow it to run independently
        connectionThread.detach();
    }

   private:
    void establishConnection(std::string input_directory, std::string output_directory, std::string temp_directory) {
        // Attempt to connect to the stub
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), this->port);
        boost::system::error_code error;

        socket.connect(endpoint, error);

        if (!error) {
            isAlive = true;
            // cout_mutex_controller.lock();
            std::cout << "Established connection to stub " << this->stub_id << " on port " << this->port << std::endl;
            std::cout << "Sending establishment message to stub " << this->stub_id << " on port " << this->port << std::endl;
            // cout_mutex_controller.unlock();

            // send directory locations for threadmanager calls from stubs
            std::string message = "FileManager:" + input_directory + "," + output_directory + "," + temp_directory;
            boost::asio::write(socket, boost::asio::buffer(message));

            for (int i = 0; i < 5; i++) {
                sleep(5);
                std::string message = "Sending heartbeat";
                boost::asio::write(socket, boost::asio::buffer(message));
            }

            // Create a ptree object
            // boost::property_tree::ptree pt;
            // pt.put("message", "hello");

            // // Convert the ptree to a JSON string
            // std::ostringstream buf;
            // boost::property_tree::write_json(buf, pt, false);
            // std::string jsonString = buf.str();

            // // Send the JSON string to the stub
            // boost::asio::write(socket, boost::asio::buffer(jsonString));

            // Write a message othe socket
            // std::string message = "Hello from controller";
            // boost::asio::write(socket, boost::asio::buffer(message));
        } else {
            isAlive = false;
            // cout_mutex_controller.lock();
            std::cout << "Failed to establish connection to stub " << this->stub_id << " on port " << this->port << std::endl;
            // cout_mutex_controller.unlock();
        }
    }
};

// Enum for stages of processing
enum class ProcessingStage : int {
    Discovered,
    Mapping,
    Mapped,
    Reducing,
    Reduced
};

// Struct tracks the files that have been processed by the MapReduce process
struct ProcessedFile {
    std::string filePath;
    ProcessingStage stage;
};

// Struct tracks in progress tasks, the stub the task is assigned to, and the file being processed
struct Task {
    int taskId;
    std::string fileName;
    ProcessingStage stage;
};

// Controller class is responsible for managing the entire MapReduce process
class Controller {
   private:
    FileManager* fileManager;
    int port;
    std::vector<std::shared_ptr<StubConnection>> stubConnections;

   public:
    Controller(FileManager* fileManager, int port, std::vector<int>& ports) : fileManager(fileManager), port(port) {
        // Create socket connections to stubs
        for (int i = 0; i < ports.size(); i++) {
            this->createStubConnection(i, ports[i]);
        }
    }

    // Use fileManager to read files from directory
    void readFiles(const std::string& directory) {
    }

    // Tell stub process to run either Map or Reduce process on specific file
    void runProcess(const std::string& processType, const std::string& fileName) {
    }

    // Create socket connection to Stub process which is listening on user-specified port
    void createStubConnection(int stub_id, int port) {
        // Create a new StubConnection object and add it to the stubConnections vector
        std::cout << "Creating connection to stub " << stub_id << " on port " << port << std::endl;
        stubConnections.push_back(std::make_shared<StubConnection>(stub_id, port));
        stubConnections.back()->start(this->fileManager->getInputDirectory(), this->fileManager->getOutputDirectory(), this->fileManager->getTempDirectory());
    }

    void sendHeartbeat(int port) {
        // Send heartbeat message to the stub connection
        // Update stubsLastHeartbeatSent
    }

    void receiveHeartbeat(int port) {
        // Update stubsLastHeartbeatReceived when a heartbeat message is received from a stub
    }

    void execute() {
        // Read files from input directory
        // Create socket connections to stubs
        // Track stub connections
        // Send heartbeat messages to stubs
        // Receive heartbeat messages from stubs
        // Run Map process on files
        // Run Reduce process on files
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};

#endif  // CONTROLLER_HPP
