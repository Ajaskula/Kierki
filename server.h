#ifndef SERVER_H
#define SERVER_H

#include <string>
#include "common.h"

int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout);

class Server{
    public:
        // konstruktor
        Server(uint16_t port, const string& file, int timeout);
        // destruktor
        ~Server();

        // uruchamia serwer
        void run();

    private:
        uint16_t port;
        std::string file;
        int timeout;
        std::CardSet cardSet;
}
#endif // SERVER_H