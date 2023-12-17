#include "StubHeartbeat.hpp"

// calculate the time since the last heartbeat was sent
std::chrono::duration<double> StubHeartbeat::timeSinceLastMessageSent() {
    return std::chrono::system_clock::now() - this->timeLastMessageSent;
}

// calculate the time since the last heartbeat was received
std::chrono::duration<double> StubHeartbeat::timeSinceLastMessageReceived() {
    return std::chrono::system_clock::now() - this->timeLastMessageReceived;
}

// calculate the time since the last heartbeat was sent
std::chrono::duration<double> StubHeartbeat::timeSinceLastHeartbeatSent() {
    return std::chrono::system_clock::now() - this->timeLastHeartbeatSent;
}

void StubHeartbeat::setTimeLastMessageSent(const std::chrono::system_clock::time_point& time) {
    timeLastMessageSent = time;
}

void StubHeartbeat::setTimeLastMessageReceived(const std::chrono::system_clock::time_point& time) {
    timeLastMessageReceived = time;
    this->resetUnresponsiveHeartbeatsSent();
}

void StubHeartbeat::setTimeLastHeartbeatSent(const std::chrono::system_clock::time_point& time) {
    timeLastHeartbeatSent = time;
}

void StubHeartbeat::resetUnresponsiveHeartbeatsSent() {
    unresponsiveHeartbeatsSent = 0;
}

StubHeartbeat::StubHeartbeat(int heartbeatCadenceSeconds) : unresponsiveHeartbeatsSent(0), heartbeatCadenceSeconds(std::chrono::seconds(heartbeatCadenceSeconds)) {
    // Initialize time points to the current time
    timeLastMessageSent = std::chrono::system_clock::now();
    timeLastMessageReceived = std::chrono::system_clock::now();
    timeLastHeartbeatSent = std::chrono::system_clock::now();
}

std::chrono::system_clock::time_point StubHeartbeat::getTimeLastMessageSent() const {
    return timeLastMessageSent;
}

std::chrono::system_clock::time_point StubHeartbeat::getTimeLastMessageReceived() const {
    return timeLastMessageReceived;
}

std::chrono::system_clock::time_point StubHeartbeat::getTimeLastHeartbeatSent() const {
    return timeLastHeartbeatSent;
}

std::chrono::duration<double> StubHeartbeat::getHeartbeatCadenceSeconds() const {
    return heartbeatCadenceSeconds;
}

int StubHeartbeat::getUnresponsiveHeartbeatsSent() const {
    return unresponsiveHeartbeatsSent;
}

// determine if a heartbeat should be sent
bool StubHeartbeat::shouldSendHeartbeat() {
    // If the time since the last message sent exceeds the heartbeat cadence, send a heartbeat
    if (this->timeSinceLastMessageSent() > this->heartbeatCadenceSeconds && this->timeSinceLastHeartbeatSent() > this->heartbeatCadenceSeconds) {
        return true;
    }
    return false;
}

void StubHeartbeat::incrementHeartbeatsSent() {
    unresponsiveHeartbeatsSent++;
}

// notify StubHeartBeat that a message has been received
void StubHeartbeat::messageReceived() {
    this->setTimeLastMessageReceived(std::chrono::system_clock::now());
}

// notify StubHeartBeat that a message has been sent
void StubHeartbeat::messageSent() {
    this->setTimeLastMessageSent(std::chrono::system_clock::now());
}
