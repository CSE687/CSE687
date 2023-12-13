
#ifndef STUB_HPP
#define STUB_HPP

#include <stdio.h>
#include <string.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/thread.hpp>
#include <iostream>

#include "FileManager.hpp"

class Stub {
   private:
    // void error(const char* msg);
    int port_num;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    boost::mutex process_running;
    boost::thread* process_thread;

    boost::system::error_code error;
    void listen();
    void startMapThreads(std::vector<std::string> input_files);
    void startReduceThreads(std::vector<std::string> input_files);
    void setupFileManager(std::string input_directory, std::string output_directory, std::string temp_directory);
    void receiveMessage(std::string message);
    void sendMessage(const boost::property_tree::ptree message);
    void performTask(const boost::property_tree::ptree message);

   public:
    FileManager* filemanager;
    Stub(int port_num);
    ~Stub();
    void operator()();
};

#endif  // STUB_HPP
