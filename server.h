#ifndef SERVER_H
#define SERVER_H

#include <string>
#include "common.h"
#include "cards.h"
#include "sys/socket.h"
#include "poll.h"

class Server{
    public:
        Server(uint16_t port, const std::string& file, int timeout);
        ~Server();

        static int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout);
        int run();
        bool validateIAM(const std::string& message);
        bool validateTRICK(const std::string& message);
        // tasuje talię i rozdaje karty
        std::vector<std::string> shuffleAndCreateDeals();
        int pointsInTrick(const std::string& trick);
        int calculateNumOfHeartsInTrick(const std::string& trick);
        int calculateNumOfQueensInTrick(const std::string& trick);
        int calculateNumOfManInTrick(const std::string& trick);
        int checkIfKingOfHeartsInTrick(const std::string& trick);
        int readDealsFromFile(const std::string& filename);
        int setupServerSocketIPv4();
        int setupServerSocketIPv6();

    private:
        uint16_t port;
        std::string file;
        int timeout;
        int connected_players;
        Gameplay gameplay;
        int queue_length;
        bool is_N_connected;
        bool is_S_connected;
        bool is_W_connected;
        bool is_E_connected;
        int current_trick;
        // triczki przyjęte przez konkretnego gracza
        std::vector<std::string> tricks_taken_N;
        std::vector<std::string> tricks_taken_S;
        std::vector<std::string> tricks_taken_W;
        std::vector<std::string> tricks_taken_E;
        // rozdanie dla konkretnego gracza
        std::vector<std::string> deal_N;
        std::vector<std::string> deal_S;
        std::vector<std::string> deal_W;
        std::vector<std::string> deal_E;
        int pointsInTrick(const std::string& trick, int type);
        // karty, które są na ręce konkretnego gracza
        std::vector<std::string> cards_on_hand_N;
        std::vector<std::string> cards_on_hand_S;
        std::vector<std::string> cards_on_hand_W;
        std::vector<std::string> cards_on_hand_E;
};
#endif // SERVER_H