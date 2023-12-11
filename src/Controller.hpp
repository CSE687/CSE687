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
    bool hasMessage;
    std::mutex& cout_mutex;

    // thread-safe console writing
    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->cout_mutex);
        std::string messagePrefix = "Stub ID: " + std::to_string(this->stub_id) + "; CircularPropertyTreeBuffer: ";
        outputStream << message;
        outputStream.flush();
    };

   public:
    CircularPropertyTreeBuffer(std::mutex& cout_mutex, int stub_id) : currentIndex(0), hasMessage(false), cout_mutex(cout_mutex), stub_id(stub_id) {}

    void push(const boost::property_tree::ptree& message) {
        buffer.push_back(message);
        currentIndex = buffer.size() - 1;
        hasMessage = true;
    }

    boost::property_tree::ptree pop() {
        if (!hasMessage) {
            this->writeConsole(std::cerr, "Warning: Tried to retrieve message from CircularBuffer but no message exists\n");
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
        unresponsiveHeartbeatsSent = 0;
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
    bool destructionRequested;

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;

    // circular buffer for property_tree
    CircularPropertyTreeBuffer receiveBuffer;
    CircularPropertyTreeBuffer sendBuffer;

    std::mutex& cout_mutex;

    // thread-safe console writing
    void writeConsole(std::ostream& outputStream, const std::string& message) {
        std::lock_guard<std::mutex> lock(this->cout_mutex);
        std::string messagePrefix = "Stub ID: " + std::to_string(this->stub_id) + "; StubConnection: ";
        outputStream << message;
        outputStream.flush();
    };

    // receive message from stub
    void receiveMessage() {
        boost::asio::streambuf buf;
        boost::system::error_code error;

        boost::asio::async_read_until(socket, buf, "\n", [&](const boost::system::error_code& error, size_t bytes_transferred) {
            if (error == boost::asio::error::eof) {
                this->writeConsole(std::cout, "Connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + " closed cleanly by peer\n");
                return;
            } else if (error) {
                throw boost::system::system_error(error);  // Some other error.
            }

            std::istream is(&buf);
            std::string message;
            std::getline(is, message);

            // parse message into ptree
            std::stringstream ss;
            ss << message;
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(ss, pt);

            // place ptree in receiveBuffer, unless message_type = "heartbeat" or "ack"
            if (pt.get<std::string>("message_type") != "heartbeat" && pt.get<std::string>("message_type") != "ack") {
                this->receiveBuffer.push(pt);
                // print ptree
                this->writeConsole(std::cout, "Placed message from Stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + " in receiveBuffer: ");
                boost::property_tree::write_json(std::cout, pt);
            } else {
                this->writeConsole(std::cout, "Received message from Stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + ": ");
                boost::property_tree::write_json(std::cout, pt);
            }

            // Update the last message received time
            this->heartbeat.setTimeLastMessageReceived(std::chrono::system_clock::now());
        });
    }

    // send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
    void sendHeartbeat() {
        this->writeConsole(std::cout, "Sending heartbeat to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");
        // Create a ptree object
        boost::property_tree::ptree pt;
        pt.put("message_type", "heartbeat");
        pt.put("message", "heartbeat");

        this->sendMessage(pt);
    };

   public:
    StubConnection(int stub_id,
                   int port,
                   std::mutex& cout_mutex,
                   int heartbeatCadenceSeconds = 0) : stub_id(stub_id),
                                                      port(port),
                                                      isAlive(false),
                                                      destructionRequested(false),
                                                      socket(io_context),
                                                      cout_mutex(cout_mutex),
                                                      receiveBuffer(cout_mutex, stub_id),
                                                      sendBuffer(cout_mutex, stub_id),
                                                      heartbeat(heartbeatCadenceSeconds) {
    }

    void start() {
        // Place the connection logic in a thread
        std::thread connectionThread([this]() {
            this->writeConsole(std::cout, "Start connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");
            // Connect to the port and keep the connection alive
            while (true) {
                if (!this->isAlive) {
                    // Attempt to reconnect to the stub
                    this->writeConsole(std::cout, "Attempting to establish connection to stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + "\n");
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
                if (this->heartbeat.shouldSendHeartbeat()) {
                    this->sendHeartbeat();
                    if (this->heartbeat.getUnresponsiveHeartbeatsSent() > 3) {
                        this->isAlive = false;
                        this->writeConsole(std::cout, "Stub " + std::to_string(this->stub_id) + " on port " + std::to_string(this->port) + " is " + std::to_string(this->heartbeat.getUnresponsiveHeartbeatsSent()) + "x unresponsive\n");
                    }
                }
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
        jsonString += "\n";

        // Send the JSON string to the stub
        boost::asio::write(socket, boost::asio::buffer(jsonString));

        // Update the last message sent time
        this->heartbeat.setTimeLastMessageSent(std::chrono::system_clock::now());
    }

   private:
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
