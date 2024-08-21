#ifndef KLIENT_H
#define KLIENT_H

#include <string>
#include <vector>
#include "common.h"

// limit na rozmiar wiadomości
#define MESSAGE_LIMIT 1000
#define BUSY 1
#define DEAL 2
#define TRICK 3
#define WRONG 4
#define TAKEN 5
#define SCORE 6
#define TOTAL 7


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
        // wypisuje karty, które aktualnie posiada klient
        void print_hand();
        void print_trick_history();
        bool wait_DEAL = true;
        bool wait_TOTAl = true;
        bool wait_SCORE = true;
        bool wait_first_TRICK = true;
        bool got_TRICK = false;
        int current_trick = 1;
        std::vector<std::string> tricks_taken;

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
        int socket_fd;

        // metoday klienta
        int connect_to_server();
        bool is_valid_card(const std::string& card);
        int validate_message(const std::string& message);
};

#endif // KLIENT_H