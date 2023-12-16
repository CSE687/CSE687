
#ifndef PROPERTY_TREE_QUEUE_HPP
#define PROPERTY_TREE_QUEUE_HPP

#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>

class PropertyTreeQueue {
   private:
    int stub_id;
    std::queue<boost::property_tree::ptree> queue;
    std::mutex& coutMutex;

    void writeConsole(std::ostream& outputStream, const std::string& message);

   public:
    std::mutex queueMutex;

    PropertyTreeQueue(std::mutex& coutMutex, int stub_id) : coutMutex(coutMutex), stub_id(stub_id){};

    void push(const boost::property_tree::ptree& message);

    boost::property_tree::ptree pop();

    bool hasMessage() const;

    void clear();
};

#endif  // PROPERTY_TREE_QUEUE_HPP
