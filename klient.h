#ifndef KLIENT_H
#define KLIENT_H

#include <string>
#include "common.h"


class Klient{
    public:
        // konstruktor klienta
        Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot);
        // destruktor klienta
        ~Klient();
        // metoda uruchamiająca klienta
        int run();
        static int parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot);
        bool validate_BUSY(const std::string& message);
        bool validate_DEAL(const std::string& message);
        void send_message(int socket_fd, const std::string& message);
        int receive_message(int socket_fd);

    private:

        // atrybuty klienta
        std::string host;
        uint16_t port;
        bool IPv4;
        bool IPv6;
        char position;
        bool isBot;
        CardSet cardSet;
        int points;

        // metoday klienta
        int connect_to_server();
        bool is_valid_card(const std::string& card);
};

#endif // KLIENT_H