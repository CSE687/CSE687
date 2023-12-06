
#include "Stub.hpp"

#include "ThreadManager.hpp"

void Stub::error(const char* msg) {
    perror(msg);
    exit(0);
}

Stub::Stub(int port) {
    this->file_manager = FileManager::GetInstance();
    this->port_num = port;
    this->client_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (this->client_socket < 0) {
        printf("\n Socket creation error \n");
        this->status = -1;
        return;
    }
    this->serv_addr.sin_family = AF_LOCAL;
    this->serv_addr.sin_port = htons(port_num);
    this->status = connect(this->client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (this->status < 0) {
        printf("\nConnection Failed\n");
    } else {
        printf("\nConnected to server\n");
        // printf("\nConnect to server at port: %d\n" % this->port_num);
    }
}

void Stub::operator()() {
    printf("Running stub.");
    // listen for message
    // int valread = read(client_fd, this->buffer, 1024 - 1);

    // run task
    // vector<string> files = {"first_file", "second_file"};
    // ThreadManager mapThreadMang(this->file_manager, &files);
    // mapThreadMang.executeMapThreads();

    // respond
    // char* msg = "Completed task";
    // send(this->client_fd, msg, strlen(msg), 0);
}

Stub::~Stub() {
    printf("\nClosing client\n");
    close(this->client_socket);
}

int Stub::fail_status() {
    return this->status;
}
