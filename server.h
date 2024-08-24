#ifndef SERVER_H
#define SERVER_H

#include <string>
#include "common.h"
#include "cards.h"

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
        

    private:
        uint16_t port;
        std::string file;
        int timeout;
        CardSet cardSet;
        int current_trick;
        int connected_players;
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
        Gameplay gameplay;
};
#endif // SERVER_H