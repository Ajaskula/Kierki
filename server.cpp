#include <iostream>
#include "server.h"
#include "common.h"
#include <cinttypes>
#include <thread>

int Server::parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout){
    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-p" && i + 1 < argc){
            port = read_port(argv[i + 1]);
            i++;
        } else if(arg == "-f" && i + 1 < argc){

            file = argv[i + 1];
            i++;
        }else if(arg == "-t" && i + 1 < argc){
            timeout = std::stoi(argv[i+1]);
            if(timeout < 1){
                std::cerr << "Timeout musi być większy= od 1\n";
                return 1;
            }
            i++;
        }else{
            std::cerr << "Nieznany parametr: " << arg << "\n";
            return 1;
        }
    }

    if(file.empty()){
        std::cerr << "Błąd parametr -f jest wymagany\n";
        return 1;
    }
    return 0;
}
Server::Server(uint16_t port, const std::string& file, int timeout)
        : port(port), file(file), timeout(timeout * 1000), connected_players(0), gameplay(file), 
        queue_length(10), is_E_connected(false), is_N_connected(false), is_S_connected(false), is_W_connected(false), current_trick(1),
        last_event_IAM(-1), last_event_TRICK(-1), lined_cards(""), finish(false), time_point_IAM(), time_point_TRICK(), current_deal_number(0),
        takenHistory(), how_many_added_card(0), first_player_in_current_trick(-1), current_deal(),
        number_of_deals_to_play(gameplay.getNumberOfDeals()), player_receiving_deal(-1), cards_of_players(4, CardSet()), current_player_receiving_trick(-1),
        player_receiving_taken(0), player_receiving_score_and_total(0), local_address(), messagesToSendFromWaitingRoom(),
        assignFromWaitingRoom('B'), disconnectFromWaitingRoom(false), message_to_raport_from_waiting_room(""), score_sent(false)
        {}

Server::~Server(){}

void Server::send_message(int socket_fd, const std::string &message){
    size_t length = message.length();
    ssize_t bytes_sent = send(socket_fd, message.c_str(), length, 0);

    if(bytes_sent == -1){
        std::cerr << "Błąd podczas wysyłania wiadomości\n";
        close(socket_fd);
        exit(1);
    }
    raport(get_local_address(socket_fd), get_server_address(socket_fd), message);
}
int Server::validateIAM(const std::string& message){
    if(message.length() != 6){
        return -1;
    }
    if(message[3] != 'N' && message[3] != 'S' && message[3] != 'W' && message[3] != 'E'){
        return -1;
    }
    if(is_E_connected && (message[3] == 'E')){
        return 1;
    }
    if(is_N_connected && (message[3] == 'N')){
        return 1;
    }
    if(is_S_connected && (message[3] == 'S')){
        return 1;
    }
    if(is_W_connected && (message[3] == 'W')){
        return 1;
    }

    return 0;
}

int Server::setupServerSocket(){

    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return -1;
    }

    int flag = 0;
    if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&flag, sizeof(flag)) < 0) {
        std::cerr << "Błąd podczas ustawiania opcji gniazda\n";
        return -1;
    }

    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);

    if(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t) sizeof(server_address)) < 0){
        std::cerr << "Błąd podczas bindowania gniazda\n";
        close(socket_fd);
        return -1;
    }

    if(listen(socket_fd, queue_length) < 0){
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe\n";
        close(socket_fd);
        return -1;
    }
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << "Błąd podczas pobierania flag gniazda\n";
        close(socket_fd);
        return -1;
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << "Błąd podczas ustawiania trybu nieblokującego\n";
        close(socket_fd);
        return -1;
    }

    socklen_t length = (socklen_t) sizeof server_address;
    if (getsockname(socket_fd, (struct sockaddr*) &server_address, &length) < 0){
        std::cerr << "Błąd podczas pobierania numeru portu\n";
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}
bool Server::validateTRICK(const std::string& trick){
    std::string message = trick;
    if(message.length() < 10 || message.length() > 12){
        return false;
    }
    if(message.substr(0, 5) != "TRICK"){
        return false;

    }
    message = message.substr(5, message.length() - 7);

    if(message[message.length() - 1] != 'C' && message[message.length() - 1] != 'D' && message[message.length() - 1] != 'H' && message[message.length() - 1] != 'S'){
        return false;
    }
    message = message.substr(0, message.length() - 1);
    if(message[message.length() - 1] == '0'){
        if(message[message.length() - 2] != '1'){
            return false;
        }
        message = message.substr(0, message.length() - 2);

    }else if(((message[message.length() - 1] <= '9' && message[message.length() - 1] >= '2') || message[message.length() - 1] == 'J' || message[message.length() - 1] == 'Q' || message[message.length() - 1] == 'K' || message[message.length() - 1] == 'A')){
        message = message.substr(0, message.length() - 1);
    }else{
        return false;
    }

    if(message.length() == 0){
        return false;
    }
    int trick_number = 0;
    if(message.length() == 1){
        trick_number = message[0] - '0';
    }
    if(message.length() == 2){
        trick_number = 10 * (message[0] - '0');
        trick_number += message[1] - '0';
    }
    if(trick_number < 1 || trick_number > 13){
        return false;
    }
    return true;
}
int Server::pointsInTrick(const std::string& trick, int type){
    int sum_of_points = 0;
    switch(type){
        case 1:
            return 1;
        case 2:
            return calculateNumOfHeartsInTrick(trick);
        case 3:
            return 5 * calculateNumOfQueensInTrick(trick);
        case 4:
            return 2 * calculateNumOfManInTrick(trick);
        case 5:
            return 18 * checkIfKingOfHeartsInTrick(trick);
        case 6:
            return 10 * ((current_trick == 7) + (current_trick == 13));
        case 7:
            for(int i = 1; i < 7; i+=1){
                sum_of_points += pointsInTrick(trick, i);
            }
            return sum_of_points;
        default:
            return 0;
    }
    return 0;
}
int Server::calculateTrickNumFromTRICK(const std::string& trick){
    std::string message = trick;
    message = message.substr(5, message.length() - 7);

    message = message.substr(0, message.length() - 1);
    if(message[message.length() - 1] == '0'){
 
        message = message.substr(0, message.length() - 2);

    }else if(((message[message.length() - 1] <= '9' && message[message.length() - 1] >= '2') || message[message.length() - 1] == 'J' || message[message.length() - 1] == 'Q' || message[message.length() - 1] == 'K' || message[message.length() - 1] == 'A')){
        message = message.substr(0, message.length() - 1);
    }

    int trick_number = 0;
    if(message.length() == 1){
        trick_number = message[0] - '0';
    }
    if(message.length() == 2){
        trick_number = 10 * (message[0] - '0');
        trick_number += message[1] - '0';
    }
    return trick_number;
}

