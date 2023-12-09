
#ifndef STUB_HPP
#define STUB_HPP

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include "FileManager.hpp"

class Stub {
   private:
    // void error(const char* msg);
    int port_num;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;

    boost::system::error_code error;
    boost::array<char, 128> buf;
    void startThreads(std::string message);
    void setupFileManager(std::string message);

   public:
    FileManager* filemanager;
    Stub(int port_num);
    ~Stub();
    void operator()();
};
#endif  // STUB_HPP
