#ifndef KLIENT_H
#define KLIENT_H

#include <string>
#include "common.h"

int parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot);

class Klient{
    public:
        // konstruktor klienta
        Klient(const string host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot);
        // destruktor klienta
        ~Klient();
        // metoda uruchamiająca klienta
        int run();

    private:

        // atrybuty klienta
        std::string host;
        uint16_t port;
        bool IPv4;
        bool IPv6;
        char position;
        bool isBot;
        std::CardSet cardSet;
        int points;

        // metoday klienta
        int connect_to_server();
}

#endif // KLIENT_H