int Server::whoTakeTrick(int first_player, const std::string& trick){
    
    char first_card_color = get_first_card_color_from_TRICK(trick);
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick.substr(6 + (current_trick >= 10), trick.length() - 8 - (current_trick >= 10)));
    int max_card_number = 0;
    for(int i = 1; i < 4; i++){
        if(cards[i][cards[i].length() - 1] == first_card_color){
            if(Card::stringToRank(cards[i].substr(0, cards[i].length() - 1)) > Card::stringToRank(cards[max_card_number].substr(0, cards[max_card_number].length() - 1))){
                max_card_number = i;
            }
        }
    }
    return (first_player + max_card_number) % 4;
}
int Server::calculateNumOfHeartsInTrick(const std::string& trick){
    int number_of_hearts = 0;
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick);
    for(std::string card : cards){
        if(card[card.length() - 1] == 'H'){
            number_of_hearts += 1;
        }

    }
    return number_of_hearts;
}
int Server::calculateNumOfQueensInTrick(const std::string& trick){
    int sum_of_points = 0;
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick);
    for(std::string card : cards){
        if(card[0] == 'Q'){
            sum_of_points += 1;
        }
    }
    return sum_of_points;
}
int Server::calculateNumOfManInTrick(const std::string& trick){
    int sum_of_points = 0;
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick);
    for(std::string card : cards){
        if(card[0] == 'K' || card[0] == 'J'){
            sum_of_points += 1;
        }
    }
    return sum_of_points;
}
int Server::checkIfKingOfHeartsInTrick(const std::string& trick){
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick);
    for(std::string card : cards){
        if(card == "KH"){
            return 1;
        }
    }
    return 0;
}
int Server::calculateTimeToWait() {
    if (last_event_IAM == -1 && last_event_TRICK == -1) {
        return -1;  
    }

    int timeout_ms = timeout;
    if (last_event_IAM != -1) {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_IAM);
        int time_to_iam_timeout = timeout - elapsed_time.count();
        timeout_ms = std::min(timeout_ms, time_to_iam_timeout);
    }

    if (last_event_TRICK != -1) {
        int time_to_trick_timeout = timeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_TRICK).count();
        timeout_ms = std::min(timeout_ms, time_to_trick_timeout);
    }

    return std::max(0, timeout_ms); 
}
std::string Server::busyPlacesToString(){
    std::string busy_places = "";
    if(is_N_connected){
        busy_places += "N";
    }
    if(is_E_connected){
        busy_places += "E";
    }
    if(is_S_connected){
        busy_places += "S";
    }
    if(is_W_connected){
        busy_places += "W";
    }
    return busy_places;
}
void Server::assignClientToPlace(char place, struct pollfd poll_descriptors[11]){
    if(place == 'N'){
        is_N_connected = true;
        poll_descriptors[NREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[NWRITE].fd = poll_descriptors[PWRITE].fd;
    }
    if(place == 'S'){
        is_S_connected = true;
        poll_descriptors[SREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[SWRITE].fd = poll_descriptors[PWRITE].fd;
    }
    if(place == 'W'){
        is_W_connected = true;
        poll_descriptors[WREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[WWRITE].fd = poll_descriptors[PWRITE].fd;
    }
    if(place == 'E'){
        is_E_connected = true;
        poll_descriptors[EREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[EWRITE].fd = poll_descriptors[PWRITE].fd;
    }
}
void Server::initialize_poll_descriptors(int socket_fd, struct pollfd poll_descriptors[11]) {
    poll_descriptors[CONNECTION_SOCKET].fd = socket_fd;
    poll_descriptors[CONNECTION_SOCKET].events = POLLIN;

    for (int i = 1; i <= 5; i++) {
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLIN;
    }

    for (int i = 6; i <= 10; i++) {
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLOUT;
    }
}
void Server::initializeBuffers(char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]) {
    for (int i = 0; i < 11; i++) {
        memset(buffer[i], 0, BUFFER_SIZE);
        buffer_counter[i] = 0;
    }
}
void Server::reset_revents(struct pollfd poll_descriptors[11]) {
    for (int i = 0; i < 11; i++) {
        poll_descriptors[i].revents = 0;
    }
}
void Server::realiseWaitingRoom(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]) {
    poll_descriptors[PWRITE].fd = -1;
    poll_descriptors[PREAD].fd = -1;
    memset(buffer[PREAD], 0, BUFFER_SIZE);
    memset(buffer[PWRITE], 0, BUFFER_SIZE);
    buffer_counter[PREAD] = 0;
    buffer_counter[PWRITE] = 0;
}
void Server::responseToIAM(const std::string& message, struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){
    int IAM_type = validateIAM(message);
    switch(IAM_type){
        case 0:

            if(deals_sent[getPlayerfromChar(message[3])] == true){
                messagesToSendFromWaitingRoom.push_back(getProperDeal(message[3]));
            }
            for(std::string& taken : takenHistory){
                messagesToSendFromWaitingRoom.push_back(taken);
            }
            if(messagesToSendFromWaitingRoom.size() == 0){
                assignClientToPlace(message[3], poll_descriptors);
                realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            }else{
                assignFromWaitingRoom = message[3];
            }
            break;
        case 1:
            messagesToSendFromWaitingRoom.push_back("BUSY" + busyPlacesToString() + "\r\n");
            assignFromWaitingRoom = 'B';
            
            break;
        default:
            close(poll_descriptors[PREAD].fd);
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
    }
}

std::string get_local_address_server(int socket_fd) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    if (getsockname(socket_fd, (struct sockaddr*)&addr, &addr_len) != 0) {
        perror("getsockname failed");
        return "";
    }

    char ip_str[INET6_ADDRSTRLEN];
    std::string local_address;

    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
        local_address = ip_str;
        local_address += ":" + std::to_string(ntohs(s->sin_port));
    } 
    else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));

        // Check if the address is an IPv4-mapped IPv6 address
        if (strncmp(ip_str, "::ffff:", 7) == 0) {
            local_address = std::string(ip_str + 7);  // Skip the "::ffff:" prefix
        } else {
            local_address = ip_str;
        }
        
        local_address += ":" + std::to_string(ntohs(s->sin6_port));
    }
    return local_address;
}

