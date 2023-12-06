
#ifndef STUB_H
#define STUB_H

#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "FileManager.hpp"

class Stub {
   private:
    void error(const char* msg);
    int port_num;
    int client_socket, status;
    char buffer[1024] = {0};
    struct sockaddr_in serv_addr;
    FileManager* file_manager;

   public:
    Stub(int port_num);
    ~Stub();
    void operator()();
    int check_status();
};
#endif  // STUB_H
