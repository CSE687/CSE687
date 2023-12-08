#ifndef STUB_HPP
#define STUB_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

class Stub {
   private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    int port;

   public:
    Stub(int port)
        : acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          socket(io_service),
          port(port) {}

    void startListening() {
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
            std::cout << "{Stub; Port: " << this->port << "}: Received message: " << message << std::endl;

            // Send acknowledge message back to the sender
            std::string acknowledgeMsg = "Ack Stub: " + std::to_string(this->port);
            boost::asio::write(socket, boost::asio::buffer(acknowledgeMsg));

            std::cout << "Acknowledgement sent" << std::endl;
        }
    }
};

#endif  // STUB_HPP
