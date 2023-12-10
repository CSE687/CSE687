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
    std::vector<boost::property_tree::ptree> buffer;
    size_t currentIndex;
    bool hasMessage;
    std::mutex& cout_mutex;

   public:
    CircularPropertyTreeBuffer(std::mutex& cout_mutex) : currentIndex(0), hasMessage(false), cout_mutex(cout_mutex) {}

    void push(const boost::property_tree::ptree& message) {
        buffer.push_back(message);
        currentIndex = buffer.size() - 1;
        hasMessage = true;
    }

    boost::property_tree::ptree pop() {
        if (!hasMessage) {
            cout_mutex.lock();
            std::cerr << "Warning: Tried to retrieve message from CircularBuffer but no message exists" << std::endl;
            cout_mutex.unlock();
        }
        boost::property_tree::ptree message = buffer[currentIndex];
        buffer.erase(buffer.begin() + currentIndex);
        if (buffer.empty()) {
            hasMessage = false;
        } else {
            currentIndex = currentIndex % buffer.size();
        }
        return message;
    }

    bool getHasMessage() const {
        return hasMessage;
    }
};

// Struct tracks the last heartbeat sent and received from a stub
struct StubHeartbeat {
    std::chrono::system_clock::time_point lastMessageSent;
    std::chrono::system_clock::time_point lastMessageReceived;

    // Track allowed time between heartbeats
    std::chrono::duration<double> secondsBetweenHeartbeats;
};

class StubConnection {
   private:
    int stub_id;
    int port;

    // Heartbeat struct to track the last heartbeat sent and received from the stub
    StubHeartbeat heartbeat;
    bool isAlive;
    bool destructionRequested;

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    // circular buffer for property_tree
    CircularPropertyTreeBuffer receiveBuffer;
    CircularPropertyTreeBuffer sendBuffer;

    std::mutex& cout_mutex;

    // receive message from stub
    void receiveMessage() {
        boost::array<char, 128> buf;
        boost::system::error_code error;

        size_t len = socket.read_some(boost::asio::buffer(buf), error);

        if (error == boost::asio::error::eof) {
            // this->isAlive = false;  // Connection closed cleanly by peer.
            // this->socket.close();
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Connection to stub " << this->stub_id << " on port " << this->port << " closed cleanly by peer" << std::endl;
            return;
        } else if (error) {
            throw boost::system::system_error(error);  // Some other error.
        }

        std::string message(buf.data(), len);
        // parse message into ptree
        std::stringstream ss;
        ss << message;
        boost::property_tree::ptree pt;
        boost::property_tree::read_json(ss, pt);
        // place ptree in receiveBuffer, unless message_type = "heartbeat" or "ack"
        if (pt.get<std::string>("message_type") != "heartbeat" && pt.get<std::string>("message_type") != "ack") {
            this->receiveBuffer.push(pt);
            // print ptree
            std::cout << "Placed message from Stub " << this->stub_id << "on port in receiveBuffer" << this->port << ": ";
            boost::property_tree::write_json(std::cout, pt);
        } else {
            std::cout << "Received message from Stub " << this->stub_id << " on port " << this->port << ": ";
            boost::property_tree::write_json(std::cout, pt);
        }

        // Update the last message received time
        this->heartbeat.lastMessageReceived = std::chrono::system_clock::now();
    }

    // send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
    void sendHeartbeat() {
        auto _timeSinceLastHeartbeatReceived = this->timeSinceLastHeartbeatReceived();
        cout_mutex.lock();
        std::cout << "Time since last heartbeat sent: " << _timeSinceLastHeartbeatReceived.count() << std::endl;
        std::cout << "Allowed time between heartbeats: " << this->heartbeat.secondsBetweenHeartbeats.count() << std::endl;
        cout_mutex.unlock();
        if (_timeSinceLastHeartbeatReceived > this->heartbeat.secondsBetweenHeartbeats) {
            cout_mutex.lock();
            std::cout << "Sending heartbeat to stub " << this->stub_id << " on port " << this->port << std::endl;
            cout_mutex.unlock();
            // Create a ptree object
            boost::property_tree::ptree pt;
            pt.put("message_type", "heartbeat");
            pt.put("message", "heartbeat");

            this->sendMessage(pt);
        }
    }

