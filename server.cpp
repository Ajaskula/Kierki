#include <iostream>
#include "server.h"
#include "common.h"
#include <cinttypes>
#include <thread>


int Server::parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout){
    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-p" && i + 1 < argc){
            port = std::stoi(argv[i + 1]);
            i++;
        } else if(arg == "-f" && i + 1 < argc){

            file = argv[i + 1];
            i++;
            // std::cout << "Plik: " << file << "\n";
        }else if(arg == "-t" && i + 1 < argc){
            timeout = std::stoi(argv[i+1]);
            if(timeout < 1){
                std::cerr << "Timeout musi być większy od 1\n";
                return 1;
            }
            i++;
            // std::cout << "Timeout: " << timeout << "\n";
        }else{
            std::cerr << "Nieznany parametr: " << arg << "\n";
            // tutaj jeszcze się zastanowić czy na pewno kończyć program
            return 1;
        }
    }

    // sprawdza czy został podany plik
    if(file.empty()){
        std::cerr << "Błąd parametr -f jest wymagany\n";
        return 1;
    }

    if(port == 0){
        std::cout << "Numer portu zostanie wybrany domyślnie przez funkcję bind.\n";
    }

    std::cout << "Plik: " << file << "\n";
    std::cout << "Timeout: " << timeout << "\n";
    std::cout << "port: " << port << "\n";
    return 0;
}
Server::Server(uint16_t port, const std::string& file, int timeout)
        : port(port), file(file), timeout(timeout * 1000), connected_players(0), gameplay(file), 
        queue_length(10), is_E_connected(false), is_N_connected(false), is_S_connected(false), is_W_connected(false), current_trick(1),
        last_event_IAM(-1), last_event_TRICK(-1), lined_cards(""), finish(false), time_point_IAM(), time_point_TRICK(), current_deal_number(0),
        number_of_deals_to_play(gameplay.getNumberOfDeals()), cards_of_player_N(), cards_of_player_S(), cards_of_player_W(), cards_of_player_E()
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
// funkcja sprawdzająca poprawność wiadomości IAM
int Server::validateIAM(const std::string& message){
    std::cout << "Wiadomość IAM: " << message << "\n";
    std::cout << "Trzecia litera wiadomości: " << message[3] << "\n";
    if(message.length() != 6){
        return -1;
    }
    if(message[3] != 'N' && message[3] != 'S' && message[3] != 'W' && message[3] != 'E'){
        return -1;
    }
    std::cout << is_N_connected << " " << is_S_connected << " " << is_W_connected << " " << is_E_connected << "\n";
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
    std::cout << "Próbuje utworzyć gniazdo\n";

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
int Server::calculateNumOfHeartsInTrick(const std::string& trick){
    //FIXME:: implement
    return 0;
}
int Server::calculateNumOfQueensInTrick(const std::string& trick){
    // FIXME:: implement
    return 0;
}
int Server::calculateNumOfManInTrick(const std::string& trick){
    //FIXME:: implement
    return 0;
}
int Server::checkIfKingOfHeartsInTrick(const std::string& trick){
    // FIXME:: implement
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
std::string Server::busyPlacesToString(){
    std::string busy_places = "";
    if(is_N_connected){
        busy_places += "N";
    }
    if(is_S_connected){
        busy_places += "E";
    }
    if(is_W_connected){
        busy_places += "S";
    }
    if(is_E_connected){
        busy_places += "W";
    }
    return busy_places;
}
void Server::assignClientToPlace(const std::string& message, struct pollfd poll_descriptors[11]){
    if(message[3] == 'N'){
        // std::cout << "Ustawiam is_N_connected na true\n";
        is_N_connected = true;
        // std::cout << "is_N_connected: " << is_N_connected << "\n";
        poll_descriptors[NREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[NWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz N podłączony\n";
    }
    if(message[3] == 'S'){
        // std::cout << "Ustawiam is_S_connected na true\n";
        is_S_connected = true;
        poll_descriptors[SREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[SWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz S podłączony\n";
    }
    if(message[3] == 'W'){
        // std::cout << "Ustawiam is_W_connected na true\n";
        is_W_connected = true;
        poll_descriptors[WREAD].fd = poll_descriptors[PREAD].fd;
        poll_descriptors[WWRITE].fd = poll_descriptors[PWRITE].fd;
        std::cout << "Gracz W podłączony\n";
    }
    if(message[3] == 'E'){
        // std::cout << "Ustawiam is_E_connected na true\n";
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
            assignClientToPlace(message, poll_descriptors);
            // std::cout << "is_N_connected po wywowłaniu assign: " << is_N_connected << "\n";
            break;
        case 1:// zajęte miejsce
            send_message(poll_descriptors[PWRITE].fd, "BUSY" + busyPlacesToString() + "\r\n");
            close(poll_descriptors[PREAD].fd);
            break;
        default:// nieprawidłowe IAM
            close(poll_descriptors[PREAD].fd);
    }
}

bool Server::areAllPlayersConnected(){
    return is_N_connected && is_S_connected && is_W_connected && is_E_connected;
}

int Server::run(){

    int socket_fd = setupServerSocket();
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return 1;
    }
    // bierząca rozgrywka
    bool time_to_deal = true;
    Deal current_deal = gameplay.getDeal(current_deal_number);
    std::cout << "Serwer gotowy do przyjmowania połączeń\n";
    // numery deskryptorów
    // O N E S W P N E S W P
    // 0 1 2 3 4 5 6 7 8 9 10 
    char buffer[11][BUFFER_SIZE] = {0};
    size_t buffer_counter[11];
    struct pollfd poll_descriptors[11];
    initialize_poll_descriptors(socket_fd, poll_descriptors);
    initializeBuffers(buffer, buffer_counter);
    
    // TODO: implement errors in poll
    bool finish = false;
     while(finish == false){
        reset_revents(poll_descriptors);
        //TODO implement timeout
        // TODO implement not readeing at once
        int time_to_wait = calculateTimeToWait();
        int poll_status = poll(poll_descriptors, 11, time_to_wait);
        wypisz_zdarzenia(poll_descriptors);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if(poll_status < 0){
            std::cerr << "Błąd podczas poll\n";
            return 1;
        }else{
        // jeśli przyszło nowe połączenie, a nie ma nikogo w poczekalni
            if((poll_descriptors[CONNECTION_SOCKET].revents & POLLIN) && poll_descriptors[PREAD].fd == -1){
            
                // najpierw akceptuje połączenie z tym klientem
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
            }

            if(poll_descriptors[PREAD].fd != -1){ // ktoś jest w poczekalni

                if(poll_descriptors[PREAD].revents & POLLIN){ // są dane do odczytu

                    ssize_t bytes_received = read(poll_descriptors[PREAD].fd, buffer[PREAD] + buffer_counter[PREAD], 1);
                    if(bytes_received < 0){
                        std::cerr << "Błąd podczas odczytywania wiadomości\n";
                        close(poll_descriptors[PREAD].fd);
                        realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                        return 1;
                    }else if(bytes_received == 0){ // client się rozłączył

                        close(poll_descriptors[PREAD].fd);
                        realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                    
                    }else{
                        char received_char = buffer[PREAD][buffer_counter[PREAD]];
                        buffer_counter[PREAD] += 1;
                        // jeśli odebrany znak to znak nowej linii
                        if(received_char == '\n'){
                            if(buffer_counter[PREAD] > 1 && buffer[PREAD][buffer_counter[PREAD] - 2] == '\r'){
                                // TODO::implement long messages without \r\n
                                std::string message(buffer[PREAD], buffer_counter[PREAD]);
                                responseToIAM(message, poll_descriptors);
                                realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                            }
                        }
                    }
                
                }else{ // nie przesłał żadnej wiadomości, ale jest w poczekalni
                    if(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time_point_IAM).count() > timeout){
                        close(poll_descriptors[PREAD].fd);
                        realiseWaitingRoom(poll_descriptors, buffer, buffer_counter);
                        std::cout << "Timeout na IAM\n";
                    }
                }

            }
            
            // jeśli mamy podłączonych wszystkich graczy, to możemy albo dealować karty, albo czekać na TRICK
            // wysyłam deal do wszystkich graczy
            if(areAllPlayersConnected() && time_to_deal){
                
                std::string message = "DEAL" + std::string(1, current_deal.getType()) + std::string(1, current_deal.getFirstPlayer());
                send_message(poll_descriptors[1].fd, message + current_deal.dealN + "\r\n");
                // dodaj karty do ręki gracza
                cards_of_player_N.addCardsFromCardsString(current_deal.dealN);
                send_message(poll_descriptors[2].fd, message + current_deal.dealE + "\r\n");
                // dodaj karty do ręki gracza
                cards_of_player_E.addCardsFromCardsString(current_deal.dealE);
                send_message(poll_descriptors[3].fd, message + current_deal.dealS + "\r\n");
                cards_of_player_S.addCardsFromCardsString(current_deal.dealS);
                send_message(poll_descriptors[4].fd, message + current_deal.dealW + "\r\n");
                cards_of_player_W.addCardsFromCardsString(current_deal.dealW);
                time_to_deal = false;
                current_player = getPlayerfromChar(current_deal.getFirstPlayer());
            }

            // wypisuje karty wszystkich graczy
            if(areAllPlayersConnected()){
                std::cout << "Karty gracza N: " << cards_of_player_N.getCardsOnHand() << "\n";
                std::cout << "Karty gracza E: " << cards_of_player_E.getCardsOnHand() << "\n";
                std::cout << "Karty gracza S: " << cards_of_player_S.getCardsOnHand() << "\n";
                std::cout << "Karty gracza W: " << cards_of_player_W.getCardsOnHand() << "\n";
            }

            
            // tricking era
            if(areAllPlayersConnected()){
                
                // teraz proszę każdego z graczy o trick
                std::string message = "TRICK" + std::to_string(current_trick);
                for(int i = 0; i < 4; i++){
                    // proszę o trick każdego z kolejnych graczy
                    send_message(poll_descriptors[current_player + 1].fd, message + "\r\n");

                    // czekam na odpowiedź
                    if(0){

                    }

                }
                current_trick += 1;
                
            }

        } // poll_status >= 0
    }// while

    // zamykam gniazdo, rozgrywka zakończona prawidłowo
    close(socket_fd);
    return 0;
}


int Server::get_next_player(){
    return (current_player + 1) % 4;
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





