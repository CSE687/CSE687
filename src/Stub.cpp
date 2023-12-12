
#include "Stub.hpp"

#include "ThreadManager.hpp"

Stub::Stub(int port_num)
    : acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num)),
      socket(io_context),
      port_num(port_num) {
}

void Stub::operator()() {
    cout << "Running Stub " << this->port_num << ".\n";
    acceptor.accept(socket);
    std::cout << "Accepted connection from " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << std::endl;
    listen();
    // this->threads[0] = new boost::thread(&Stub::listen, this);
    // this->threads[0]->join();
}

void Stub::joinThreads() {
    for (int i = 0; i < 2; i++) {
        this->threads[i]->join();
    }
}

void Stub::listen() {
    while (true) {
        boost::array<char, 128> buf;

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
        std::cout << "{Stub; Port: " << this->port_num << "}: Received message: ";
        boost::property_tree::write_json(std::cout, pt);

        // send ack ptree back to sender
        boost::property_tree::ptree ack;
        ack.put("message_type", "ack");
        std::stringstream ss_ack;
        boost::property_tree::write_json(ss_ack, ack);
        std::string ack_string = ss_ack.str();
        ack_string += "\n";
        boost::asio::write(socket, boost::asio::buffer(ack_string));

        std::cout << "Sent message: ";
        std::cout << ack_string << std::endl;

        if (message.find("FileManager:") != std::string::npos) {
            if (process_running.try_lock()) {
                this->threads[1] = new boost::thread(&Stub::setupFileManager, this, message);
                this->threads[1]->detach();
            }
        } else {
            if (process_running.try_lock()) {
                cout << "Starting start Threads\n";
                this->threads[1] = new boost::thread(&Stub::startThreads, this, message);
                this->threads[1]->detach();
            } else {
                cout << "{Stub; Port: " << this->port_num << "}: Cannot run, another process is currently running.\n";
            }
        }
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
    process_running.unlock();
}

void Stub::startThreads(std::string message) {
    // TODO: get information from message rather than hardcoded.
    std::vector<std::string> input_files = {
        "workdir/input/Cymbeline.txt",
        "workdir/input/TheTempest.txt",
        "workdir/input/TheComedyOfErrors.txt",
        "workdir/input/Love'sLabourLost.txt",
        "workdir/input/TheTwoGentlemenOfVerona.txt",
        "workdir/input/Winter'sTale.txt",
        "workdir/input/All'sWellThatEndsWell.txt",
        "workdir/input/TamingOfTheShrew.txt",
        "workdir/input/TheTwelthNight.txt",
        "workdir/input/PericlesPrinceOfTyre.txt",
        "workdir/input/TroilusAndCressida.txt",
        "workdir/input/AsYouLIkeIte.txt",
        "workdir/input/TheMerchantOfVenice.txt",
        "workdir/input/MeasureForMeasure.txt",
        "workdir/input/TheMerryWivesOfWindsor.txt",
        "workdir/input/MuchAdoAboutNothing.txt",
        "workdir/input/AMidSummerNightsDream.txt"};

    // Initialize Thread Manager and launch map threads
    ThreadManager mapThreadMang(&input_files);
    mapThreadMang.executeMapThreads();

    input_files = {
        "workdir/temp/Cymbeline.txt",
        "workdir/temp/TheTempest.txt",
        "workdir/temp/TheComedyOfErrors.txt",
        "workdir/temp/Love'sLabourLost.txt",
        "workdir/temp/TheTwoGentlemenOfVerona.txt",
        "workdir/temp/Winter'sTale.txt",
        "workdir/temp/All'sWellThatEndsWell.txt",
        "workdir/temp/TamingOfTheShrew.txt",
        "workdir/temp/TheTwelthNight.txt",
        "workdir/temp/PericlesPrinceOfTyre.txt",
        "workdir/temp/TroilusAndCressida.txt",
        "workdir/temp/AsYouLIkeIte.txt",
        "workdir/temp/TheMerchantOfVenice.txt",
        "workdir/temp/MeasureForMeasure.txt",
        "workdir/temp/TheMerryWivesOfWindsor.txt",
        "workdir/temp/MuchAdoAboutNothing.txt",
        "workdir/temp/AMidSummerNightsDream.txt"};

    // Initialize Thread Manager and launch map threads
    ThreadManager reduceThreadMang(&input_files);
    reduceThreadMang.executeReduceThreads();

    // Send acknowledge message back to the sender
    std::string acknowledgeMsg = "Ack Stub: " + std::to_string(this->port_num);
    boost::asio::write(socket, boost::asio::buffer(acknowledgeMsg));

    std::cout << "Acknowledgement sent" << std::endl;
    process_running.unlock();
}

Stub::~Stub() {
    for (int j = 0; j < 2; j++) {
        delete this->threads[j];
    }
    cout << "Closing Stub client " << this->port_num << ".\n";
    socket.close(error);
    if (error)
        cout << "[!] ERROR: " << error;  // Some other error.
}
