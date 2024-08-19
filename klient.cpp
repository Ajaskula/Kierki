#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <regex>
#include <set>
#include "klient.h"
#include "common.h"
#include <netdb.h>
#include <poll.h>

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

Klient::Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot)
        : host(host), port(port), IPv4(IPv4), IPv6(IPv6), position(position), isBot(isBot), cardSet(), points(0), trick_history(){}
Klient::~Klient(){
            std::cout << "Instancja klienta została zniszczona\n";
        }

// sprawdza poprawność wiadomości BUSY
bool Klient::validate_BUSY(const std::string& message){
    // poprawna wiadomość BUSY może mieć maksymalnie 8 znaków i minimalnie 5
    if(message.length() > 8 || message.length() < 5){
        return false;
    }
    // poprawna wiadomość BUSY może zawierać tylko znaki N, S, E, W
    for(std::string::size_type i = 4; i < message.length(); i++){
        if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
            return false;
        }
    }
    // poprawna wiadomość BUSY musi zawierać znak gracza, który próbuje dołączyć
    if(message.substr(4).find(position) == std::string::npos){
        return false;
    }
    // każdy ze znaków N, S , E, W może wystąpić tylko raz
    for(std::string::size_type i = 4; i < message.length(); i++){
        // metoda rfind zwraca pierwsze występienia znaku od końca
        if(message.substr(4).find(message[i]) != message.substr(4).rfind(message[i])){
            return false;
        }
    }
    return true;
}
// sprawdzam w ten sposób, czy karta jest poprawna
// korzystjąc z wyrażeń regularnych sprawdzam poprawność karty
bool is_valid_card(const std::string& card){
    static const std::regex card_regex("^(10|[2-9]|[JQKA])[CDHS]$");
    return std::regex_match(card, card_regex);
}
// funkcja wypisuje karty, które klient, aktualnie ma na ręce
void Klient::print_hand(){
    std::cout << "Karty na ręce: ";
    for(const Card& card : cardSet.cards){
        std::cout << card.to_string() << " ";
    }
    std::cout << "\n";
}

// sprawdzam czy otrzymane karty są poprawne
bool is_valid_hand(const std::string& hand) {
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

        if (!is_valid_card(card)) {
            return false; // Znaleziono niepoprawną kartę
        }

        if (!unique_cards.insert(card).second) {
            return false; // Znaleziono duplikat
        }

        card_count++;

        if (card_count > 13) {
            return false; // Zbyt wiele kart
        }
    }

    return card_count == 13; // Sprawdzamy, czy przetworzono dokładnie 13 kart
}

// sprawdzam poprawność wiadomości DEAL
bool Klient::validate_DEAL(const std::string& message){
    
    // zła długość wiadomości
    if(message.length() < 32 || message.length() > 36){
        return false;
    }
    // sprawdzenie typu rozdania
    if (message.length() > 4 && std::isdigit(message[4])) {
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
    if(is_valid_hand(message.substr(6, 39)) == false){
        return false;
    }

    // dodaje karty do klienta
    return true;
}

// konwertuje stringa do talii kart
std::vector<Card> cards_to_set(const std::string& hand) {
    std::vector<Card> card_set;

    size_t i = 0;
    size_t hand_length = hand.length();

    while (i < hand_length) {
        std::string rank_str;
        char color;

        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            rank_str = "10"; // Ranga to "10"
            color = hand[i + 2]; // Kolor jest na pozycji 3 (indeks i + 2)
            i += 3;
        } else if (i + 1 < hand_length) {
            rank_str = hand.substr(i, 1); // Ranga to jeden znak (2-9, J, Q, K, A)
            color = hand[i + 1]; // Kolor jest na pozycji 2 (indeks i + 1)
            i += 2;
        } else {
            throw std::invalid_argument("Niekompletna karta w stringu");
        }

        Rank rank = Card::string_to_rank(rank_str);
        card_set.push_back(Card(color, rank));
    }

    return card_set;
}

// wyświetla historię wziętych lew
void Klient::print_trick_history(){
    std::cout << "Historia wziętych lew: ";
    for(const std::string& trick : trick_history){
        std::cout << trick << " ";
    }
    std::cout << "\n";
}


