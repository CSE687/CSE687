#include "Controller.hpp"

// thread-safe console writing
void PropertyTreeQueue::writeConsole(std::ostream& outputStream, const std::string& message) {
    std::lock_guard<std::mutex> lock(this->coutMutex);
    std::string messageWithPrefix = "Stub ID: " + std::to_string(this->stub_id) + "; PropertyTreeQueue: ";
    outputStream << messageWithPrefix << message;
    outputStream.flush();
}

// push a message to the back of the queue
void PropertyTreeQueue::push(const boost::property_tree::ptree& message) {
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
boost::property_tree::ptree PropertyTreeQueue::pop() {
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
bool PropertyTreeQueue::hasMessage() const {
    return !this->queue.empty();
}

// remove all messages from the queue
void PropertyTreeQueue::clear() {
    while (!this->queue.empty()) {
        this->queue.pop();
    }
}
