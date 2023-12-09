
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
}

Stub::~Stub() {
    cout << "Closing Stub client " << this->port_num << ".\n";
    socket.close(error);
    if (error)
        cout << "[!] ERROR: " << error;  // Some other error.
}