int Klient::run(){

    int socket_fd = connect_to_server();
    std::string local_address = getLocalAddress(socket_fd);
    std::string server_address = getServerAddress(socket_fd);
    if(socket_fd == -1){
        std::cerr << "Nie udało się nawiązać połączenia\n";
        return 1;
    }
    // wysyłam do serwera wiadomość IAM
    std::string message = std::string("IAM") + position + "\r\n";
    send_message(socket_fd, message);
    // jeśli jestem botem raportuje wysłaną wiadomość
    if(isBot){
        std::cout << message;
    }
    
    struct pollfd fds[2];
    fds[0].fd = socket_fd;  // gniazdo serwera
    fds[0].events = POLLIN;
    // tutaj czekam na wejściu standardowym
    fds[1].fd = STDIN_FILENO;  // stdin
    fds[1].events = POLLIN;

    // bufor na wiadomości od serwera
    char buffer[1024];
    memset(buffer, 0, 1024);
    size_t buffer_index = 0;

    while(true){

        for(int i = 0; i < 2; i++){
            fds[i].revents = 0;
        }

        int poll_count = poll(fds, 2, -1); // Czekamy na zdarzenie
        if (poll_count < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
            close(socket_fd);
            return 1;
        }

        // odczytanie wiadomości od serwera
        if(fds[0].revents & POLLIN){
            ssize_t bytes_received = read(socket_fd, &buffer[buffer_index], 1);

            if(bytes_received == -1){
                std::cerr << "Błąd podczas odbierania wiadomości\n";
                close(socket_fd);
                return 1;
            }else if(bytes_received == 0){
                std::cerr << "Serwer zamknął połączenie\n";
                close(socket_fd);
                return 1;
            }
            char received_char = buffer[buffer_index];
            std::cout << "Otrzymano bajt: " << received_char << "\n";
            std::cout << "Numer bajta w kodzie ascii: " << (int)received_char << "\n";
            buffer_index++;
            // jeśli odebrany bajt to znak nowej linni
            if(received_char == '\n'){
                if(buffer_index > 1 && buffer[buffer_index - 2] == '\r'){
                    buffer_index = 0;
                    if(isBot){
                        std::cout << raport(server_address, local_address, buffer);
                    }
                    // BUSY
                    if(0){
                    
                    // DEAL
                    }else if(0){

                    // TRICK
                    }else if(0){

                    // WRONG
                    }else if(0){

                    // TAKEN
                    }else if(){

                    // SCORE
                    }else if{

                    // TOTAL
                    }else if(){

                    }
                    memset(buffer, 0, 1024);
                }
            }
            // jeśli przekroczyłem limit długości wiadomości
            if(buffer_index == MESSAGE_LIMIT){
                buffer[buffer_index] = '\r';
                buffer[buffer_index + 1] = '\n';
                if(isBot){
                    std::cout << raport(server_address, local_address, buffer);
                }
                buffer_index = 0;
                memset(buffer, 0, 1024);
            }
        }
        if(fds[1].revents & POLLIN){
            // odczytanie wiadomości od klienta
            std::string message;
            std::cin >> message;
            std::cout << "Otrzymano wiadomość z stdin: " << message << "\n";
            // wysłanie wiadomości do serwera
            // send_message(socket_fd, message);
            if("cards" == message){
                std::cout << "Wypisuje karty\n";
                print_hand();
                // std::cout << getCurrentTime() << "\n";
                // std::cout << getServerAddress(socket_fd) << "\n";
                // std::cout << getLocalAddress(socket_fd) << "\n";
                std::cout << raport(server_address, local_address, "Wypisuje karty\r\n");
            }
            if("tricks" == message){
                print_trick_history();
            }
            // jeśli przyszło polecenie dołożenia karty wysyłam komunikat trick
            if(0){

            }

        }
    }


    return 0;
}


// funkcja pozwala na wysyłanie wiadomości do serwera
// podaje deskryptor gniazda, na który ma zostać wysłana wiadomość
// przekazanie stringa przez stałą referencję, const informuje, że funkcja nie zmieni wartości stringa
void Klient::send_message(int socket_fd, const std::string &message){
    // odczytuje długość stringa
    size_t length = message.length();
    // wysyłam wiadomość do serwera na podstawie deskryptora gniazda, nie przekazuje żandych flag
    // metoda .c_str() zwraca wskaźnik na tablicę znaków, która jest przechowywana w stringu
    ssize_t bytes_sent = send(socket_fd, message.c_str(), length, 0);

    // jeśli nie udało się wysłać wiadomości do serwera
    if(bytes_sent == -1){
        std::cerr << "Błąd podczas wysyłania wiadomości\n";
        close(socket_fd);
        exit(1);
    }
    // pomyślnie wysłano wiadomość do serwera
    std::cout << "Wysłano wiadomość: " << message << "\n";
}

// funkcja pozwala na odbieranie wiadomości od serwera
int Klient::receive_message(int socket_fd){

    // rozmiar bufora
    const size_t buffer_size = 1024;
    // tablica znaków, w której zostanie zapisana wiadomość
    char buffer[buffer_size];

    // odbieranie wiadomości od serwera
    // zwraca liczbę otrzymanych bajtów
    // buffer - wskaźnik na tablicę znaków, w której zostanie zapisana wiadomość
    // odejumjemu 1 od rozmiaru bufora, żeby zostawić miejsce na terminalne zero
    ssize_t bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);

    // nie udąło się załadować wiadomości
    if(bytes_received == -1){
        std::cerr << "Błąd podczas odbierania wiadomości\n";
        close(socket_fd);
        exit(1);
    
    // serwer zamknął połączenie
    } else if (bytes_received == 0){
        std::cerr << "Serwer zamknął połączenie\n";
        close(socket_fd);
        return 1;
    }

    // dodajemy terminalne zero na końcu otrzymanej wiadomości
    buffer[bytes_received] = '\0';
    std::cout << "Otrzymano wiadomość: " << buffer << "\n";
    return 0;
}




// funkcja nawiązująca połączenie przez klienta
int Klient::connect_to_server(){

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

