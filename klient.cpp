#include "klient.h"

Klient::Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot)
        : host(host), port(port), IPv4(IPv4), IPv6(IPv6), position(position), isBot(isBot), cardSet(), tricks_taken(), socket_fd(0),
        wait_DEAL(true), wait_TOTAL(true), wait_SCORE(true), wait_first_TRICK(true), got_TRICK(false), current_trick(1)
        {}
Klient::~Klient(){}

int Klient::parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot){
    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-h" && i + 1 < argc){
            host = argv[i + 1];
            i++;
        }else if(arg == "-p" && i + 1 < argc){
            port = std::stoi(argv[i + 1]);
            i++;
        }else if(arg == "-4"){
            if(IPv6 == false && IPv4 == false){
                IPv4 = true;
            }
        }else if(arg == "-6"){
            if(IPv4 == false && IPv6 == false){
                IPv6 = true;
            }
        }else if(arg == "-N"){
            position = 'N';
        }else if(arg == "-S"){
            position = 'S';
        }else if(arg == "-E"){
            position = 'E';
        }else if(arg == "-W"){
            position = 'W';
        }else if(arg == "-a"){
            isBot = true;
        }else{
            std::cerr << "Nieznany parametr: " << arg << "\n";
            return 1;
        }

    }
    if(IPv4 && IPv6){
        std::cerr << "Nie można jednocześnie używać IPv4 i IPv6\n";
        return 1;
    }
    if('\0' == position){
        std::cerr << "Nie podano pozycji\n";
        return 1;
    }
    if(0 == port){
        std::cerr << "Obowiązkowy numer portu nie został podany\n";
        return 1;
    }
    if(host.empty()){
        std::cerr << "Obowiązkowy parametr -h nie został podany\n";
        return 1;
    }
    std::cout << "Host: " << host << "\n";
    std::cout << "Port: " << port << "\n";
    std::cout << "IPv4: " << IPv4 << "\n";
    std::cout << "IPv6: " << IPv6 << "\n";
    std::cout << "Pozycja: " << position << "\n";
    std::cout << "Bot: " << isBot << "\n";
    
    return 0;
}
bool Klient::isStringValidHandDealed(const std::string& hand) {
    // zbiór do przechowywanie unikalnych kart
    std::set<std::string> unique_cards; // Zbiór do przechowywania unikalnych kart

    // przetwarzanie karty po karcie
    size_t i = 0;
    size_t hand_length = hand.length();
    int card_count = 0;

    // dopóki nie przejdę przez cały string
    while (i < hand_length) {
        std::string card;
        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            card = hand.substr(i, 3); // Karta to "10X"
            i += 3;
        } else if (i + 1 < hand_length) {
            card = hand.substr(i, 2); // Karta to "VX"
            i += 2;
        } else {
            return false; // Niekompletna karta na końcu stringa
        }
        if (!Card::isStringValidCard(card)) {
            return false; // Znaleziono niepoprawną kartę
        }
        if (!unique_cards.insert(card).second) {
            return false; // Znaleziono duplikat
        }
        card_count++;
    }
    return card_count == 13; // Sprawdzamy, czy przetworzono dokładnie 13 kart
}

