
#ifndef STUBHEARTBEAT_HPP
#define STUBHEARTBEAT_HPP

#include <chrono>
#include <mutex>

class StubHeartbeat {
   private:
    std::chrono::system_clock::time_point timeLastMessageSent;
    std::chrono::system_clock::time_point timeLastMessageReceived;
    std::chrono::system_clock::time_point timeLastHeartbeatSent;
    std::chrono::duration<double> heartbeatCadenceSeconds;

    int unresponsiveHeartbeatsSent;

    std::chrono::duration<double> timeSinceLastMessageSent();
    std::chrono::duration<double> timeSinceLastMessageReceived();
    std::chrono::duration<double> timeSinceLastHeartbeatSent();

    void setTimeLastMessageSent(const std::chrono::system_clock::time_point& time);
    void setTimeLastMessageReceived(const std::chrono::system_clock::time_point& time);
    void setTimeLastHeartbeatSent(const std::chrono::system_clock::time_point& time);
    void resetUnresponsiveHeartbeatsSent();

   public:
    std::mutex mutex;

    StubHeartbeat(int heartbeatCadenceSeconds);

    std::chrono::system_clock::time_point getTimeLastMessageSent() const;
    std::chrono::system_clock::time_point getTimeLastMessageReceived() const;
    std::chrono::system_clock::time_point getTimeLastHeartbeatSent() const;
    std::chrono::duration<double> getHeartbeatCadenceSeconds() const;
    int getUnresponsiveHeartbeatsSent() const;

    bool shouldSendHeartbeat();
    void incrementHeartbeatsSent();
    void messageReceived();
    void messageSent();
};

#endif  // STUBHEARTBEAT_HPP
