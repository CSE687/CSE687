#ifndef STUB_HPP
#define STUB_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>

class Stub {
   private:
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    int port;

   public:
    Stub(int port)
        : acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
          socket(io_context),
          port(port) {}

    void startListening() {
        acceptor.accept(socket);
        std::cout << "Accepted connection from " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << std::endl;
        while (true) {
            boost::array<char, 128> buf;
            boost::system::error_code error;

            size_t len = socket.read_some(boost::asio::buffer(buf), error);

            if (error == boost::asio::error::eof)
                break;  // Connection closed cleanly by peer.
            else if (error)
                throw boost::system::system_error(error);  // Some other error.

            std::string message(buf.data(), len);
            // parse message into ptree
            std::stringstream ss;
            ss << message;
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(ss, pt);
            // print ptree
            std::cout << "Received message: ";
            boost::property_tree::write_json(std::cout, pt);

            // send ack ptree back to sender
            boost::property_tree::ptree ack;
            ack.put("message_type", "ack");
            std::stringstream ss_ack;
            boost::property_tree::write_json(ss_ack, ack);
            std::string ack_string = ss_ack.str();
            boost::asio::write(socket, boost::asio::buffer(ack_string));

            std::cout << "Sent message: ";
            std::cout << ack_string << std::endl;
        }
    }
};

#endif  // STUB_HPP