bool Klient::validateBUSY(const std::string& message){
    if(message.length() > 10 || message.length() < 7){
        return false;
    }
    if(message.substr(0, 4) != "BUSY"){
        return false;
    }

    for(std::string::size_type i = 4; i < message.length() - 2; i++){
        if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
            return false;
        }
    }
    if(message.substr(4).find(position) == std::string::npos){
        return false;
    }
    // każdy ze znaków N, S , E, W może wystąpić tylko raz
    for(std::string::size_type i = 4; i < message.length(); i++){
        if(message.substr(4).find(message[i]) != message.substr(4).rfind(message[i])){
            return false;
        }
    }
    return true;
}
bool Klient::validateDEAL(const std::string& message){
    
    // zła długość wiadomości
    if(message.length() < 34 || message.length() > 38){
        return false;
    }
    if(message.substr(0, 4) != "DEAL"){
        return false;
    }
    // sprawdzenie typu rozdania
    if (std::isdigit(message[4])) {
        int value = message[4] - '0'; // Convert char to int
        
        // Check if the value is within the desired range
        if (value < 1 || value > 7) {
            return false;
        }
    } else {
        return false;
    }

    // sprawdzenie pozycji klienta wychodzącego jako pierwszy
    if(message[5] != 'N' && message[5] != 'S' && message[5] != 'E' && message[5] != 'W'){
        return false;
    }

    // sprawdzenie czy lista kart jest poprawna
    if(isStringValidHandDealed(message.substr(6, (message.length() - 8))) == false){
        return false;
    }
 
    return true;
}
bool Klient::validateTRICK(const std::string& message){
    if(message.length() < 8 || message.length() > 18){
        
        return false;
    }
    if(message.substr(0, 5) != "TRICK"){
        
        return false;
    }

    // sprawdzam czy numer lewy jest jedno czy dwucyfrowy
    int trick_number = 0;
    if(( message[7] >= '1' && message[7] <= '9') || message[7] == 'J' || message[7] == 'Q' || message[7] == 'K' || message[7] == 'A' || message[7] == '\r'){
        
        if(isdigit(message[5]) && isdigit(message[6])){
            trick_number = 10 * (message[5] - '0');
            trick_number += message[6] - '0';

        }else{
            return false;
        }

    // nie 
    }else{
        if(isdigit(message[5])){
            trick_number = message[5] - '0';
        }else{
            return false;
        }
    }

    if(trick_number != current_trick){
        return false;
    }

    // sprawdzenie listy kart, czy zawiera poprawne karty
    if(!is_string_correct_card_list(message.substr(7 - (trick_number >= 10), message.length() - 8 - (trick_number >= 10)))){
        return false;
    }
    std::vector<std::string> cards_vec = Card::extractCardsVectorFromCardsStringStr(message.substr(7 - (trick_number >= 10), message.length() - 8 - (trick_number >= 10)));
    
    if(cards_vec.size() >= 4){
        return false;
    }

    // muszę jeszcze sprawdzić, czy nie posiadam takiej karty jak te wyłożone na stole
    for(std::vector<std::string>::size_type i = 0; i < cards_vec.size(); i++){
        if(cardSet.isCardInSet(Card::stringToCard(cards_vec[i]))){
            return false;
        }
    }
    return true;
}
bool Klient::validateTAKEN(const std::string& message){
    // sprawdzenie długości
    if(message.length() < 17 || message.length() > 22){
        return false;
    }

    // sprawdzenie pierwszego słowa
    if(message.substr(0, 5) != "TAKEN"){
        return false;
    }

    // sprawdzenie numeru lewy
    int trick_number = 0;
    if(( message[7] >= '1' && message[7] <= '9')|| message[7] == 'J' || message[7] == 'Q' || message[7] == 'K' || message[7] == 'A'){
        
        if(isdigit(message[5]) && isdigit(message[6])){
            trick_number = 10 * (message[5] - '0');
            trick_number += message[6] - '0';

        }else{
            return false;
        }
    // nie 
    }else{
        if(isdigit(message[5])){
            trick_number = message[5] - '0';
        }else{
            return false;
        }
    }

    if(trick_number != current_trick){
        return false;
    }

    // czy jest poprawną listą kart
    if(!is_string_correct_card_list(message.substr(6 + (trick_number >= 10), message.length() - 9 - (trick_number >= 10)))){
        return false;
    }
    std::vector<std::string> cards_vec = Card::extractCardsVectorFromCardsStringStr(message.substr(6 + (trick_number >= 10), message.length() - 9 - (trick_number >= 10)));
    int in_my_hand = 0;
    for(std::vector<std::string>::size_type i = 0; i < cards_vec.size(); i++){
        if(cardSet.isCardInSet(Card::stringToCard(cards_vec[i]))){
            in_my_hand++;
        }
    }
    if(in_my_hand != 1){
        return false;
    }

    if(message[message.length() - 3] != 'N' && message[message.length() - 3] != 'S' && message[message.length() - 3] != 'E' && message[message.length() - 3] != 'W'){
        return false;
    }
    
    return true;
}
bool Klient::validateWRONG(const std::string& message){
    // sprawdzenie długości
    if(message.length() < 8 || message.length() > 9){
        return false;
    }
    if(message.substr(0, 5) != "WRONG"){
        return false;
    }
    // sprawdzenie numeru lewy
    if(!(message[5] == '1' && message[6] <= '3' && message[6] >= '0') && !(message[5] >= '1' && message[6] <= '9' && !isdigit(message[6]))){
        return false;
    }
    int trick_number = 0;
    if(message.length() == 8){
        trick_number = message[5] - '0';
    }else{
        trick_number = 10 * (message[5] - '0');
        trick_number += message[6] - '0';
    }

    if(trick_number != current_trick){
        return false;
    }

    return true;
}
bool Klient::validateSCORE(const std::string& message) {
    const std::regex pattern(R"(SCORE[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)\r\n)");

    if (!std::regex_match(message, pattern)) {
        return false;
    }

    std::unordered_map<char, int> seat_count = {{'N', 0}, {'S', 0}, {'E', 0}, {'W', 0}};

    // Przejdź przez ciąg i zlicz wystąpienia liter N, S, E, W
    for (char ch : message.substr(5)) {
        if (seat_count.find(ch) != seat_count.end()) {
            seat_count[ch]++;
        }
    }

    // Sprawdź, czy każda litera wystąpiła dokładnie raz
    for (const auto& pair : seat_count) {
        if (pair.second != 1) {
            return false;
        }
    }

    return true;
}
bool Klient::validateTOTAL(const std::string& message){

    const std::regex pattern(R"(TOTAL[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)[NSEW](0|[1-9]\d*)\r\n)");
    if (!std::regex_match(message, pattern)) {
        return false;
    }

    std::unordered_map<char, int> seat_count = {{'N', 0}, {'S', 0}, {'E', 0}, {'W', 0}};

    // Przejdź przez ciąg i zlicz wystąpienia liter N, S, E, W
    for (char ch : message) {
        if (seat_count.find(ch) != seat_count.end()) {
            seat_count[ch]++;
        }
    }

    // Sprawdź, czy każda litera wystąpiła dokładnie raz
    for (const auto& pair : seat_count) {
        if (pair.second != 1) {
            return false;
        }
    }

    return true;
}
int Klient::validateMessage(const std::string& message){

    if(validateBUSY(message)){
        return BUSY;
    }
    if(validateDEAL(message)){
        return DEAL;
    }
    if(validateTRICK(message)){
        return TRICK;
    }
    if(validateTAKEN(message)){
        return TAKEN;
    }
    if(validateWRONG(message)){
        return WRONG;
    }
    if(validateSCORE(message)){
        return SCORE;
    }
    if(validateTOTAL(message)){
        return TOTAL;
    }

    return -1;
}

