#pragma once

#include <string>
#include <memory>
#include "core/HostelSystem.hpp"
#include "core/AuthManager.hpp"
#include "dsa/Graph.hpp"
#include "dsa/Trie.hpp"
#include "dsa/SegmentTree.hpp"
#include "dsa/DisjointSet.hpp"

namespace httplib {
    class Server;
}

namespace hostel {

/**
 * @brief REST API and Static Web Server for Hostel Management System.
 */
class HttpServer {
private:
    std::shared_ptr<HostelSystem> system_;
    std::shared_ptr<AuthManager> auth_;
    std::unique_ptr<httplib::Server> server_;

    // Advanced DSA components hosted for interactive demos
    dsa::Graph campusGraph_;
    dsa::Trie searchTrie_;
    dsa::SegmentTree<float> feeSegmentTree_;
    dsa::DisjointSet roommateDSU_;

    int port_;
    std::string frontendPath_;

    void setupRoutes();
    void initCampusGraph();
    void syncSearchTrie();

public:
    HttpServer(std::shared_ptr<HostelSystem> sys,
               std::shared_ptr<AuthManager> auth,
               int port = 8080,
               std::string frontendPath = "frontend");
    ~HttpServer();

    void start();
    void stop();
};

} // namespace hostel