bool Server::areAllPlayersConnected(){
    return is_N_connected && is_S_connected && is_W_connected && is_E_connected;
}

int Server::run(){
    signal(SIGPIPE, sigpipe_handler); 
    int socket_fd = setupServerSocket();
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return 1;
    }
    if(gameplay.getNumberOfDeals() == 0){
        return 0;
    }
    char buffer[11][BUFFER_SIZE] = {0};
    size_t buffer_counter[11] = {0};
    struct pollfd poll_descriptors[11];
    initialize_poll_descriptors(socket_fd, poll_descriptors);
    initializeBuffers(buffer, buffer_counter);
    current_deal = gameplay.getDeal(current_deal_number);
    
    do{

        reset_revents(poll_descriptors);
        int time_to_wait = calculateTimeToWait();
        int poll_status = poll(poll_descriptors, 11, time_to_wait);
        if(poll_status < 0){
            if (errno == EINTR) {
                std::cerr << "interrupted system call\n";
            }
            else {
                syserr("poll");
            }
        }else{
        
            if((poll_descriptors[CONNECTION_SOCKET].revents & POLLIN) && poll_descriptors[PREAD].fd == -1){
                if(manConnectionSocket(socket_fd, poll_descriptors) == 1){
                    return 1;
                }
            }

            if(poll_descriptors[PREAD].fd != -1){ 

                if(manWaitingRoom(poll_descriptors, buffer, buffer_counter) == 1){
                    return 1;
                }
            }
            
            if(areAllPlayersConnected() && player_receiving_deal == -1){
                player_receiving_deal = 0;
            }

            if(areAllPlayersConnected() && player_receiving_deal < 4 && player_receiving_deal >= 0){

                manageDealCards(poll_descriptors, buffer, buffer_counter);
            }

            if(areAllPlayersConnected() && how_many_added_card < 4 && current_trick <= 13 && player_receiving_deal == 4){
                
                std::string message = "TRICK" + std::to_string(current_trick) + lined_cards + "\r\n";
                if(buffer_counter[current_player_receiving_trick + 6] == 0 && (trick_sent[current_player_receiving_trick] == false)){
                    buffer_counter[current_player_receiving_trick + 6] = message.length();
                    strcpy(buffer[current_player_receiving_trick + 6], message.c_str());
                }

                if(poll_descriptors[current_player_receiving_trick + 6].revents & POLLOUT && (trick_sent[current_player_receiving_trick] == false)){
                    ssize_t bytes_sent = send(poll_descriptors[current_player_receiving_trick + 6].fd, buffer[current_player_receiving_trick + 6], buffer_counter[current_player_receiving_trick + 6], 0);
                    if(bytes_sent < 0){
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            close(poll_descriptors[current_player_receiving_trick + 6].fd);
                            poll_descriptors[current_player_receiving_trick + 6].fd = -1;
                            poll_descriptors[current_player_receiving_trick + 1].fd = -1;
                            buffer_counter[current_player_receiving_trick + 6] = 0;
                            memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(current_player_receiving_trick);
                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                    }else{
                        buffer_counter[current_player_receiving_trick + 6] -= bytes_sent;
                        memmove(buffer[current_player_receiving_trick + 6], buffer[current_player_receiving_trick + 6] + bytes_sent, buffer_counter[current_player_receiving_trick + 6]);
                    }
                    

                    if(buffer_counter[current_player_receiving_trick + 6] == 0 && poll_descriptors[current_player_receiving_trick + 6].fd != -1){
                        time_point_TRICK = std::chrono::steady_clock::now();
                        last_event_TRICK = 0;
                        trick_sent[current_player_receiving_trick] = true;
                        raport(get_local_address(poll_descriptors[current_player_receiving_trick + 6].fd), get_local_address(poll_descriptors[current_player_receiving_trick + 6].fd), message);
                    }
                }
                if(trick_sent[current_player_receiving_trick] == true){
                    
                    if(poll_descriptors[current_player_receiving_trick + 1].revents & POLLIN){ 
                        ssize_t bytes_received = read(poll_descriptors[current_player_receiving_trick + 1].fd, buffer[current_player_receiving_trick + 1] + buffer_counter[current_player_receiving_trick + 1], 1);
                        if(bytes_received < 0){
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            disconnectAllPlayers(poll_descriptors);
                            return 1;
                        }
                        if(bytes_received == 0){
                            close(poll_descriptors[current_player_receiving_trick + 1].fd);
                            poll_descriptors[current_player_receiving_trick + 1].fd = -1;
                            poll_descriptors[current_player_receiving_trick + 6].fd = -1;
                            buffer_counter[current_player_receiving_trick + 1] = 0;
                            buffer_counter[current_player_receiving_trick + 6] = 0;
                            memset(buffer[current_player_receiving_trick + 1], 0, BUFFER_SIZE);
                            memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                            trick_sent[current_player_receiving_trick] = false;
                            disconnectPlayer(current_player_receiving_trick);
                            
                        }else{ // got byte
                            char received_char = buffer[current_player_receiving_trick + 1][buffer_counter[current_player_receiving_trick + 1]];
                            buffer_counter[current_player_receiving_trick + 1] += 1;
                            if(received_char == '\n'){
                                if(buffer_counter[current_player_receiving_trick + 1] > 1 && buffer[current_player_receiving_trick + 1][buffer_counter[current_player_receiving_trick + 1] - 2] == '\r'){
                                    last_event_TRICK = -1; 
                                    std::string message(buffer[current_player_receiving_trick + 1], buffer_counter[current_player_receiving_trick + 1]);
                                    
                                    raport(get_server_address(poll_descriptors[current_player_receiving_trick + 1].fd), get_local_address(poll_descriptors[current_player_receiving_trick + 1].fd), message);
                                    if(validateTRICK(message) == true){

                                        int trick_number_from_message = calculateTrickNumFromTRICK(message); 
                                        
                                        std::string card = message.substr(6 + (current_trick >=10 ), message.length() - 8 - (current_trick >= 10));
                                        if(checkIfPlayerCanPlayCard(card) && (trick_number_from_message == current_trick)){

                                            trick_sent[current_player_receiving_trick] = false;
                                            lined_cards += card;
                                            takeCardAwayFromPlayer(card);
                                            memset(buffer[current_player_receiving_trick + 1], 0, BUFFER_SIZE);
                                            memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                                            buffer_counter[current_player_receiving_trick + 1] = 0;
                                            buffer_counter[current_player_receiving_trick + 6] = 0;
                                            how_many_added_card += 1;
                                            current_player_receiving_trick = get_next_player();

                                        }else{

                                            sending_wrong[current_player_receiving_trick] = true;
                                            trick_sent[current_player_receiving_trick] = false;
                                            std::string wrong_message = "WRONG" + std::to_string(current_trick) + "\r\n";
                                            strcpy(buffer[current_player_receiving_trick + 6], wrong_message.c_str());
                                            buffer_counter[current_player_receiving_trick + 6] = wrong_message.length();
                                            memset(buffer[current_player_receiving_trick + 1], 0, BUFFER_SIZE);
                                            buffer_counter[current_player_receiving_trick + 1] = 0;
                                            
                                        }


                                    }else{

                                        close(poll_descriptors[current_player_receiving_trick + 6].fd);
                                        trick_sent[current_player_receiving_trick] = false;
                                        memset(buffer[current_player_receiving_trick + 1], 0, BUFFER_SIZE);
                                        memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                                        buffer_counter[current_player_receiving_trick + 6] = 0;
                                        buffer_counter[current_player_receiving_trick + 1] = 0;
                                        disconnectPlayer(current_player_receiving_trick);


                                    }
                                        
                                } // buffer_counter > 1 && buffer[buffer_counter - 2] == '\r'
                            }// received_char == '\n'
                        }
                    }else{

                        if(last_event_TRICK == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_TRICK).count() > timeout){
                            trick_sent[current_player_receiving_trick] = false;
                            last_event_TRICK = -1;
                        }
                    }

                }    
                
            }

       
            for(int i = 6; i < 10; i++){
                
                if(poll_descriptors[i].revents & POLLOUT && (sending_wrong[i-6] == true) && buffer_counter[i] > 0){

                 
                    ssize_t bytes_sent = send(poll_descriptors[i].fd, buffer[i], buffer_counter[i], 0);
                    if(bytes_sent < 0){
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            close(poll_descriptors[i].fd);
                            poll_descriptors[i].fd = -1;
                            poll_descriptors[i - 5].fd = -1;
                            buffer_counter[i] = 0;
                            memset(buffer[i], 0, BUFFER_SIZE);
                            sending_wrong[i-6] = false;
                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                    }else{
                        buffer_counter[i] -= bytes_sent;
                        memmove(buffer[i], buffer[i] + bytes_sent, buffer_counter[i]);
                    }
                    if(buffer_counter[i] == 0){
                        sending_wrong[i-6] = false;
                        raport(get_local_address(poll_descriptors[i].fd), get_server_address(poll_descriptors[i].fd), "WRONG" + std::to_string(current_trick) + "\r\n");
                    }

                }

            }

         
            if(areAllPlayersConnected() && (how_many_added_card == 4 && player_receiving_deal == 4)){

               manageSendingTaken(poll_descriptors, buffer, buffer_counter);
                
            }

        
            if(areAllPlayersConnected() && current_trick == 14 && player_receiving_deal == 4){

                manageSendingScoreAndTotal(poll_descriptors, buffer, buffer_counter);
            }

     
        if( (messagesToSendFromWaitingRoom.size() > 0 || buffer_counter[PWRITE] > 0)){
           
            if(buffer_counter[PWRITE] == 0){
                if(messagesToSendFromWaitingRoom.size() > 0){
                    std::string message = messagesToSendFromWaitingRoom.front();
                    if (!messagesToSendFromWaitingRoom.empty()) {
                        messagesToSendFromWaitingRoom.erase(messagesToSendFromWaitingRoom.begin());
                    }
                    buffer_counter[PWRITE] = message.length();
                    strcpy(buffer[PWRITE], message.c_str());
                    message_to_raport_from_waiting_room = message;
                }
            }
            
        
            if(poll_descriptors[PWRITE].revents & POLLOUT){
                
              
                ssize_t bytes_sent = send(poll_descriptors[PWRITE].fd, buffer[PWRITE], buffer_counter[PWRITE], 0);
        
                if(bytes_sent < 0){
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            last_event_IAM = -1;
                            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                }else{
                  
                    buffer_counter[PWRITE] -= bytes_sent;
                    memmove(buffer[PWRITE], buffer[PWRITE] + bytes_sent, buffer_counter[PWRITE]);
                }
                
             
                if(buffer_counter[PWRITE] == 0){
                
                    raport(get_local_address(poll_descriptors[PWRITE].fd), get_server_address(poll_descriptors[PWRITE].fd), message_to_raport_from_waiting_room);
                 
                    if(messagesToSendFromWaitingRoom.size() == 0){

                
                        if(assignFromWaitingRoom == 'B'){

                            close(poll_descriptors[PREAD].fd);
                            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            
                        }else{
                            assignClientToPlace(assignFromWaitingRoom, poll_descriptors);
                            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                        }
                    }

                }
            }

        }
                
        }// poll status > 0

         
    }while(finish == false);

    close(poll_descriptors[CONNECTION_SOCKET].fd);
    return 0;
}
void Server::sigpipe_handler(int signum) {
    std::cerr << "Caught SIGPIPE signal: " << signum << std::endl;
    
}

