#pragma once
#include <attaServer/network/Acceptor.h>
class App {
    asio::io_context io_;
    Acceptor acceptor_;
public:
    explicit App();
    void run();
};