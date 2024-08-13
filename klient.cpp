#include <iostream>
#include <string>
#include "klient.h"
#include "common.h"

// metoda nie związana z klasą klienta
int parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot){
    for(int i = 0; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-h" && i + 1 < argc){
            host = argv[i + 1];
        }else if(arg == "-p" && i + 1 < argc){
            port = std::stoi(argv[i + 1]);
        }else if(arg == "-4"){
            IPv4 = true;
        }else if(arg == "-6"){
            IPv6 = true;
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
        }

    }
    if(IPv4 && IPv6){
        std::cerr << "Nie można jednocześnie używać IPv4 i IPv6\n";
        return 1;
    }
    if("\0" == position){
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
    return 0;
}

Klient::Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot)
        : host(host), port(port), IPv4(IPv4), IPv6(IPv6), position(position), isBot(isBot), cardSet(), points(0)
        {
            std::cout << "Instancja klienta została stworzona\n";
        }
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
    for(int i = 4; i < message.length(); i++){
        if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
            return false;
        }
    }
    // poprawna wiadomość BUSY musi zawierać znak gracza, który próbuje dołączyć
    if(message.substr(4).find(position) == std::string::npos){
        return false;
    }
    // każdy ze znaków N, S , E, W może wystąpić tylko raz
    for(int i = 4; i < message.length(); i++){
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

// sprawdzam poprawność wiadomości DEAL
bool Klient::validate_DEAL(const std::string& message){
    
    // zła długość wiadomości
    if(message.length() < 32 || message.length() > 36){
        return false;
    }
    // sprawdzenie typu rozdania
    if(atoi(message[4]) < 1 || atoi(message[4]) > 7){
        return false;
    }

    // sprawdzenie pozycji klienta wychodzącego jako pierwszy
    if(message[5] != 'N' && message[5] != 'S' && message[5] != 'E' && message[5] != 'W'){
        return false;
    }

    // sprawdzenie czy lista kart jest poprawna
    int card_counter = 0;
    for(){

    }

}

int Klient::run(){

    // nawiązanie połączenia
    int socket_fd = connect_to_server();

    // jeśli nie udąło się nawiązać połączenia
    if(socket_fd == -1){
        std::cerr << "Nie udało się nawiązać połączenia\n";
        return -1;
    }

    std::cout << "Połączenie nawiązane\n";

    // wyślij wiadomość powitalną do serwera
    std::string message = "IAM" + position + "\r\n";
    send_message(socket_fd, message);

    // w tym miejscu klient czeka na odpowiedź od serwera
    // możliwe odpowiedzi to:
    // - busy i zakończenie połączenia, jeśli miejsce przy stole jest zajęte
    // Deal i rozpoczęcie rozgrywki
    // wszystkie inne rzeczy są ignorowane (niepoprawne odpowiedzi serwera)

    // odbieranie wiadomości od serwera, wiadomość znajdzie się we wskazanym buforze

    const size_t buffer_size = 1024;
    char buffer[buffer_size] = {0};
    ssize_t bytes_received = 0;
    for(;;){
        bool incorrect_message = false;
        // odbieram wiadomość
        bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);

        // wystąpił błąd, lub serwer zamknął połączenie
        if(bytes_received == -1){
            std::cerr << "Błąd podczas odbierania wiadomości\n";
            close(socket_fd);
            return -1;
        }else if(bytes_received == 0){
            std::cerr << "Serwer zamknął połączenie\n";
            close(socket_fd);
            return -1;
        }

        std::cout << "Otrzymano wiadomość: " << buffer << "\n";

        // szukam pierwszego wystąpienia \r\n
        string accumulated_data(buffer, bytes_received);

        size_t pos = accumulated_data.find("\r\n");
        if(pos != std::string::npos){
            message = accumulated_data.substr(0, pos);
            accumulated_data.erase(0, pos + 2);
            break;
        }

        // w message znajduje się wiadomość, gotowa do analizy
        if(message.substr(0, 4) == "BUSY"){

            // sprawdzenie poprawności wiadomości BUSY
            // poprawna wiadomość BUSY może mieć maksymalnie 8 znaków
            // poprawna wiadomość BUSY może zawierać tylko znaki N, S, E, W
            // poprawna wiadomość BUSY musi zawierać znak gracza, który próbuje dołączyć
            if(message.length() > 8){
                cerr << "Niepoprawna wiadomość serwera\n";
                cerr << message.c_str() << "\n";
                incorrect_message = true;
            
            // jeśli na liście pozycji nie znajduje się znak gracza, który próbuje dołączyć
            }else if(message.substr(4).find(position) == std::string::npos){
                cerr << "Niepoprawna wiadomość serwera\n";
                cerr << message.c_str() << "\n";
                incorrect_message = true;
            
            // jeśli na liście pozycji znajduje się znak, który nie powinien tam być
            }else{
                // sprawdzenie czy lista zawiera tylko odpowiednie znaki
                for(int i = 4; i < message.length(); i++){
                    if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
                        cerr << "Niepoprawna wiadomość serwera\n";
                        cerr << message.c_str() << "\n";
                        incorrect_message = true;
                        break;
                    }

            }
            // wiadomość busy jest poprawna, zamykam połączenie
            if(!incorrect_message){
                close(socket_fd);
                return -1;
            }

        // analizuję wiadomość DEAL
        }else if(message.substr(0, 4) == "DEAL"){

            // sprawdzenie poprawności wiadomości DEAL
            // sprawdzenie typu rozdania
            if(message.length() < 32 || message.length() > 36){
                cerr << "Niepoprawna wiadomość serwera\n";
                cerr << message.c_str() << "\n";
                incorrect_message = true;

            }else if(){

            }
        
        // inne wiadomości ignoruję jako błędne wiadomości serwera
        }else{
            cerr << "Niepoprawna wiadomość serwera\n";
            cerr << message.c_str() << "\n";
        }

        // sprawdzam czy otrzymana wiadomość jest poprawna czy nie
        if(incorrect_message){
            cout << "Czekam na następne wiadomości\n";
        }else{
            break;
        }
    }


    // liczba otrzymanych bajtów

    // jeśli wystąpił błąd podczas 

    // otrzymaliśmy wiadomość, dodajemy terminalne zero na końcu

    // otrzymałem jakąś wiadmość, teraz chcę ją przeanalizować

    std::string accumulated_data;
    accumulated_data.append(buffer, bytes_received);
    size_t pos = 0;

    // dopóki w buforze nie znajdę braku dopasowania
    while((pos = accumulated_data.find("\r\n")) != std::string::npos){

        message = accumulatedData.substr(0, pos);
        accumulated_data.erase(0, pos + 2);

        // jeśli wiadomość to "busy"
        if(message.substr(0, 4) == "BUSY"){
            std::cout << "Miejsce przy stole jest zajęte\n";

            // możliwe przyczyny niepoprawności tego rodzaju komunikatu
            // muszę jeszcze sprawdzić poprawność otrzymanej wiadomości
            if(message.length() > 8){
                // niepoprawna wiadomość
            }
            // jeśli w 
            bool my_position = false;

            // sprawdzam czy w wiadomości znajduje się mój znak i czy znajdują się tylko poprawne znaki
            for(int i = 4; i < message.length(); i++){
                if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
                    // niepoprawna wiadomość
                }
                if(message[i] == position){
                    my_position = true;
                }

            }
            if(my_position == false){
                // niepoprawna wiadomość
            }


            close(socket_fd);
            return -1;

        // jeśli wiadomość to "DEAL"
        }else if(message.substr(0, 4) == "DEAL"){
            std::cout << "Rozpoczęcie rozgrywki\n";

            // tutaj powinienem otrzymać 13 kart
            break;
        
        // inne wiadomości są ignorowane
        }else{

        }


    }


    // odpowiadanie na komunikaty typu trick



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
        return;
    }

    // dodajemy terminalne zero na końcu otrzymanej wiadomości
    buffer[bytes_received] = '\0';
    std::cout << "Otrzymano wiadomość: " << buffer << "\n";
}

