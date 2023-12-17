#ifndef STUBCONNECTION_HPP
#define STUBCONNECTION_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "PropertyTreeQueue.hpp"
#include "StubHeartbeat.hpp"

class StubConnection {
   public:
    StubConnection(int stub_id,
                   int port,
                   std::string input_directory,
                   std::string output_directory,
                   std::string temp_directory,
                   std::mutex& coutMutex,
                   int heartbeatCadenceSeconds = 5);

    void start();

    int getStubId() const;

    int getPort() const;

    const StubHeartbeat& getHeartbeat() const;

    bool getIsAlive() const;

    PropertyTreeQueue receivePTreeQueue;
    PropertyTreeQueue sendPTreeQueue;

   private:
    void connect();

    void receiveMessages();

    void sendMessages();

    void writeConsole(std::ostream& outputStream, const std::string& message);

    void sendHeartbeat();

    void sendMessage(const boost::property_tree::ptree pt);

    void setIsAliveFalse();

    int stub_id;
    int port;
    std::string input_directory;
    std::string output_directory;
    std::string temp_directory;
    bool isAlive;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::socket socket;
    std::mutex& coutMutex;
    StubHeartbeat heartbeat;
    std::thread connectThread;
    std::thread receiveThread;
    std::thread sendThread;
};

#endif  // STUBCONNECTION_HPP
