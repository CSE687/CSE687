/////////////////////////////////////////////////////////////////////
//  Stub.hpp           -   header file for Stub class              //
//  Language           -   C++                                     //
//  Author             -   Chandler Johns                          //
/////////////////////////////////////////////////////////////////////

/*
The Stub class contains all the functionality to receive message
from a controller server to run map or reduce task threads.
*/

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

// Task struct to lock threads and hold task id being completed
struct stubTask {
    boost::mutex process_lock;
    boost::thread* process_thread;
    int task_id;
};

class Stub {
   private:
    int port_num;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    stubTask stubProcess;

    boost::system::error_code error;
    void listen();                                                                                                 // method that constantly listens for messages from controller
    void startMapThreads(std::vector<std::string> input_files);                                                    // stars the mapper threads
    void startReduceThreads(std::vector<std::string> input_files);                                                 // starts the reducer threads
    void setupFileManager(std::string input_directory, std::string output_directory, std::string temp_directory);  // sets up the file manager instance for stub client
    void receiveMessage(std::string message);                                                                      // receives and parses the message type from controller
    void sendMessage(const boost::property_tree::ptree message);                                                   // sends message back to controller
    void performTask(const boost::property_tree::ptree message);                                                   // performs task requested by controller

   public:
    FileManager* filemanager;  // filemanager instance
    Stub(int port_num);
    ~Stub();
    void operator()();  // functor to allow class to be run using class declaration
};

#endif  // STUB_HPP