void Server::manageSendingScoreAndTotal(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){
  
                std::string score_message = prepareScoreMessage();
                std::string total_message = prepareTotalMessage();
                std::string message = score_message + total_message;


              
                if(buffer_counter[player_receiving_score_and_total + 6] == 0){
                    buffer_counter[player_receiving_score_and_total + 6] = message.length();
                    strcpy(buffer[player_receiving_score_and_total + 6], message.c_str());
                }

            
                if(poll_descriptors[player_receiving_score_and_total + 6].revents & POLLOUT){ 
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_score_and_total + 6].fd, buffer[player_receiving_score_and_total + 6], buffer_counter[player_receiving_score_and_total + 6], 0);
                    if(bytes_sent < 0){
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            close(poll_descriptors[player_receiving_score_and_total + 6].fd);
                            poll_descriptors[player_receiving_score_and_total + 6].fd = -1;
                            poll_descriptors[player_receiving_score_and_total + 1].fd = -1;
                            buffer_counter[player_receiving_score_and_total + 6] = 0;
                            memset(buffer[player_receiving_score_and_total + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(player_receiving_score_and_total);
                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                    }else{

                
                        buffer_counter[player_receiving_score_and_total + 6] -= bytes_sent;
                        memmove(buffer[player_receiving_score_and_total + 6], buffer[player_receiving_score_and_total + 6] + bytes_sent, buffer_counter[player_receiving_score_and_total + 6]);
                        if(buffer_counter[player_receiving_score_and_total + 6] <= total_message.length() && score_sent == false){
                            raport(get_local_address(poll_descriptors[player_receiving_score_and_total+6].fd), get_server_address(poll_descriptors[player_receiving_score_and_total+6].fd), score_message);
                            score_sent = true;
                        }
                        if(buffer_counter[player_receiving_score_and_total + 6] == 0){
                            
                            raport(get_local_address(poll_descriptors[player_receiving_score_and_total+6].fd), get_server_address(poll_descriptors[player_receiving_score_and_total+6].fd), total_message);
                            score_sent = false;
                        }
                        
                    }
                    
                }

             
                if(buffer_counter[player_receiving_score_and_total + 6] == 0 && poll_descriptors[player_receiving_score_and_total + 6].fd != -1){
                    player_receiving_score_and_total += 1;
                }

            
                if(player_receiving_score_and_total == 4){
                    player_receiving_score_and_total = 0;
                    player_receiving_deal = -1;
                    zeroPointsInDeal();
                    zeroTakenHistory();
                    reset_deals_sent();
                    current_deal_number += 1;
                    current_trick = 1;
                    if(current_deal_number == number_of_deals_to_play){
                        finish = true;
                        disconnectAllPlayers(poll_descriptors);
                    }else{
                        current_deal = gameplay.getDeal(current_deal_number);
                    }
                }
}
void Server::manageSendingTaken(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){

                int who_takes_current_trick = whoTakeTrick(first_player_in_current_trick, "TRICK" + std::to_string(current_trick) + lined_cards + "\r\n");
                std::string message = "TAKEN" + std::to_string(current_trick) + lined_cards + getCharOfPlayer(who_takes_current_trick) + "\r\n";

               
                if(buffer_counter[player_receiving_taken + 6] == 0){
                    buffer_counter[player_receiving_taken + 6] = message.length();
                    strcpy(buffer[player_receiving_taken + 6], message.c_str());
                }

             
                if(poll_descriptors[player_receiving_taken + 6].revents & POLLOUT){ 
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_taken + 6].fd, buffer[player_receiving_taken + 6], buffer_counter[player_receiving_taken + 6], 0);
                    if(bytes_sent < 0){
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            close(poll_descriptors[player_receiving_taken + 6].fd);
                            poll_descriptors[player_receiving_taken + 6].fd = -1;
                            poll_descriptors[player_receiving_taken + 1].fd = -1;
                            buffer_counter[player_receiving_taken + 6] = 0;
                            memset(buffer[player_receiving_taken + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(player_receiving_taken);
                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                    }else{
                     
                        buffer_counter[player_receiving_taken + 6] -= bytes_sent;
                        memmove(buffer[player_receiving_taken + 6], buffer[player_receiving_taken + 6] + bytes_sent, buffer_counter[player_receiving_taken + 6]);
                        if(buffer_counter[player_receiving_taken + 6] == 0){
                            raport(get_local_address(poll_descriptors[player_receiving_taken+6].fd), get_server_address(poll_descriptors[player_receiving_taken+6].fd), message);
                        }
                    }
                }

            
                if(buffer_counter[player_receiving_taken + 6] == 0 && poll_descriptors[player_receiving_taken + 6].fd != -1){
                    player_receiving_taken += 1;
                }
                
            
                if(player_receiving_taken == 4){
                    int how_many_point_in_trick = pointsInTrick(lined_cards, current_deal.getType() - '0');
                    addPointsToPlayer(who_takes_current_trick, how_many_point_in_trick);
                    addTakenToHistory(message);
                    current_trick += 1;
                    first_player_in_current_trick = who_takes_current_trick;
                    current_player_receiving_trick = first_player_in_current_trick;
                    lined_cards = "";
                    how_many_added_card = 0;
                    player_receiving_taken = 0;
                }

}
void Server::manageDealCards(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){
    
                if(buffer_counter[player_receiving_deal + 6] == 0){

                    std::string message = getProperDeal(getCharOfPlayer(player_receiving_deal));
                    buffer_counter[player_receiving_deal + 6] = message.length();
                    strcpy(buffer[player_receiving_deal + 6], message.c_str());

                }

              
                if(poll_descriptors[player_receiving_deal + 6].revents & POLLOUT){ // jest możliwość zapisu
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_deal + 6].fd, buffer[player_receiving_deal + 6], buffer_counter[player_receiving_deal + 6], 0);
               
                    if(bytes_sent < 0){

                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }else if(errno == EPIPE || errno == ECONNRESET){
                            
                            close(poll_descriptors[player_receiving_deal + 6].fd);
                            poll_descriptors[player_receiving_deal + 6].fd = -1;
                            poll_descriptors[player_receiving_deal + 1].fd = -1;
                            buffer_counter[player_receiving_deal + 6] = 0;
                            memset(buffer[player_receiving_deal + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(player_receiving_deal);

                        }else{
                            disconnectAllPlayers(poll_descriptors);
                            close(poll_descriptors[CONNECTION_SOCKET].fd);
                            syserr("write");
                        }
                    
                  
                        }else{

                            buffer_counter[player_receiving_deal + 6] -= bytes_sent;
                            memmove(buffer[player_receiving_deal + 6], buffer[player_receiving_deal + 6] + bytes_sent, buffer_counter[player_receiving_deal + 6]);
                        }
                
                }

                if(buffer_counter[player_receiving_deal + 6] == 0 && poll_descriptors[player_receiving_deal + 6].fd != -1){
                 
                    deals_sent[player_receiving_deal] = true;
                    raport(get_local_address(poll_descriptors[player_receiving_deal+6].fd), get_server_address(poll_descriptors[player_receiving_deal+6].fd), getProperDeal(getCharOfPlayer(player_receiving_deal)));   
                    cards_of_players[player_receiving_deal].addCardsFromCardsString(getDealCardsForPlayer(getCharOfPlayer(player_receiving_deal)));
                    player_receiving_deal += 1;
                }
                
                if(player_receiving_deal == 4){
                    first_player_in_current_trick = getPlayerfromChar(current_deal.getFirstPlayer());
                    current_player_receiving_trick = getPlayerfromChar(current_deal.getFirstPlayer());
                }
}
bool Server::checkIfPlayerCanPlayCard(const std::string& card){
    Card my_card = Card(Card::stringToCard(card));
    if(checkIfPlayerHasCard(card) == false){
        return false;
    }
    if(lined_cards == ""){
        return true;
    }
    Card first_card = Card::stringToCard("2C");

    if(isdigit(lined_cards[1])){
        first_card = Card(Card::stringToCard(lined_cards.substr(0, 3)));
    }else{
        first_card = Card(Card::stringToCard(lined_cards.substr(0, 2)));
    }

    if(first_card.getColor() == my_card.getColor()){
        return true;
    }

    for(Card card : cards_of_players[current_player_receiving_trick].cards){
        if(card.getColor() == first_card.getColor()){
            return false;
        }
    }

    return true;
}
bool Server::checkIfPlayerHasCard(const std::string& card){
   
    Card my_card = Card(Card::stringToCard(card));
    bool has_card = false;
    for(Card card : cards_of_players[current_player_receiving_trick].cards){
        if(card.getColor() == my_card.getColor() && card.getRank() == my_card.getRank()){
            has_card = true;
        }
    }
    return has_card;
}
void Server::disconnectPlayer(int player){
    switch(player){
        case 0:
            is_N_connected = false;
            break;
        case 1:
            is_E_connected = false;
            break;
        case 2:
            is_S_connected = false;
            break;
        case 3:
            is_W_connected = false;
            break;
        default:
            break;
    }
}
void Server::reset_deals_sent(){
    for(int i = 0; i < 4; i++){
        deals_sent[i] = false;
    }
}
std::string Server::getProperDeal(char type){
    switch(type){
        case 'N':
            return "DEAL" + std::string(1, current_deal.getType()) + std::string(1, current_deal.getFirstPlayer()) + current_deal.dealN + "\r\n";
        case 'E':
            return "DEAL" + std::string(1, current_deal.getType()) + std::string(1, current_deal.getFirstPlayer()) + current_deal.dealE + "\r\n";
        case 'S':
            return "DEAL" + std::string(1, current_deal.getType()) + std::string(1, current_deal.getFirstPlayer()) + current_deal.dealS + "\r\n";
        case 'W':
            return "DEAL" + std::string(1, current_deal.getType()) + std::string(1, current_deal.getFirstPlayer()) + current_deal.dealW + "\r\n";
        default:
            return "";
    }
}
std::string Server::getDealCardsForPlayer(char type){
    switch(type){
        case 'N':
            return current_deal.dealN;
        case 'E':
            return current_deal.dealE;
        case 'S':
            return current_deal.dealS;
        case 'W':
            return current_deal.dealW;
        default:
            return "";
    }

}

