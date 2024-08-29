#include <iostream>
#include "server.h"
#include "common.h"
#include <cinttypes>
#include <thread>

// chyba dobrze
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
        takenHistory(), how_many_added_card(0), first_player_in_current_trick(-1), current_deal(gameplay.getDeal(current_deal_number).getType(), gameplay.getDeal(current_deal_number).getFirstPlayer(), gameplay.getDeal(current_deal_number).dealN, gameplay.getDeal(current_deal_number).dealE, gameplay.getDeal(current_deal_number).dealS, gameplay.getDeal(current_deal_number).dealW),
        number_of_deals_to_play(gameplay.getNumberOfDeals()), player_receiving_deal(-1), cards_of_players(4, CardSet()), current_player_receiving_trick(-1),
        player_receiving_taken(0), player_receiving_score_and_total(0)
        {}

Server::~Server(){}

// dobrze
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
// dobrze
int Server::validateIAM(const std::string& message){
    if(message.length() != 6){
        return -1;
    }
    if(message[3] != 'N' && message[3] != 'S' && message[3] != 'W' && message[3] != 'E'){
        return -1;
    }
    // sprawdzam czy ten zawodnik jest akurat podłączony
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
    // prawidłowe IAM na wolne miejsce
    return 0;
}

int Server::setupServerSocket(){

    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return -1;
    }

    // Ustawienie gniazda, aby obsługiwało zarówno IPv6 jak i IPv4
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

    // próbuje bindować gniazdo do konkretnego adresu
    if(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t) sizeof(server_address)) < 0){
        std::cerr << "Błąd podczas bindowania gniazda\n";
        close(socket_fd);
        return -1;
    }

    // przełączam gniazdo w tryb nasłuchiwania
    if(listen(socket_fd, queue_length) < 0){
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe\n";
        close(socket_fd);
        return -1;
    }

    // sprawdzam jaki port został wybrany
    socklen_t length = (socklen_t) sizeof server_address;
    if (getsockname(socket_fd, (struct sockaddr*) &server_address, &length) < 0){
        std::cerr << "Błąd podczas pobierania numeru portu\n";
        close(socket_fd);
        return -1;
    }
    std::cout << "Serwer nasłuchuje na porcie: " << ntohs(server_address.sin6_port) << "\n";

    return socket_fd;
}
bool Server::validateTRICK(const std::string& message){
    //FIXME: implement
    return true;
}
int Server::pointsInTrick(const std::string& trick, int type){
    std::cout << "Punkty w tricku\n";
    std::cout << "Trick: " << trick << "\n";
    std::cout << "Typ: " << type << "\n";
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

// decyduje, który gracz bierze trick
int Server::whoTakeTrick(int first_player, const std::string& trick){
    
    std::cout << "Kto bierze trick\n";
    std::cout << "Trick: " << trick << "\n";
    char first_card_color = get_first_card_color_from_TRICK(trick);
    std::cout << "Kolor pierwszej karty: " << first_card_color << "\n";
    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick.substr(6 + (current_trick >= 10), trick.length() - 8 - (current_trick >= 10)));
    int max_card_number = 0;
    // przechodzę przez wszystkie karty w tricku
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
        return -1;  // Jeśli żadne zdarzenie nie miało miejsca, zwróć -1
    }

    int timeout_ms = timeout;
    if (last_event_IAM != -1) {
        // Oblicz czas pozostały do timeoutu na otrzymanie IAM
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_IAM);
        int time_to_iam_timeout = timeout - elapsed_time.count();
        timeout_ms = std::min(timeout_ms, time_to_iam_timeout);
    }

    if (last_event_TRICK != -1) {
        // Oblicz czas pozostały do timeoutu na otrzymanie TRICK
        int time_to_trick_timeout = timeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_TRICK).count();
        timeout_ms = std::min(timeout_ms, time_to_trick_timeout);
    }

    return std::max(0, timeout_ms);  // Upewnij się, że timeout nie jest ujemny
}
// dobra funkcja
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
void Server::assignClientToPlace(const std::string& message, struct pollfd poll_descriptors[11]){
    if(message[3] == 'N'){
        is_N_connected = true;
        poll_descriptors[NREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[NWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz N podłączony\n";
    }
    if(message[3] == 'S'){
        is_S_connected = true;
        poll_descriptors[SREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[SWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz S podłączony\n";
    }
    if(message[3] == 'W'){
        is_W_connected = true;
        poll_descriptors[WREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[WWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz W podłączony\n";
    }
    if(message[3] == 'E'){
        is_E_connected = true;
        poll_descriptors[EREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[EWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz E podłączony\n";
    }
}
void Server::initialize_poll_descriptors(int socket_fd, struct pollfd poll_descriptors[11]) {
    // Zainicjalizuj gniazdo połączenia
    poll_descriptors[CONNECTION_SOCKET].fd = socket_fd;
    poll_descriptors[CONNECTION_SOCKET].events = POLLIN;

    // Inicjalizacja deskryptorów dla przyjmowania wiadomości od klientów
    for (int i = 1; i <= 5; i++) {
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLIN;
    }

    // Inicjalizacja deskryptorów dla zapisywania do klientów
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
void Server::wypisz_zdarzenia(struct pollfd poll_descriptors[11]){
    std::cout << "Zdarzenia: ";
    std::cout << "CONNECTION_SOCKET: " << poll_descriptors[CONNECTION_SOCKET].revents << " ";
    std::cout << "NREAD: " << poll_descriptors[NREAD].revents << " ";
    std::cout << "EREAD: " << poll_descriptors[EREAD].revents << " ";
    std::cout << "SREAD: " << poll_descriptors[SREAD].revents << " ";
    std::cout << "WREAD: " << poll_descriptors[WREAD].revents << " ";
    std::cout << "PREAD: " << poll_descriptors[PREAD].revents << " ";
    std::cout << "NWRITE: " << poll_descriptors[NWRITE].revents << " ";
    std::cout << "EWRITE: " << poll_descriptors[EWRITE].revents << " ";
    std::cout << "SWRITE: " << poll_descriptors[SWRITE].revents << " ";
    std::cout << "WWRITE: " << poll_descriptors[WWRITE].revents << " ";
    std::cout << "PWRITE: " << poll_descriptors[PWRITE].revents << " ";
    std::cout << "\n";
}
void Server::realiseWaitingRoom(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]) {
    poll_descriptors[PWRITE].fd = -1;
    poll_descriptors[PREAD].fd = -1;
    memset(buffer[PREAD], 0, BUFFER_SIZE);
    memset(buffer[PWRITE], 0, BUFFER_SIZE);
    buffer_counter[PREAD] = 0;
    buffer_counter[PWRITE] = 0;
    last_event_IAM = -1;
}
void Server::responseToIAM(const std::string& message, struct pollfd poll_descriptors[11]){
    int IAM_type = validateIAM(message);
    switch(IAM_type){
        case 0:// wolne miejsce

            // jeśli reconnectuje to dostanie swojego deala
            if(deals_sent[getPlayerfromChar(message[3])] == true){
                send_message(poll_descriptors[PWRITE].fd, getProperDeal(message[3]));
            }
            // dostanie również historie lew
            for(std::string& message : takenHistory){
                send_message(poll_descriptors[PWRITE].fd, message);
            }
            // dopiero takiego zawodnika, który zna stan gry, przypisuje do miejsca
            assignClientToPlace(message, poll_descriptors);
            break;
        case 1:// zajęte miejsce
            send_message(poll_descriptors[PWRITE].fd, "BUSY" + busyPlacesToString() + "\r\n");
            close(poll_descriptors[PREAD].fd);
            break;
        default:// nieprawidłowe IAM
            close(poll_descriptors[PREAD].fd);
    }
}
// dobrze
bool Server::areAllPlayersConnected(){
    return is_N_connected && is_S_connected && is_W_connected && is_E_connected;
}

int Server::run(){
    int socket_fd = setupServerSocket();
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return 1;
    }

    char buffer[11][BUFFER_SIZE] = {0};
    size_t buffer_counter[11] = {0};
    struct pollfd poll_descriptors[11];
    initialize_poll_descriptors(socket_fd, poll_descriptors);
    initializeBuffers(buffer, buffer_counter);
    current_deal = gameplay.getDeal(current_deal_number);
    
    // TODO: implement errors in poll
    //TODO implement timeout
    // TODO implement not readeing at once
    do{

        reset_revents(poll_descriptors);
        int time_to_wait = calculateTimeToWait();
        int poll_status = poll(poll_descriptors, 11, time_to_wait);
        // wypisz_zdarzenia(poll_descriptors);
        // std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if(poll_status < 0){
            std::cerr << "Błąd podczas poll\n";
            return 1;
        }else{
        
            // nowe połączenie, obsługujemy jeśli nie ma nikogo w poczeklani
            if((poll_descriptors[CONNECTION_SOCKET].revents & POLLIN) && poll_descriptors[PREAD].fd == -1){
                // biorę timestamp IM i wkładam do poczekalni
                if(manConnectionSocket(socket_fd, poll_descriptors) == 1){
                    return 1;
                }
            }

            // ktoś jest w poczekalni, obsługujemy zawsze
            if(poll_descriptors[PREAD].fd != -1){ // ktoś jest w poczekalni

                // 
                if(manWaitingRoom(poll_descriptors, buffer, buffer_counter) == 1){
                    return 1;
                }
            }
            
            // jeśli wszyscy są podłączeni i nie było jeszcze dealowania
            if(areAllPlayersConnected() && player_receiving_deal == -1){
                player_receiving_deal = 0;
            }

            // dealuje karty
            if(areAllPlayersConnected() && player_receiving_deal < 4 && player_receiving_deal >= 0){

                // jeśli w bufferze skierowanego do tego gracza nic nie ma
                if(buffer_counter[player_receiving_deal + 6] == 0){

                    std::string message = getProperDeal(getCharOfPlayer(player_receiving_deal));
                    buffer_counter[player_receiving_deal + 6] = message.length();
                    strcpy(buffer[player_receiving_deal + 6], message.c_str());

                }

                // próbuje wysłać deala do tego gracza
                if(poll_descriptors[player_receiving_deal + 6].revents & POLLOUT){ // jest możliwość zapisu
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_deal + 6].fd, buffer[player_receiving_deal + 6], buffer_counter[player_receiving_deal + 6], 0);
                    // wystąpił błąd 
                    if(bytes_sent < 0){

                        if (errno == EPIPE || errno == ECONNRESET) {
                            // Klient się rozłączył lub połączenie zostało zresetowane
                            perror("send failed, disconnecting client");
                            close(poll_descriptors[player_receiving_deal + 6].fd);
                            poll_descriptors[player_receiving_deal + 6].fd = -1;
                            poll_descriptors[player_receiving_deal + 1].fd = -1;
                            buffer_counter[player_receiving_deal + 6] = 0;
                            buffer_counter[player_receiving_deal + 1] = 0;
                            memset(buffer[player_receiving_deal + 6], 0, BUFFER_SIZE);
                            memset(buffer[player_receiving_deal + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(player_receiving_deal);

                        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }
                    
                    // udało się wysłać niezerowe bajty
                        }else{

                            buffer_counter[player_receiving_deal + 6] -= bytes_sent;
                            memmove(buffer[player_receiving_deal + 6], buffer[player_receiving_deal + 6] + bytes_sent, buffer_counter[player_receiving_deal + 6]);
                        }
                    // przesuwam bajty na początek bufora
                }

                if(buffer_counter[player_receiving_deal + 6] == 0 && poll_descriptors[player_receiving_deal + 6].fd != -1){
                    // muszę dodać karty
                    deals_sent[player_receiving_deal] = true;
                    cards_of_players[player_receiving_deal].addCardsFromCardsString(getDealCardsForPlayer(getCharOfPlayer(player_receiving_deal)));
                    player_receiving_deal += 1;
                }
                
                if(player_receiving_deal == 4){
                    first_player_in_current_trick = getPlayerfromChar(current_deal.getFirstPlayer());
                    current_player_receiving_trick = getPlayerfromChar(current_deal.getFirstPlayer());
                }
            }

            

            // wysyłamy tricka do graczy
            if(areAllPlayersConnected() && how_many_added_card < 4 && current_trick <= 13 && player_receiving_deal == 4){
                
                // próba wysłania tricka
                // do aktualnego gracza
                // jeśli jego buffor wysyłkowy jest pusty
                // to umieszam w nim poprawny trick
                if(buffer_counter[current_player_receiving_trick + 6] == 0 && (trick_sent[current_player_receiving_trick] == false)){
                    std::string message = "TRICK" + std::to_string(current_trick) + lined_cards + "\r\n";
                    buffer_counter[current_player_receiving_trick + 6] = message.length();
                    strcpy(buffer[current_player_receiving_trick + 6], message.c_str());
                }

                // próbuje wysłać tricka do gracza
                if(poll_descriptors[current_player_receiving_trick + 6].revents & POLLOUT && (trick_sent[current_player_receiving_trick] == false)){

                    ssize_t bytes_sent = send(poll_descriptors[current_player_receiving_trick + 6].fd, buffer[current_player_receiving_trick + 6], buffer_counter[current_player_receiving_trick + 6], 0);
                    if(bytes_sent < 0){
                        if (errno == EPIPE || errno == ECONNRESET) {
                            // Klient się rozłączył lub połączenie zostało zresetowane
                            perror("send failed, disconnecting client");
                            close(poll_descriptors[current_player_receiving_trick + 6].fd);
                            poll_descriptors[current_player_receiving_trick + 6].fd = -1;
                            poll_descriptors[current_player_receiving_trick + 1].fd = -1;
                            buffer_counter[current_player_receiving_trick + 6] = 0;
                            buffer_counter[current_player_receiving_trick + 1] = 0;
                            memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                            memset(buffer[current_player_receiving_trick + 6], 0, BUFFER_SIZE);
                            disconnectPlayer(current_player_receiving_trick);

                        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            perror("send would block, try again later");
                        }
                    }else{
                        buffer_counter[current_player_receiving_trick + 6] -= bytes_sent;
                        memmove(buffer[current_player_receiving_trick + 6], buffer[current_player_receiving_trick + 6] + bytes_sent, buffer_counter[current_player_receiving_trick + 6]);
                    }
                    
                    // przesuwam bajty na początek bufora
                    if(buffer_counter[current_player_receiving_trick + 6] == 0 && poll_descriptors[current_player_receiving_trick + 6].fd != -1){
                        time_point_TRICK = std::chrono::steady_clock::now();
                        last_event_TRICK = 0;
                        trick_sent[current_player_receiving_trick] = true;
                    }
                }
                // jeśli wysłałem tricka, to czekam na odpowiedź od tego gracza
                if(trick_sent[current_player_receiving_trick] == true){
                    
                    if(poll_descriptors[current_player_receiving_trick + 1].revents & POLLIN){ // coś przyszło od tego gracza
                            // odczytuje wiadomość od tego gracza bajt po bajcie
                        ssize_t bytes_received = read(poll_descriptors[current_player_receiving_trick + 1].fd, buffer[current_player_receiving_trick + 1] + buffer_counter[current_player_receiving_trick + 1], 1);
                        if(bytes_received < 0){
                            std::cerr << "Błąd podczas odczytywania wiadomości\n";
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
                            
                        }else{ // dostałem bajt
                            char received_char = buffer[current_player_receiving_trick + 1][buffer_counter[current_player_receiving_trick + 1]];
                            buffer_counter[current_player_receiving_trick + 1] += 1;
                            if(received_char == '\n'){
                                last_event_TRICK = -1;
                                if(buffer_counter[current_player_receiving_trick + 1] > 1 && buffer[current_player_receiving_trick + 1][buffer_counter[current_player_receiving_trick + 1] - 2] == '\r'){
                                    std::string message(buffer[current_player_receiving_trick + 1], buffer_counter[current_player_receiving_trick + 1]);
                                        
                                    std::string card = message.substr(6 + (current_trick >=10 ), message.length() - 8 - (current_trick >= 10));
                                    if(checkIfPlayerCanPlayCard(card)){

                                        trick_sent[current_player_receiving_trick] = false;
                                        lined_cards += card;
                                        takeCardAwayFromPlayer(card);
                                        memset(buffer[current_player_receiving_trick + 1], 0, BUFFER_SIZE);
                                        buffer_counter[current_player_receiving_trick + 1] = 0;
                                        how_many_added_card += 1;
                                        current_player_receiving_trick = get_next_player();
                                        
                                    }else{
                                        send_message(poll_descriptors[current_player_receiving_trick + 6].fd, "WRONG" + std::to_string(current_trick) +"\r\n");
                                        trick_sent[current_player_receiving_trick] = false;
                                    }
                                        
                                } // buffer_counter > 1 && buffer[buffer_counter - 2] == '\r'
                            }// received_char == '\n'
                        }
                    }else{// nic nie przyszło od tego gracza

                            // jeśli został przekroczony timeout
                        if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_TRICK).count() > timeout){
                            trick_sent[current_player_receiving_trick] = false;
                            last_event_TRICK = -1;
                            std::cout << "Timeout na Trick\n";
                        }
                    }

                }    
                
            }

            // wysyłam taken
            if(areAllPlayersConnected() && (how_many_added_card == 4 && player_receiving_deal == 4)){

                // próbuje wysłać taken do wszystkich zaczynając od pierwszego gracza
                std::cout << "trick message to send: " << "TRICK" + std::to_string(current_trick) + lined_cards + "\r\n";
                int who_takes_current_trick = whoTakeTrick(first_player_in_current_trick, "TRICK" + std::to_string(current_trick) + lined_cards + "\r\n");
                std::cout << "who takes current trick: " << who_takes_current_trick << "\n";
                    std::cout << "Kto bierze trick: " << getCharOfPlayer(who_takes_current_trick) << "\n";
                    std::string message = "TAKEN" + std::to_string(current_trick) + lined_cards + getCharOfPlayer(who_takes_current_trick) + "\r\n";

                // jeśli w bufferze skierowanego do tego gracza nic nie ma
                if(buffer_counter[player_receiving_taken + 6] == 0){
                    buffer_counter[player_receiving_taken + 6] = message.length();
                    strcpy(buffer[player_receiving_taken + 6], message.c_str());
                }

                // próbuje wystawić taken do gracza
                if(poll_descriptors[player_receiving_taken + 6].revents & POLLOUT){ // jeśli jest możliwość zapisu
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_taken + 6].fd, buffer[player_receiving_taken + 6], buffer_counter[player_receiving_taken + 6], 0);
                    if(bytes_sent < 0){
                        std::cerr << "Błąd podczas wysyłania taken\n";
                        close(poll_descriptors[player_receiving_taken + 6].fd);
                        return 1;
                    }
                    // ziomeczek się rozłączył
                    if(bytes_sent == 0){
                        close(poll_descriptors[player_receiving_taken + 6].fd);
                        poll_descriptors[player_receiving_taken + 6].fd = -1;
                        poll_descriptors[player_receiving_taken + 1].fd = -1;
                        buffer_counter[player_receiving_taken + 6] = 0;
                        memset(buffer[player_receiving_taken + 6], 0, BUFFER_SIZE);
                        disconnectPlayer(player_receiving_taken);
                    }
                    // przesuwam bajty na początek bufora
                    buffer_counter[player_receiving_taken + 6] -= bytes_sent;
                    memmove(buffer[player_receiving_taken + 6], buffer[player_receiving_taken + 6] + bytes_sent, buffer_counter[player_receiving_taken + 6]);
                }

                // udało się wysłać taken do gracza
                if(buffer_counter[player_receiving_taken + 6] == 0 && poll_descriptors[player_receiving_taken + 6].fd != -1){
                    player_receiving_taken += 1;
                }
                
                // po wysłaniu taken do wszystkich
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

            // wysyłam score i total
            if(areAllPlayersConnected() && current_trick == 14 && player_receiving_deal == 4){

                // przygotuj score i total
                std::string score_message = prepareScoreMessage();
                std::string total_message = prepareTotalMessage();
                std::string message = score_message + total_message;

                // próbuje wysłać score i total do gracza
                if(buffer_counter[player_receiving_score_and_total + 6] == 0){
                    buffer_counter[player_receiving_score_and_total + 6] = message.length();
                    strcpy(buffer[player_receiving_score_and_total + 6], message.c_str());
                }

                // próbuje wysłać score i total do gracza
                if(poll_descriptors[player_receiving_score_and_total + 6].revents & POLLOUT){ // jeśli jest możliwość zapisu
                    ssize_t bytes_sent = send(poll_descriptors[player_receiving_score_and_total + 6].fd, buffer[player_receiving_score_and_total + 6], buffer_counter[player_receiving_score_and_total + 6], 0);
                    if(bytes_sent < 0){
                        std::cerr << "Błąd podczas wysyłania score i total\n";
                        close(poll_descriptors[player_receiving_score_and_total + 6].fd);
                        return 1;
                    }
                    // ziomeczek się rozłączył
                    if(bytes_sent == 0){
                        close(poll_descriptors[player_receiving_score_and_total + 6].fd);
                        poll_descriptors[player_receiving_score_and_total + 6].fd = -1;
                        poll_descriptors[player_receiving_score_and_total + 1].fd = -1;
                        buffer_counter[player_receiving_score_and_total + 6] = 0;
                        memset(buffer[player_receiving_score_and_total + 6], 0, BUFFER_SIZE);
                        disconnectPlayer(player_receiving_score_and_total);
                    }
                    // przesuwam bajty na początek bufora
                    buffer_counter[player_receiving_score_and_total + 6] -= bytes_sent;
                    memmove(buffer[player_receiving_score_and_total + 6], buffer[player_receiving_score_and_total + 6] + bytes_sent, buffer_counter[player_receiving_score_and_total + 6]);
                }

                // udało się wysłać score i total do gracza
                if(buffer_counter[player_receiving_score_and_total + 6] == 0 && poll_descriptors[player_receiving_score_and_total + 6].fd != -1){
                    player_receiving_score_and_total += 1;
                }

                // jeśli wysłałem wszystkie score i total
                if(player_receiving_score_and_total == 4){
                    player_receiving_score_and_total = 0;
                    player_receiving_deal = -1;
                    zeroPointsInDeal();
                    zeroTakenHistory();
                    reset_deals_sent();
                    current_deal_number += 1;
                    current_trick = 1;
                    // zaznacz, że jest to czas na rozdanie
                    if(current_deal_number == number_of_deals_to_play){
                        finish = true;
                        disconnectAllPlayers(poll_descriptors);
                    }else{
                        current_deal = gameplay.getDeal(current_deal_number);
                    }
                }
            }
                
        }// poll status > 0

         
    }while(finish == false);

    close(poll_descriptors[CONNECTION_SOCKET].fd);
    return 0;
}

bool Server::checkIfPlayerCanPlayCard(const std::string& card){
    // sprawdzam czy gracz ma taką kartę
    Card my_card = Card(Card::stringToCard(card));
    // std::cout << "Karta gracza: " << my_card.toString() << "\n";
    if(checkIfPlayerHasCard(card) == false){
        return false;
    }
    // std::cout << "czyli mam kartę\n";
    if(lined_cards == ""){
        return true;
    }
    // std::cout << "lined cards: " << lined_cards << "\n";
    Card first_card = Card::stringToCard("2C");

    if(isdigit(lined_cards[1])){
        first_card = Card(Card::stringToCard(lined_cards.substr(0, 3)));
    }else{
        first_card = Card(Card::stringToCard(lined_cards.substr(0, 2)));
    }
    std::cout << "Karta utworzona z lined cards: " << first_card.toString() << "\n";

    if(first_card.getColor() == my_card.getColor()){
        return true;
    }

    // jeśli znajdę w śród jego kart kartę o kolorze pierwszej karty
    for(Card card : cards_of_players[current_player_receiving_trick].cards){
        // std::cout << "Karta w ręku: " << card.toString() << "\n";
        // std::cout << "Karta pierwsza: " << first_card.toString() << "\n";
        if(card.getColor() == first_card.getColor()){
            return false;
        }
    }

    // nie 
    return true;
}

bool Server::checkIfPlayerHasCard(const std::string& card){
    // sprawdzam czy gracz ma taką kartę
    Card my_card = Card(Card::stringToCard(card));
    // std::cout << "Moja karta: " << my_card.toString() << "\n";
    bool has_card = false;
    // przeszukuję karty gracza
    for(Card card : cards_of_players[current_player_receiving_trick].cards){
        // std::cout << "Karta w ręku: " << card.toString() << "\n";
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

int Server::manConnectionSocket(int socket_fd, struct pollfd poll_descriptors[11]){
    
    int waiting_room_fd = accept(socket_fd, NULL, NULL);
    if(waiting_room_fd < 0){
        std::cerr << "Błąd podczas akceptowania połączenia\n";
        return 1;
    }
    std::cout << "Nowe połączenie\n";
    last_event_IAM = 0;
    time_point_IAM = std::chrono::steady_clock::now();
    poll_descriptors[PREAD].fd = waiting_room_fd;
    poll_descriptors[PWRITE].fd = waiting_room_fd;

    return 0;
}
int Server::manWaitingRoom(struct pollfd poll_descriptors[11], char buffer[11][BUFFER_SIZE], size_t buffer_counter[11]){

    if(poll_descriptors[PREAD].revents & POLLIN){ // są dane do odczytu

        ssize_t bytes_received = read(poll_descriptors[PREAD].fd, buffer[PREAD] + buffer_counter[PREAD], 1);

        if(bytes_received < 0){ // błąd podczas odczytywania
            std::cerr << "Błąd podczas odczytywania wiadomości\n";
            close(poll_descriptors[CONNECTION_SOCKET].fd);
            disconnectAllPlayers(poll_descriptors);
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            return 1;
        }else if(bytes_received == 0){ // client się rozłączył

            close(poll_descriptors[PREAD].fd);
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                    
        }else{ // otrzymałem bajt
            char received_char = buffer[PREAD][buffer_counter[PREAD]];
            buffer_counter[PREAD] += 1;
            
            if(received_char == '\n'){
                if(buffer_counter[PREAD] > 1 && buffer[PREAD][buffer_counter[PREAD] - 2] == '\r'){
                    std::string message(buffer[PREAD], buffer_counter[PREAD]);
                    responseToIAM(message, poll_descriptors);
                    realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                }
            }

            if(buffer_counter[PREAD] == MESSAGE_LIMIT){
                buffer[PREAD][buffer_counter[PREAD]] = '\r';
                buffer[PREAD][buffer_counter[PREAD] + 1] = '\n';
                //FIXME:: add raport here
                close(poll_descriptors[PREAD].fd);
                realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            }
        }
                
    }else{ // sprawdzam, czy nie został przekroczony limit czasu na przesłanie IAM
        if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_IAM).count() > timeout){
            // rozłączam się z takim klientem
            close(poll_descriptors[PREAD].fd);
            // czyszę poczekalnię
            realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
            std::cout << "Timeout na IAM\n";
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
void Server::wypisz_punkty(){
    std::cout << "Punkty w rozdaniu: ";
    for(int i = 0; i < 4; i++){
        std::cout << pointsInDeal[i] << " ";
    }
    std::cout << "\n";
    std::cout << "Punkty w sumie: ";
    for(int i = 0; i < 4; i++){
        std::cout << pointsInTotal[i] << " ";
    }
    std::cout << "\n";
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