// funkcja nawiązująca połączenie przez klienta
int Klient::connect_to_server(){
            std::cout << "Próba połączenia z serwerem " << host << " na porcie " << port << "\n";

            // hints jest zmienną lokalną, która służy do przekazywania informacji o tym, jakiego typu adresy chcemy uzyskać
            // res jest wskaznikiem na strukturę, która będzie przechowywać wynik działania funkcji getaddrinfo
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof (hints)); // wypełniamy strukturę hints zerami

            // w zależności od tego co zostało podane ustawiam odpowiedni atrybut w strukturze hints
            if(IPv4){
                hints.ai_family = AF_INET; // chcemy uzyskać adresy IPv4
            }else if(IPv6){
                hints.ai_family = AF_INET6; // chcemy uzyskać adresy IPv6
            }else{
                hints.ai_family = AF_UNSPEC; // chcemy uzyskać adresy IPv4 lub IPv6
            }
            hints.ai_socktype = SOCK_STREAM; // chcemy uzyskać adresy dla protokołu TCP

            // funkcja getaddrinfo zwraca 0, jeśli udało się uzyskać adresy
            if(getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0){
                std::cerr << "Nie udało się uzyskać adresów\n";
                return -1;
            }

            // tworzenie socketu i próba połączenia
            int socket_fd;
            // iteruje poprzez wszystkie adresy zwrócone przez getaddrinfo
            for(struct addrinfo *p = res; p != NULL; p = p->ai_next){
                
                // próba utowrzenia socketu
                socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if(socket_fd == -1){
                    continue;
                } // próba kolejnego połączenia

                if(connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1){
                    close(socket_fd);
                    continue;
                } // próba kolejnego połączenia

                freeaddrinfo(res);
                return socket_fd;
            }
            
            // zwolni całą listę uzyskanych adresów na raz
            freeaddrinfo(res);
            return -1;
}


