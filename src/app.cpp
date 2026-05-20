#include <attaServer/App.h>
#include <attaServer/logger/Logger.h>
App::App() : acceptor_(io_.get_executor()){}

void init_logger() {
    Logger::setBufferSize(8192*100);
    Logger::setLevel(LogLevel::INFO);
    Logger::setOutputFilename("Log.txt");
    Logger::start();
}

void App::run()  {
    init_logger();
    acceptor_.start();
    io_.run();
}