std::string getClientInfo(int client_fd) {
    struct sockaddr_storage client_address;
    socklen_t client_address_len = sizeof(client_address);

    if (getpeername(client_fd, (struct sockaddr*)&client_address, &client_address_len) < 0) {
        std::cerr << "Błąd podczas pobierania informacji o kliencie\n";
        return "";
    }

    char ip_str[INET6_ADDRSTRLEN]; 
    int client_port;

    if (client_address.ss_family == AF_INET) { // IPv4
        struct sockaddr_in* s = (struct sockaddr_in*)&client_address;
        client_port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
    } else if (client_address.ss_family == AF_INET6) { // IPv6
        struct sockaddr_in6* s = (struct sockaddr_in6*)&client_address;
        client_port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));

       
        if (IN6_IS_ADDR_V4MAPPED(&s->sin6_addr)) {
            // Conversion to IPv4
            struct in_addr ipv4_addr;
            memcpy(&ipv4_addr, &s->sin6_addr.s6_addr[12], sizeof(ipv4_addr));
            inet_ntop(AF_INET, &ipv4_addr, ip_str, sizeof(ip_str));
        }
    } else {
        std::cerr << "Nieznany typ adresu\n";
        return "";
    }

    return std::string(ip_str) + ":" + std::to_string(client_port);
}
int Server::manConnectionSocket(int socket_fd, struct pollfd poll_descriptors[11]){
    
    int waiting_room_fd = accept(socket_fd, NULL, NULL);
    
    if(waiting_room_fd < 0){
        std::cerr << "Błąd podczas akceptowania połączenia\n";
        return 1;
    }
    last_event_IAM = 0;
    time_point_IAM = std::chrono::steady_clock::now();
    poll_descriptors[PREAD].fd = waiting_room_fd;
    poll_descriptors[PWRITE].fd = waiting_room_fd;

    return 0;
}
int Server::manWaitingRoom(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){

    if(poll_descriptors[PREAD].revents & POLLIN){ 

        ssize_t bytes_received = read(poll_descriptors[PREAD].fd, buffer[PREAD] + buffer_counter[PREAD], 1);

        if(bytes_received < 0){
            std::cerr << "Błąd podczas odczytywania wiadomości\n";
            close(poll_descriptors[CONNECTION_SOCKET].fd);
            disconnectAllPlayers(poll_descriptors);
            last_event_IAM = -1;
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            return 1;
        }else if(bytes_received == 0){ 

            close(poll_descriptors[PREAD].fd);
            last_event_IAM = -1;
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                    
        }else{ 
            char received_char = buffer[PREAD][buffer_counter[PREAD]];
            buffer_counter[PREAD] += 1;
            last_event_IAM = -1;
            
            if(received_char == '\n'){
                if(buffer_counter[PREAD] > 1 && buffer[PREAD][buffer_counter[PREAD] - 2] == '\r'){
                    std::string message(buffer[PREAD], buffer_counter[PREAD]);
                    raport(get_server_address(poll_descriptors[PREAD].fd), getClientInfo(poll_descriptors[PREAD].fd), message);
                    responseToIAM(message, poll_descriptors, buffer, buffer_counter);
                }
            }

            if(buffer_counter[PREAD] == MESSAGE_LIMIT){
                buffer[PREAD][buffer_counter[PREAD]] = '\r';
                buffer[PREAD][buffer_counter[PREAD] + 1] = '\n';
                std::string message(buffer[PREAD], buffer_counter[PREAD]);
                raport(get_server_address(poll_descriptors[PREAD].fd), getClientInfo(poll_descriptors[PREAD].fd), message);
                close(poll_descriptors[PREAD].fd);
                realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            }
        }
                
    }else{ 
        if(last_event_IAM == 0 && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_IAM).count() > timeout){
            
            close(poll_descriptors[PREAD].fd);
            last_event_IAM = -1;
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
        }
    }
    return 0;
}
void Server::disconnectAllPlayers(struct pollfd poll_descriptors[11]){
    
    for(int i = 1; i < 6; i++){
        if(poll_descriptors[i].fd != -1){
            close(poll_descriptors[i].fd);
            close(poll_descriptors[i + 5].fd);
        }
    }
}
std::string Server::prepareScoreMessage(){
    std::string message = "SCORE";
    for(int i = 0; i < 4; i++){
        message += getCharOfPlayer(i);
        message += std::to_string(pointsInDeal[i]);
    }
    message += "\r\n";
    return message;
}
std::string Server::prepareTotalMessage(){
    std::string message = "TOTAL";
    for(int i = 0; i < 4; i++){
        message += getCharOfPlayer(i);
        message += std::to_string(pointsInTotal[i]);
    }
    message += "\r\n";
    return message;
}

