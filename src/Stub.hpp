
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
    int client_socket, status;
    FileManager* file_manager;
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    void startListening();

   public:
    Stub(int port_num);
    ~Stub();
    void operator()();
    int fail_status();
};
#endif  // STUB_HPP