   public:
    StubConnection(int stub_id,
                   int port,
                   std::mutex& cout_mutex,
                   int secondsBetweenHeartbeats) : stub_id(stub_id),
                                                   port(port),
                                                   isAlive(false),
                                                   destructionRequested(false),
                                                   socket(io_context),
                                                   cout_mutex(cout_mutex),
                                                   receiveBuffer(cout_mutex),
                                                   sendBuffer(cout_mutex) {
        // Initialize StubHeartbeat struct
        this->heartbeat.lastMessageSent = std::chrono::system_clock::now();
        this->heartbeat.lastMessageReceived = std::chrono::system_clock::now();
        // Set the allowed time between heartbeats to 5 seconds
        this->heartbeat.secondsBetweenHeartbeats = std::chrono::seconds(secondsBetweenHeartbeats);
    }

    void start() {
        std::cout << "Start connection to stub " << this->stub_id << " on port " << this->port << std::endl;
        // Place the connection logic in a thread
        std::thread connectionThread([this]() {
            // Connect to the port and keep the connection alive
            while (true) {
                if (!this->isAlive) {
                    // Attempt to reconnect to the stub
                    this->cout_mutex.lock();
                    std::cout << "Attempting to establish connection to stub " << this->stub_id << " on port " << this->port << std::endl;
                    this->cout_mutex.unlock();
                    establishConnection();

                    if (!this->isAlive) {
                        // Sleep for a certain duration before attempting to reconnect
                        std::this_thread::sleep_for(std::chrono::seconds(1));

                        // The rest of the loop is comms - if the connection is not alive, we don't want to continue
                        continue;
                    }
                }

                // Load the receiveBuffer with messages from the stub
                this->receiveMessage();

                // Send messages from the sendBuffer to the stub
                if (this->sendBuffer.getHasMessage()) {
                    boost::property_tree::ptree message = this->sendBuffer.pop();
                    this->sendMessage(message);
                }

                // Send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
                this->sendHeartbeat();
            }
        });

        // Detach the thread to allow it to run independently
        connectionThread.detach();
    };

    // Send a message to the stub
    void sendMessage(const boost::property_tree::ptree pt) {
        // Convert the ptree to a JSON string
        std::ostringstream buf;
        boost::property_tree::write_json(buf, pt, false);
        std::string jsonString = buf.str();

        // Send the JSON string to the stub
        boost::asio::write(socket, boost::asio::buffer(jsonString));

        // Update the last message sent time
        this->heartbeat.lastMessageSent = std::chrono::system_clock::now();
    }

   private:
    // calculate the time since the last heartbeat was sent
    std::chrono::duration<double> timeSinceLastHeartbeatSent() {
        return std::chrono::system_clock::now() - this->heartbeat.lastMessageSent;
    }

    // calculate the time since the last heartbeat was received
    std::chrono::duration<double> timeSinceLastHeartbeatReceived() {
        return std::chrono::system_clock::now() - this->heartbeat.lastMessageReceived;
    }

    // Establish a connection to the stub
    void establishConnection() {
        // Attempt to connect to the stub
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), this->port);
        boost::system::error_code error;

        socket.connect(endpoint, error);

        if (!error) {
            cout_mutex.lock();
            std::cout << "Established connection to stub " << this->stub_id << " on port " << this->port << std::endl;
            std::cout << "Sending establishment message to stub " << this->stub_id << " on port " << this->port << std::endl;
            cout_mutex.unlock();

            // Create a ptree object
            boost::property_tree::ptree pt;
            pt.put("message_type", "establish_connection");
            pt.put("message", "Establish connection to stub");

            this->sendMessage(pt);
            isAlive = true;
        } else {
            isAlive = false;
            cout_mutex.lock();
            std::cout << "Failed to establish connection to stub " << this->stub_id << " on port " << this->port << std::endl;
            std::cout << "Error: " << error.message() << std::endl;
            cout_mutex.unlock();
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
    std::mutex cout_mutex;

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
        stubConnections.push_back(std::make_shared<StubConnection>(stub_id, port, this->cout_mutex, 5));
        stubConnections.back()->start();
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
