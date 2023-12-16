
#include "Stub.hpp"

#include "ThreadManager.hpp"

Stub::Stub(int port_num)
    : acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num)),
      socket(io_context),
      port_num(port_num) {
}

void Stub::operator()() {
    acceptor.accept(socket);
    std::cout << "Accepted connection from " << socket.remote_endpoint().address().to_string() << ":" << socket.remote_endpoint().port() << std::endl;
    listen();
}

void Stub::listen() {
    while (true) {
        boost::array<char, 496> buf;

        size_t len = socket.read_some(boost::asio::buffer(buf), error);

        if (error == boost::asio::error::eof)
            break;  // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error);  // Some other error.

        std::string message(buf.data(), len);
        receiveMessage(message);
    }
}

void Stub::receiveMessage(std::string message) {
    // parse message into ptree
    std::stringstream ss;
    ss << message;
    boost::property_tree::ptree receivedPTree;
    try {
        boost::property_tree::read_json(ss, receivedPTree);
        std::cout << "[Stub; Port: " << this->port_num << "]: Received message: ";
        boost::property_tree::write_json(std::cout, receivedPTree);

        performTask(receivedPTree);
    } catch (const boost::property_tree::json_parser_error& e) {
        // Handle the exception (e.g., print error message)
        std::cerr << "[Stub; Port: " << this->port_num << "]: Failed to read JSON: " + std::string(e.what()) + "\n";
        return;
    }
}

void Stub::performTask(boost::property_tree::ptree message) {
    std::string message_type = message.get<std::string>("message_type");

    if (message_type == "establish_connection") {
        std::string input_directory = message.get<std::string>("input_directory");
        std::string output_directory = message.get<std::string>("output_directory");
        std::string temp_directory = message.get<std::string>("temp_directory");
        if (this->stubProcess.process_lock.try_lock()) {
            this->stubProcess.process_thread = new boost::thread(&Stub::setupFileManager, this, input_directory, output_directory, temp_directory);
            this->stubProcess.process_thread->detach();
        }
    } else if (message_type == "heartbeat") {
        // send ack ptree back to sender
        boost::property_tree::ptree ack;
        ack.put("message_type", "ack");
        sendMessage(ack);
    } else if (message_type == "map_task") {
        if (this->stubProcess.process_lock.try_lock()) {
            std::string file_list = message.get<std::string>("files");
            this->stubProcess.batch_id = message.get<int>("batch_id");
            std::vector<std::string> input_files = getFileList(file_list);

            this->stubProcess.process_thread = new boost::thread(&Stub::startMapThreads, this, input_files);
            this->stubProcess.process_thread->detach();
        } else {
            cout << "[Stub; Port: " << this->port_num << "]: Cannot run, another process is currently running.\n";
            // Send error back to sender
            boost::property_tree::ptree error;
            error.put("message_type", "batch_status");
            error.put("batch_id", message.get<std::string>("batch_id"));
            error.put("status", "Error");
            sendMessage(error);
        }
    } else if (message_type == "reduce_task") {
        if (stubProcess.process_lock.try_lock()) {
            std::string file_list = message.get<std::string>("files");
            this->stubProcess.batch_id = message.get<int>("batch_id");
            std::vector<std::string> temp_files = getFileList(file_list);

            this->stubProcess.process_thread = new boost::thread(&Stub::startReduceThreads, this, temp_files);
            this->stubProcess.process_thread->detach();
        } else {
            cout << "[Stub; Port: " << this->port_num << "]: Cannot run, another process is currently running.\n";
            // Send error back to sender
            boost::property_tree::ptree error;
            error.put("message_type", "batch_status");
            error.put("batch_id", message.get<std::string>("batch_id"));
            error.put("status", "Error");
            sendMessage(error);
        }
    } else {
        std::cout << "[Stub; Port: " << this->port_num << "]: Received unkown message type.\n";
    }
}

std::vector<std::string> Stub::getFileList(std::string file_message) {
    std::stringstream ss(file_message);
    std::string file;
    std::vector<std::string> file_list;
    while (!ss.eof()) {
        std::getline(ss, file, ',');
        file_list.push_back(file);
    }
    return file_list;
}

void Stub::sendMessage(const boost::property_tree::ptree message) {
    std::ostringstream buf;
    boost::property_tree::write_json(buf, message, false);
    std::string jsonString = buf.str();
    // jsonString += "\n";

    // Send the JSON string to the stub
    boost::asio::write(this->socket, boost::asio::buffer(jsonString));
    std::cout << "[Stub; Port: " << this->port_num << "]: Sent message: ";
    std::cout << jsonString << std::endl;
}

void Stub::setupFileManager(std::string input_directory, std::string output_directory, std::string temp_directory) {
    this->filemanager = nullptr;
    try {
        this->filemanager = FileManager::GetInstance(input_directory, output_directory, temp_directory);
    } catch (exception const& exc) {
        cerr << "Could not start FileManager: " << exc.what() << endl;
        exit(1);
    }
    this->stubProcess.process_lock.unlock();
}

void Stub::startMapThreads(std::vector<std::string> input_files) {
    boost::property_tree::ptree inprogress;
    inprogress.put("message_type", "batch_status");
    inprogress.put("batch_id", this->stubProcess.batch_id);
    inprogress.put("message", "InProgress");
    sendMessage(inprogress);
    // Initialize Thread Manager and launch map threads
    try {
        ThreadManager mapThreadMang(&input_files);
        mapThreadMang.executeMapThreads();

        // Send acknowledge message back to the sender
        boost::property_tree::ptree complete;
        complete.put("message_type", "batch_status");
        complete.put("batch_id", this->stubProcess.batch_id);
        complete.put("message", "Complete");
        sendMessage(complete);
        this->stubProcess.process_lock.unlock();
    } catch (exception e) {
        // Send error back to sender
        boost::property_tree::ptree error;
        error.put("message_type", "batch_status");
        error.put("batch_id", this->stubProcess.batch_id);
        error.put("status", "Error");
        sendMessage(error);
    }
}

void Stub::startReduceThreads(std::vector<std::string> input_files) {
    boost::property_tree::ptree inprogress;
    inprogress.put("message_type", "batch_status");
    inprogress.put("batch_id", this->stubProcess.batch_id);
    inprogress.put("message", "InProgress");
    sendMessage(inprogress);
    // Initialize Thread Manager and launch reduce threads
    try {
        ThreadManager reduceThreadMang(&input_files);
        reduceThreadMang.executeReduceThreads();
        // Send acknowledge message back to the sender
        boost::property_tree::ptree complete;
        complete.put("message_type", "batch_status");
        complete.put("batch_id", this->stubProcess.batch_id);
        complete.put("message", "Complete");
        sendMessage(complete);
    } catch (exception e) {
        // Send error back to sender
        boost::property_tree::ptree error;
        error.put("message_type", "batch_status");
        error.put("batch_id", this->stubProcess.batch_id);
        error.put("status", "Error");
        sendMessage(error);
    }
    this->stubProcess.process_lock.unlock();
}

Stub::~Stub() {
    delete this->stubProcess.process_thread;
    cout << "Closing Stub client " << this->port_num << ".\n";
    socket.close(error);
    if (error)
        cout << "[!] ERROR: " << error;  // Some other error.
}
