
#include "Stub.hpp"

#include "ThreadManager.hpp"

// void Stub::error(const char* msg) {
//     perror(msg);
//     exit(0);
// }

void Stub::startListening() {
    acceptor.accept(socket);
    while (true) {
        boost::array<char, 128> buf;
        boost::system::error_code error;

        size_t len = socket.read_some(boost::asio::buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error);  // Some other error.

        std::string message(buf.data(), len);
        std::cout << "{Stub; Port: " << this->port_num << "}: Received message: " << message << std::endl;

        // Send acknowledge message back to the sender
        std::string acknowledgeMsg = "Ack Stub: " + std::to_string(this->port_num);
        boost::asio::write(socket, boost::asio::buffer(acknowledgeMsg));

        std::cout << "Acknowledgement sent" << std::endl;
    }
}

Stub::Stub(int port_num)
    : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num)),
      socket(io_service),
      port_num(port_num) {
    this->file_manager = FileManager::GetInstance();
}

// this->file_manager = FileManager::GetInstance();
// this->port_num = port;
// this->client_socket = socket(AF_LOCAL, SOCK_STREAM, 0);
// if (this->client_socket < 0) {
//     printf("[!] Socket creation error\n");
//     this->status = -1;
//     return;
// }
// this->serv_addr.sin_family = AF_LOCAL;
// this->serv_addr.sin_port = htons(port_num);
// this->status = connect(this->client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
// if (this->status < 0) {
//     printf("[!] Connection Failed\n");
// } else {
//     printf("Connected to server\n");
//     // printf("\nConnect to server at port: %d\n" % this->port_num);
// }
// }

void Stub::operator()() {
    printf("Running stub.\n");
    startListening();
    // listen for message
    // int valread = read(client_fd, this->buffer, 1024 - 1);

    // run task
    // vector<string> files = {"first_file", "second_file"};
    // ThreadManager mapThreadMang(this->file_manager, &files);
    // mapThreadMang.executeMapThreads();

    // respond
    // char* msg = "Completed task";
    // send(this->client_fd, msg, strlen(msg), 0);

    // Obtain vector of input files
    // vector<string> input_files = this->file_manager->getDirectoryFileList(this->file_manager->getInputDirectory());

    // // Initialize Thread Manager and launch map threads
    // ThreadManager mapThreadMang(this->file_manager, &input_files);
    // mapThreadMang.executeMapThreads();

    // // Obtain vector of input files
    // vector<string> temp_files = this->file_manager->getDirectoryFileList(this->file_manager->getTempDirectory());

    // // Initialize Thread Manager and launch map threads
    // ThreadManager reduceThreadMang(this->file_manager, &temp_files);
    // reduceThreadMang.executeReduceThreads();
}

Stub::~Stub() {
    printf("\nClosing client\n");
    close(this->client_socket);
}

int Stub::fail_status() {
    return this->status;
}
