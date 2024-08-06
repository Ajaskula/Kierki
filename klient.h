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
        void run();

    private:

        // atrybuty klienta
        std::string host;
        uint16_t port;
        bool IPv4;
        bool IPv6;
        char position;
        bool isBot;
        std::set<Card> cardSet;

        // metoday klienta
        bool connect();
}

#endif // KLIENT_H