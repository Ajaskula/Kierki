#ifndef SERVER_H
#define SERVER_H

#define MAX_PLAYERS 4
#define CONNECTION_SOCKET 0
#define NREAD 1
#define EREAD 2
#define SREAD 3
#define WREAD 4
#define PREAD 5
#define NWRITE 6
#define EWRITE 7
#define SWRITE 8
#define WWRITE 9
#define PWRITE 10
#define BUFFER_SIZE 1024


#include <string>
#include "common.h"
#include "cards.h"
#include "sys/socket.h"
#include "poll.h"
#include <chrono>

class Server{
    public:
        Server(uint16_t port, const std::string& file, int timeout);
        ~Server();

        static int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout);
        int run();
        bool validateTRICK(const std::string& message);
        // int pointsInTrick(const std::string& trick, int type);
        int calculateNumOfHeartsInTrick(const std::string& trick);
        int calculateNumOfQueensInTrick(const std::string& trick);
        int calculateNumOfManInTrick(const std::string& trick);
        int checkIfKingOfHeartsInTrick(const std::string& trick);
        int readDealsFromFile(const std::string& filename);
        int setupServerSocket();
        int calculateTimeToWait();
        int validateIAM(const std::string& message);
        void realiseWaitingRoom(struct pollfd fds[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        std::string busyPlacesToString();
        void assignClientToPlace(const std::string& message, struct pollfd* fds);
        void initialize_poll_descriptors(int socket_fd, struct pollfd* fds);
        void initializeBuffers(char buffer[11][1024], size_t buffer_counter[11]);
        void reset_revents(struct pollfd fds[11]);
        void responseToIAM(const std::string& message, struct pollfd fds[11]);
        void send_message(int fd, const std::string& message);
        void wypisz_zdarzenia(struct pollfd fds[11]);
        bool areAllPlayersConnected();
        int get_next_player();
        int getPlayerfromChar(char player);
        void takeCardAwayFromPlayer(const std::string& card);
        int whoTakeTrick(int first_player, const std::string& trick);
        char getCharOfPlayer(int player);
        void zeroPointsInDeal();
        void wypisz_punkty();
        void addPointsToPlayer(int player, int points);
        void addTakenToHistory(const std::string& trick);
        void zeroTakenHistory();
        std::string prepareTotalMessage();
        std::string prepareScoreMessage();
        void closeAllDescriptors(struct pollfd fds[11]);
        int manConnectionSocket(int socket_fd, struct pollfd fds[11]);
        int manWaitingRoom(struct pollfd fds[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        void dealCardsToPlayers(struct pollfd fds[11]);
        void reset_deals_sent();
        std::string getProperDeal(char player);
        std::string getDealCardsForPlayer(char type);
        void disconnectPlayer(int player);
        bool checkIfPlayerCanPlayCard(const std::string& card);
        bool checkIfPlayerHasCard(const std::string& card);


    private:
        uint16_t port;
        std::string file;
        int timeout;
        int connected_players;
        Gameplay gameplay;
        int queue_length;
        bool is_E_connected;
        bool is_N_connected;
        bool is_S_connected;
        bool is_W_connected;
        int current_trick;
        int last_event_IAM;
        int last_event_TRICK;
        std::string lined_cards;
        bool finish;
        std::chrono::steady_clock::time_point time_point_IAM;
        std::chrono::steady_clock::time_point time_point_TRICK;
        // triczki przyjęte przez konkretnego gracza
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
        int current_deal_number;
        int current_player;
        int pointsInDeal[4] = {0, 0, 0, 0};
        int pointsInTotal[4] = {0, 0, 0, 0};
        std::vector<std::string> takenHistory;
        // tutaj nowe
        int how_many_added_card;
        int first_player_in_current_trick;
        bool time_to_deal;
        Deal current_deal;
        int number_of_deals_to_play;
        bool deals_sent[4] = {false, false, false, false};
        // bool finish;
        int player_receiving_deal;
        std::vector<CardSet> cards_of_players;
        int current_player_receiving_trick;
        bool trick_sent[4] = {false, false, false, false};
        int player_receiving_taken;
        int player_receiving_score_and_total;

};
#endif // SERVER_H