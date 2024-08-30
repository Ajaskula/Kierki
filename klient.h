#ifndef KLIENT_H
#define KLIENT_H

#include <string>
#include <vector>
#include <unordered_map>
#include <netdb.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <regex>
#include <set>
#include "cards.h"
#include "common.h"

#define BUSY 1
#define DEAL 2
#define TRICK 3
#define WRONG 4
#define TAKEN 5
#define SCORE 6
#define TOTAL 7

std::string extract_card_list_from_taken(const std::string& message);
std::vector<std::string> extract_card_vector_from_taken(const std::string& message);

class Klient{
    public:
        Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot);
        ~Klient();
        int run();
        // Function that parses arguments
        static int parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot);

    private:
        
        std::string host;
        uint16_t port;
        bool IPv4;
        bool IPv6;
        char position;
        bool isBot;
        CardSet cardSet;
        std::vector<std::string> tricks_taken;
        int socket_fd;
        bool wait_DEAL;
        bool wait_TOTAL;
        bool wait_SCORE;
        bool wait_first_TRICK;
        bool got_TRICK;
        int current_trick;
        bool sending;
        std::string messageToRaport;
        int buffer_index;
        int buffer_sending_counter;
        // Function that validates the message BUSY
        bool validate_BUSY(const std::string& message);
        // Function that validates the message DEAL
        bool validate_DEAL(const std::string& message);
        // Function that places message in the buffer in order to send data
        void send_message(char buffer[1024], const std::string& message);
        // Function that receives message from the server
        int receive_message(int socket_fd);
        int connectToServer();
        // Function that validates the message TRICK
        bool validateTRICK(const std::string& message);
        // Function that validates the message TAKEN
        bool validateTAKEN(const std::string& message);
        // Function that validates the message WRONG
        bool validateWRONG(const std::string& message);
        // Function that validates the message SCORE
        bool validateSCORE(const std::string& message);
        // Function that validates the message TOTAL
        bool validateTOTAL(const std::string& message);
        // Function that validate the message DEAL
        bool validateDEAL(const std::string& message);
        // Function that validates the message BUSY
        bool validateBUSY(const std::string& message);
        // Function that returns the list of busy places
        std::string print_busy_places(const std::string& message);
        // Function that prints taken tricks
        void printTakenTricks();

        void performTaken(const std::string& message);
        // checks if the string is a valid hand dealed
        bool isStringValidHandDealed(const std::string& hand);
};

#endif // KLIENT_H