void Server::zeroTakenHistory(){
    takenHistory.clear();
}

int Server::get_next_player(){
    return (current_player_receiving_trick + 1) % 4;
}

void Server::addTakenToHistory(const std::string& taken){
    takenHistory.push_back(taken);
}

void Server::addPointsToPlayer(int player, int points){
    pointsInDeal[player] += points;
    pointsInTotal[player] += points;
}

void Server::zeroPointsInDeal(){
    for(int i = 0; i < 4; i++){
        pointsInDeal[i] = 0;
    }
}

int Server::getPlayerfromChar(char current_player){
    switch(current_player){
        case 'N':
            return 0;
        case 'E':
            return 1;
        case 'S':
            return 2;
        case 'W':
            return 3;
        default:
            return -1;
    }
}
char Server::getCharOfPlayer(int player){
    switch(player){
        case 0:
            return 'N';
        case 1:
            return 'E';
        case 2:
            return 'S';
        case 3:
            return 'W';
        default:
            return 'Z';
    }
}

void Server::takeCardAwayFromPlayer(const std::string& card){
    switch(current_player_receiving_trick){
        case 0:
            cards_of_players[0].removeCard(Card::stringToCard(card));
            break;
        case 1:
            cards_of_players[1].removeCard(Card::stringToCard(card));
            break;
        case 2:
            cards_of_players[2].removeCard(Card::stringToCard(card));
            break;
        case 3:
            cards_of_players[3].removeCard(Card::stringToCard(card));
            break;
        default:
            break;
    }

}






