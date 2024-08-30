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
#include "signal.h"
#include <chrono>

class Server{
    public:
        Server(uint16_t port, const std::string& file, int timeout);
        ~Server();

        static int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout);
        int run();


    private:
        // Function check if the message is proper TRICK
        bool validateTRICK(const std::string& message);
        int calculateNumOfHeartsInTrick(const std::string& trick);
        int calculateNumOfQueensInTrick(const std::string& trick);
        int calculateNumOfManInTrick(const std::string& trick);
        int checkIfKingOfHeartsInTrick(const std::string& trick);
        // read list of deals to play from the file
        int readDealsFromFile(const std::string& filename);
        int setupServerSocket();
        // calculates time to next timeout
        int calculateTimeToWait();
        int validateIAM(const std::string& message);
        // tidy up waiting room descriptors and buffers
        void realiseWaitingRoom(struct pollfd fds[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        // returns string with busy places
        std::string busyPlacesToString();
        void assignClientToPlace(char message, struct pollfd* fds);
        void initialize_poll_descriptors(int socket_fd, struct pollfd* fds);
        void initializeBuffers(char buffer[11][1024], size_t buffer_counter[11]);
        void reset_revents(struct pollfd fds[11]);
        // response to IAM depending on if it is proper or not
        void responseToIAM(const std::string& message, struct pollfd fds[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        void send_message(int fd, const std::string& message);
        // check if are players are connected
        bool areAllPlayersConnected();
        // finds out who is next player
        int get_next_player();
        int getPlayerfromChar(char player);
        // takes card away from player
        void takeCardAwayFromPlayer(const std::string& card);
        // finds out who take trick
        int whoTakeTrick(int first_player, const std::string& trick);
        char getCharOfPlayer(int player);
        // reset points after deal
        void zeroPointsInDeal();
        void wypisz_punkty();
        // add points to the player who took trick
        void addPointsToPlayer(int player, int points);
        // add taken trick to history
        void addTakenToHistory(const std::string& trick);
        // reste taken history after deal
        void zeroTakenHistory();
        std::string prepareTotalMessage();
        std::string prepareScoreMessage();
        void closeAllDescriptors(struct pollfd fds[11]);
        // manage connection clients
        int manConnectionSocket(int socket_fd, struct pollfd fds[11]);
        // manage client in waiting room wairing to receive IAM
        int manWaitingRoom(struct pollfd fds[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        // send deals to all players
        void dealCardsToPlayers(struct pollfd fds[11]);
        void reset_deals_sent();

        // return deal for a given player
        std::string getProperDeal(char player);
        std::string getDealCardsForPlayer(char type);
        // disconnect player and make it known in serwer
        void disconnectPlayer(int player);
        // if player can play card according to rules
        bool checkIfPlayerCanPlayCard(const std::string& card);
        // check if player has card
        bool checkIfPlayerHasCard(const std::string& card);
        void disconnectAllPlayers(struct pollfd fds[11]);
        // manage dealing cards
        void manageDealCards(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        // manage sending taken trick to players
        void manageSendingTaken(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        // manage sending score and total to players
        void manageSendingScoreAndTotal(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]);
        // return number of the trick based on trick massage
        int calculateTrickNumFromTRICK(const std::string& trick);
        // calculate points in trick
        int pointsInTrick(const std::string& trick, int type);
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
        std::vector<std::string> deal_N;
        std::vector<std::string> deal_S;
        std::vector<std::string> deal_W;
        std::vector<std::string> deal_E;
        std::vector<std::string> cards_on_hand_N;
        std::vector<std::string> cards_on_hand_S;
        std::vector<std::string> cards_on_hand_W;
        std::vector<std::string> cards_on_hand_E;
        int current_deal_number;
        int current_player;
        int pointsInDeal[4] = {0, 0, 0, 0};
        int pointsInTotal[4] = {0, 0, 0, 0};
        std::vector<std::string> takenHistory;
        int how_many_added_card;
        int first_player_in_current_trick;
        bool time_to_deal;
        Deal current_deal;
        int number_of_deals_to_play;
        bool deals_sent[4] = {false, false, false, false};
        int player_receiving_deal;
        std::vector<CardSet> cards_of_players;
        int current_player_receiving_trick;
        bool trick_sent[4] = {false, false, false, false};
        bool sending_wrong[4] = {false, false, false, false};
        int player_receiving_taken;
        int player_receiving_score_and_total;
        std::string local_address;
        std::vector<std::string> messagesToSendFromWaitingRoom;
        char assignFromWaitingRoom;
        bool disconnectFromWaitingRoom;
        std::string message_to_raport_from_waiting_room;
        bool score_sent;
        static void sigpipe_handler(int signo);

};
#endif // SERVER_H