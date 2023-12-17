#include "StubConnection.hpp"

StubConnection::StubConnection(int stub_id,
                               int port,
                               std::string input_directory,
                               std::string output_directory,
                               std::string temp_directory,
                               std::mutex& coutMutex,
                               int heartbeatCadenceSeconds) : stub_id(stub_id),
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

void StubConnection::start() {
    // Create and start the receive and send threads
    connectThread = std::thread(&StubConnection::connect, this);
    connectThread.detach();
    receiveThread = std::thread(&StubConnection::receiveMessages, this);
    receiveThread.detach();
    sendThread = std::thread(&StubConnection::sendMessages, this);
    sendThread.detach();
};

// Getter for stub_id
int StubConnection::getStubId() const {
    return stub_id;
}

// Getter for port
int StubConnection::getPort() const {
    return port;
}

// Getter for heartbeat
const StubHeartbeat& StubConnection::getHeartbeat() const {
    return heartbeat;
}

// Getter for isAlive
bool StubConnection::getIsAlive() const {
    return isAlive;
}

void StubConnection::connect() {
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
void StubConnection::receiveMessages() {
    this->writeConsole(std::cout, "receiveThread started\n");
    while (true) {
        while (isAlive) {
            // Receive and convert the message to PropertyTree object
            boost::array<char, 496> buf;
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
            std::string buffer_message(buf.data(), len);
            std::vector<std::string> jsonStrings;

            // Split buffer_message into separate JSON strings
            size_t startPos = 0;
            size_t endPos = buffer_message.find('\n');
            while (endPos != std::string::npos) {
                std::string jsonString = buffer_message.substr(startPos, endPos - startPos);
                jsonStrings.push_back(jsonString);
                startPos = endPos + 1;
                endPos = buffer_message.find('\n', startPos);
            }

            // Process each JSON string separately
            for (const std::string& jsonString : jsonStrings) {
                boost::property_tree::ptree receivedPTree;
                std::stringstream stringStream;
                // Clear the stringStream
                stringStream.str("");
                stringStream.clear();

                stringStream << jsonString;

                try {
                    boost::property_tree::read_json(stringStream, receivedPTree);
                    this->heartbeat.messageReceived();
                } catch (const boost::property_tree::json_parser_error& e) {
                    // Handle the exception (e.g., print error message)
                    this->writeConsole(std::cerr, "Failed to read JSON: " + std::string(e.what()) + "\n");
                    this->writeConsole(std::cerr, "Received message: " + jsonString + "\n");
                    continue;
                }

                // Safely read the message_type key from the received PropertyTree object
                try {
                    std::string message_type = receivedPTree.get<std::string>("message_type");
                    // If the message type is "ack", continue
                    if (message_type == "ack") {
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
            // this->writeConsole(std::cout, "receiveMessages sleeping\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // Sleep for 1s if the connection is not alive
        this->writeConsole(std::cout, "!isAlive: receiveMessages sleeping\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    this->writeConsole(std::cout, "receiveThread ending\n");
}

// Function to continuously monitor sendPTreeQueue and send PropertyTree objects on the socket
void StubConnection::sendMessages() {
    this->writeConsole(std::cout, "sendThread started\n");
    while (true) {
        while (this->isAlive) {
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
            // Sleep for 100ms
            // this->writeConsole(std::cout, "sendMessages sleeping\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        // Sleep for 1s if the connection is not alive
        this->writeConsole(std::cout, "!isAlive: sendMessages sleeping\n");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    this->writeConsole(std::cout, "sendThread ending\n");
};

// thread-safe console writing
void StubConnection::writeConsole(std::ostream& outputStream, const std::string& message) {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::string messageWithPrefix = "Stub ID: " + std::to_string(this->stub_id) + "; StubConnection: ";
    outputStream << messageWithPrefix << message;
    outputStream.flush();
};

// send a heartbeat if the time since the last heartbeat sent exceeds the allowed time between heartbeats
void StubConnection::sendHeartbeat() {
    // Create a ptree object
    boost::property_tree::ptree pt;
    pt.put("message_type", "heartbeat");
    pt.put("message", "heartbeat");

    // Lock heartbeat
    this->writeConsole(std::cout, "Writing heartbeat to Stub " + std::to_string(this->stub_id) + " sendPTreeQueue. Unresponsive Count: " + std::to_string(this->heartbeat.getUnresponsiveHeartbeatsSent()) + "\n");
    this->heartbeat.mutex.lock();
    this->heartbeat.incrementHeartbeatsSent();
    this->heartbeat.mutex.unlock();
    std::lock_guard<std::mutex> lock(this->sendPTreeQueue.queueMutex);
    this->sendPTreeQueue.push(pt);
};

// Send a message to the stub
void StubConnection::sendMessage(const boost::property_tree::ptree pt) {
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
    this->heartbeat.messageSent();
}

// set isAlive to false
void StubConnection::setIsAliveFalse() {
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
