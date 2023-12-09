
#include "Stub.hpp"

#include "ThreadManager.hpp"

Stub::Stub(int port_num)
    : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num)),
      socket(io_service),
      port_num(port_num) {
}

void Stub::operator()() {
    cout << "Running Stub " << this->port_num << ".\n";
    acceptor.accept(socket);
    while (true) {
        size_t len = socket.read_some(boost::asio::buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error);  // Some other error.

        std::string message(buf.data(), len);
        std::cout << "{Stub; Port: " << this->port_num << "}: Received message: " << message << std::endl;

        if (message.find("FileManager:") != std::string::npos) {
            setupFileManager(message);
            startThreads(message);
        }

        // Send acknowledge message back to the sender
        std::string acknowledgeMsg = "Ack Stub: " + std::to_string(this->port_num);
        boost::asio::write(socket, boost::asio::buffer(acknowledgeMsg));

        std::cout << "Acknowledgement sent" << std::endl;
    }
}

void Stub::setupFileManager(std::string message) {
    this->filemanager = nullptr;
    try {
        this->filemanager = FileManager::GetInstance("workdir/input", "workdir/output", "workdir/temp");
    } catch (exception const& exc) {
        cerr << "Could not start FileManager: " << exc.what() << endl;
        exit(1);
    }
}

void Stub::startThreads(std::string message) {
    // TODO: get information from message rather than hardcoded.
    std::vector<std::string> input_files = {
        "All'sWellThatEndsWell.txt",
        "AMidSummerNightsDream.txt",
        "AsYouLikeIte.txt",
        "Cymbeline.txt"};

    // Initialize Thread Manager and launch map threads
    ThreadManager mapThreadMang(&input_files);
    mapThreadMang.executeMapThreads();

    // Initialize Thread Manager and launch map threads
    ThreadManager reduceThreadMang(&input_files);
    reduceThreadMang.executeReduceThreads();
}

Stub::~Stub() {
    cout << "Closing Stub client " << this->port_num << ".\n";
    socket.close(error);
    if (error)
        cout << "[!] ERROR: " << error;  // Some other error.
}
