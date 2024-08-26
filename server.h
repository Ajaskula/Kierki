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
        // tasuje talię i rozdaje karty
        std::vector<std::string> shuffleAndCreateDeals();
        int pointsInTrick(const std::string& trick);
        int calculateNumOfHeartsInTrick(const std::string& trick);
        int calculateNumOfQueensInTrick(const std::string& trick);
        int calculateNumOfManInTrick(const std::string& trick);
        int checkIfKingOfHeartsInTrick(const std::string& trick);
        int readDealsFromFile(const std::string& filename);
        int setupServerSocket();
        int calculateTimeToWait();
        int validateIAM(const std::string& message);
        void realiseWaitingRoom(struct pollfd fds[11], char buffer[10][BUFFER_SIZE], size_t buffer_counter[10]);
        std::string busyPlacesToString();
        void assignClientToPlace(const std::string& message, struct pollfd* fds);
        void initialize_poll_descriptors(int socket_fd, struct pollfd* fds);
        void initializeBuffers(char buffer[10][1024], size_t buffer_counter[10]);
        void reset_revents(struct pollfd fds[11]);
        void responseToIAM(const std::string& message, struct pollfd fds[11]);
        void send_message(int fd, const std::string& message);
        void wypisz_zdarzenia(struct pollfd fds[11]);
        int setupServerSocketIPv4();

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