void Klient::printTakenTricks(){
    for(const std::string& trick : tricks_taken){
        std::cout << trick << "\n";
    }
}

void Klient::performTaken(const std::string& message){
    
    // jeśli ja biorę lewę to dodaje do wziętę karty do historii lew
    std::vector<std::string> card_vector = Card::extractCardsVectorFromCardsStringStr(message.substr(6 + (current_trick >= 10), message.length() - 9 - (current_trick >= 10)));
    std::string card_list;
    for(std::vector<std::string>::size_type i = 0; i < card_vector.size(); i++){
        card_list += card_vector[i] + ", ";
    }
    card_list = card_list.substr(0, card_list.length() - 2);
    if(position == message[message.length() - 3]){
        tricks_taken.push_back(card_list);
    }
    for(std::string card : card_vector){
        cardSet.removeCard(Card::stringToCard(card));
    }

}
void Klient::send_message(int socket_fd, const std::string &message){
    size_t length = message.length();
    ssize_t bytes_sent = send(socket_fd, message.c_str(), length, 0);

    if(bytes_sent == -1){
        std::cerr << "Błąd podczas wysyłania wiadomości\n";
        close(socket_fd);
        exit(1);
    }
    if(isBot){
        raport(get_local_address(socket_fd), get_server_address(socket_fd), message);
    }
}
int Klient::connectToServer(){

            std::cout << "Próba połączenia z serwerem " << host << " na porcie " << port << "\n";
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof (hints)); 

            if(IPv4){
                hints.ai_family = AF_INET;
            }else if(IPv6){
                hints.ai_family = AF_INET6;
            }else{
                hints.ai_family = AF_UNSPEC;
            }
            hints.ai_socktype = SOCK_STREAM;

            if(getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0){
                std::cerr << "Nie udało się uzyskać adresów\n";
                return -1;
            }

            int socket_fd;
            for(struct addrinfo *p = res; p != NULL; p = p->ai_next){
                
                socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if(socket_fd == -1){
                    continue;
                }
                if(connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1){
                    close(socket_fd);
                    continue;
                }
                freeaddrinfo(res);
                return socket_fd;
            }
            
            freeaddrinfo(res);
            return -1;
}
int Klient::run(){

    socket_fd = connectToServer();
    if(socket_fd < 0){
        std::cerr << "Nie udało się nawiązać połączenia\n";
        return 1;
    }
    std::string local_address = get_local_address(socket_fd);
    std::string server_address = get_server_address(socket_fd);

    std::string message = std::string("IAM") + position + "\r\n";
    send_message(socket_fd, message);
    
    struct pollfd fds[2]; fds[0].fd = socket_fd;  fds[0].events = POLLIN; fds[1].fd = STDIN_FILENO;  fds[1].events = POLLIN;
    char buffer[1024]; memset(buffer, 0, 1024); size_t buffer_index = 0;

    // TODO implement non-blocking sending
    // TODO mabye try to implement reading more than one byte at a time

    // TODO implement poll error handling
    // TODO implement writing out correct message
    // TODO check if writing out is as it should be, pay attention to stdout, and stderr

    while(true){

        for(int i = 0; i < 2; i++){ fds[i].revents = 0; }
        int poll_count = poll(fds, 2, -1); // Czekamy na zdarzenie
        if (poll_count < 0) {std::cerr << "Poll error" << std::endl;close(socket_fd);return 1;}

        // odczytanie wiadomości od serwera
        if(fds[0].revents & POLLIN){
            ssize_t bytes_received = read(socket_fd, &buffer[buffer_index], 1);

            if(bytes_received < 0){
                std::cerr << "Błąd podczas odbierania wiadomości\n";
                close(socket_fd);
                return 1;
            }else if(bytes_received == 0){
                std::cerr << "Serwer zamknął połączenie z klientem\n";
                close(socket_fd);
                if(wait_TOTAL || wait_SCORE){
                    std::cout<< "rozgrywka zakończona nieprawidłowo\n";
                    return 1;
                }else{
                    std::cout << "rozgrywka zakończona poprawnie\n";
                    return 0;
                }
            }
            char received_char = buffer[buffer_index];
            buffer_index++;
            // jeśli odebrany bajt to znak nowej linni
            if(received_char == '\n'){
                if(buffer_index > 1 && buffer[buffer_index - 2] == '\r'){
                    std::string message(buffer, buffer_index);
                    if(isBot){
                        raport(server_address, local_address, message);
                    }
                    // jeśli jestem na etapie oczekiwania na wiadomość DEAL lub BUSY
                    if(wait_DEAL){
                        if(validateMessage(message) == BUSY){
                            if(!isBot){
                                std::cout << "Place busy, list of busy places received: " + get_busy_places_from_BUSY(message) + ".\n";
                            }
                        }else if(validateMessage(message) == DEAL){
                            cardSet.addCardsFromCardsString(message.substr(6, message.length() - 8));
                            if(!isBot){
                                std::cout << "New deal " << message[4] << ": staring place " << message[5] << ", your cards: " << cardSet.getCardsOnHand() <<".\n";
                            }
                            // dodaje karty do tali
                            wait_DEAL = false;
                            wait_SCORE = true;
                            wait_TOTAL = true;
                            wait_first_TRICK = true;
                            got_TRICK = false;
                            current_trick = 1;
                        }
                    }else if(current_trick < 14){
                        if(validateMessage(message) == TRICK){
                            got_TRICK = true;
                            wait_first_TRICK = false;

                            // wypisuje instrukcję dla zawodnika
                            if(!isBot){
                                std::cout << "Trick: (" << current_trick << ") " + extract_cards_from_TRICK(message) + "\n";
                                std::cout << "Available: " + cardSet.getCardsOnHand() + "\n";
                            }else{
                                
                                Card card = cardSet.getCardOfColor(get_first_card_color_from_TRICK(message));
                                std::string to_send = "TRICK" + std::to_string(current_trick) + card.rankToString(card.getRank()) + card.getColor() + "\r\n";
                                send_message(socket_fd, to_send);
                            }
                        }else if(validateMessage(message) == TAKEN){
                            // tylko jeśli czekam wciąż na pierwszego tricka
                            if(wait_first_TRICK || got_TRICK){
                                if(!isBot){
                                    std::cout << convert_taken_message(message) << "\n";
                                }
                                //TODO implement perform TAKEN (jeśli ja biorę lewę to biorę, ale dodatkowo usuwam na pewno kartę z ręki)
                                performTaken(message);
                                current_trick++;
                                got_TRICK = false;
                            }
                        }else if(validateMessage(message) == WRONG){
                                if(!isBot){
                                    std::cout << "Wrong message received in trick " << message[5] << ".\n";
                                }
                        }
                    
                    // tutaj powinienem otrzymać SCORE i TOTAL
                    // tutaj jest okej, bo mogę to otrzymać jedynie gdy curr_trick == 14
                    }else{
                            if(validateMessage(message) == SCORE){
                                
                                // w przypadku tej implementacj też chyba nic nie robię
                                if(!isBot){
                                    std::cout << convert_score_message(message) << "\n";
                                }
                                wait_SCORE = false;

                            }else if(validateMessage(message) == TOTAL){
                                
                                if(!isBot){
                                    std::cout << convert_total_message(message) << "\n";
                                }
                                wait_TOTAL = false;
                            }
                            if(!wait_TOTAL && !wait_SCORE){
                                wait_DEAL = true;
                            }
                        }
                    // otrzymałem faktycznie wiadomość
                    buffer_index = 0;
                    memset(buffer, 0, 1024);
                }
            }
        
            // jeśli przekroczyłem limit długości wiadomości
            if(buffer_index == MESSAGE_LIMIT){
                buffer[buffer_index] = '\r';
                buffer[buffer_index + 1] = '\n';
                if(isBot){
                    raport(server_address, local_address, buffer);
                }
                buffer_index = 0;
                memset(buffer, 0, 1024);
            }
        }
        // odczytanie wiadomości z stdin
        if(fds[1].revents & POLLIN){
            std::string message;
            std::cin >> message;
            if("cards" == message){
                // print_hand();
                std::cout << cardSet.getCardsOnHand() << "\n";
            }
            if("tricks" == message){
                printTakenTricks();
            }
            if(is_valid_request_to_send_card(message)){
                std::string to_send = "TRICK" + std::to_string(current_trick) + message.substr(1) + "\r\n";
                send_message(socket_fd, to_send);
            }

        }
    }
    return 0;
}

