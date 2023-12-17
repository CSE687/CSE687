#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <iostream>
#include <string>
#include <vector>

#include "FileManager.hpp"
#include "StubConnection.hpp"
#include "TaskManager.hpp"

class Controller {
   private:
    FileManager* fileManager;
    int port;
    std::vector<std::shared_ptr<StubConnection>> stubConnections;
    std::shared_ptr<TaskManager> taskManager;
    std::mutex coutMutex;

    void createStubConnection(int stub_id, int port);
    void writeConsole(std::ostream& outputStream, const std::string& message);

   public:
    Controller(FileManager* fileManager, int port, std::vector<int>& ports);
    void execute();
};

#endif  // CONTROLLER_HPP
