#include <iostream>
#include "server.h"
#include "common.h"


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
        : port(port), file(file), timeout(timeout * 1000), connected_players(0), gameplay(file), queue_length(5), is_E_connected(false), is_N_connected(false), is_S_connected(false), is_W_connected(false), current_trick(0)
        {}

Server::~Server(){}

// funkcja sprawdzająca poprawność wiadomości IAM
bool Server::validateIAM(const std::string& message){

    if(message.length() != 6){
        return false;
    }
    if(message[3] != 'N' && message[3] != 'S' && message[3] != 'W' && message[3] != 'E'){
        return false;
    }

    return true;
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
int Server::calculateTimeToWait(int last_event_TRICK, int last_event_IAM) {
    if (last_event_IAM == -1 && last_event_TRICK == -1) {
        return -1;  // Jeśli żadne zdarzenie nie miało miejsca, zwróć -1
    }

    int timeout_ms = timeout;
    if (last_event_IAM != -1) {
        // Oblicz czas pozostały do timeoutu na otrzymanie IAM
        int time_to_iam_timeout = timeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_event_IAM).count();
        timeout_ms = min(timeout_ms, time_to_iam_timeout);
    }

    if (last_event_TRICK != -1) {
        // Oblicz czas pozostały do timeoutu na otrzymanie TRICK
        int time_to_trick_timeout = timeout - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - last_event_TRICK).count();
        timeout_ms = min(timeout_ms, time_to_trick_timeout);
    }

    return max(0, timeout_ms);  // Upewnij się, że timeout nie jest ujemny
}

int Server::run(){

    // tworzę gniazdo obsługujące zarówno połączenia ipv4 jak i ipv6
    int socket_fd = setupServerSocket();
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda IPv6\n";
        return 1;
    }

    // numery deskryptorów
    // O N E S W P N E S W P
    // 0 1 2 3 4 5 6 7 8 9 10 
    // deskryptor do zapisu jest oddalony o 5 od deskryptora do odczytu
    // inicjalizacja tablicy deskryptorów
    struct pollfd poll_descriptors[11];
    poll_descriptors[CONNECTION_SOCKET].fd = socket_fd;
    poll_descriptors[CONNECTION_SOCKET].events = POLLIN;

    // deskryptory na przyjmowanie wiadomości od klientów
    for(int i = 0; i < 5; i++){
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLIN;
    }
    // deskryptory na zapisywanie do klientów
    for(int i = 5; i < 10; i++){
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLOUT;
    }
    // tworzę bufory dla poszczególnych klientów i poczekalni
    // powinienem mieć dla każdego dwa deskryptory, deskryptor gotowy do odczytu
    // deskryptor gotowy do zapisu
    // O 
    char buffer[10][BUFFER_SIZE];
    // countery dla poszczególnych klientów
    for(int i = 0; i < 10; i++){
        // zeruję bufory
        memset(buffer[i], 0, BUFFER_SIZE);
    }
    // inicjalizacja liczników buforów dla klientów
    size_t buffer_counter[10];
    for(int i = 0; i < 10; i++){
        buffer_counter[i] = 0;
    }
    int last_event_TRICK = -1;
    int last_event_IAM = -1;
    // TODO: implement errors in poll
    std::string lined_cards = "";
    bool finish = false;
    int time_to_wait = -1;
    while(finish == false){

        // zerujemy wydarzenia dla wszystkich deskryptorów
        for(int i = 0; i < 11; i++){
            poll_descriptors[i].revents = 0;
        }

        // czekam na zdarzenia maksymalnie timout
        //TODO implement timeout
        // TODO implement not readeing at once
        int time_to_wait = calcualteTimeToWait(last_event_TRICK, last_event_IAM);
        int poll_status = poll(poll_descriptors, 6, time_to_wait);
        // arguement na 

        // jeśli przyszło nowe połączenie, a nie ma nikogo w poczekalni
        if(poll_descriptors[CONNECTION_SOCKET].revents & POLLIN && poll_descriptors[PREAD].fd != -1){

            // najpierw akceptuje połączenie z tym klientem
            int waiting_room_fd = accept(socket_fd, NULL, NULL);
            if(waiting_room < 0){
                std::cerr << "Błąd podczas akceptowania połączenia\n";
                return 1;
            }
            // aktualizuje czas ostatniego zdarzenia IAM
            last_event_IAM = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()).count();
            // dodaję deskryptor do tablicy
            poll_descriptors[PREAD].fd = waiting_room_fd;
            poll_descriptors[PWRITE].fd = waiting_room_fd;
        }

        // jeśli ktoś jest w poczekalni
        if(poll_descriptors[PREAD].fd != -1){

            // sprawdzam czy nie przysłał mi wiadomości
            if(poll_descriptors[PREAD].revents & POLLIN){

                // odczytuje wiadomość
            }

            // jeśli mam coś do zapisania do klienta
            if(){

            }

        }

        // sprawdzam czy coś dzieje się w poczeklani
        if(poll_descriptors[5].fd != -1){

            // jeśli ktoś jest w poczeklani
            // to sprawdzam czy nie przysłał mi wiadomości
            if(poll_descriptors[5].revents & POLLIN){

                // FIXME:: implement timeout
                // jeśli przysłał wczytuje wiadomość
                ssize_t bytes_received = read(poll_descriptors[5].fd, buffer[5] + buffer_counter[5], 1);

                if(bytes_received < 0){
                    std::cerr << "Błąd podczas odczytywania wiadomości\n";
                    close(poll_descriptors[5].fd);
                    poll_descriptors[5].fd = -1;
                    return 1;

                // klient się rozłączył trzeba to jakoś obsłużyć
                }else if(bytes_received == 0){
                    close(poll_descriptors[5].fd);
                    poll_descriptors[5].fd = -1;
                }

                char received_char = buffer[5][buffer_counter[5]];
                buffer_counter[5] += 1;
                
                // jeśli jest to możliwy koniec wiadomości
                if(received_char == '\n'){
                    if(buffer_counter[5] > 1 && buffer[5][buffer_counter[5] - 2] == '\r'){
                        std::string message(buffer[5], buffer_counter[5]);
                        // FIXME: implement raport

                    }
                }

            // nie ma danych do odczytrania, więc sprawdzam czy nie mineło wystarczająco dużo czasu
            }else{
                
                // jeśli mineło zbyt dużo czasu
                // if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - last_event_timeWaiting).count() > timeout){
                //     // zamykam połączenie z klientem
                //     close(poll_descriptors[5].fd);
                //     poll_descriptors[5].fd = -1;

                // }
            }
        }

        // jeśli wykonaliśmy już wszystkie rzeczy to zmieniamy finish na true
        // i kończymy program, rozłączamy wtedy wszystkich klientów

    }
    


    // zamykam gniazdo, rozgrywka zakończona prawidłowo
    close(socket_fd_ipv6);
    return 0;
}


int Server::setupServerSocket(){
    std::cout << "Próbuje utworzyć gniazdo\n";

    int socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return 1;
    }

    // Ustawienie gniazda, aby obsługiwało zarówno IPv6 jak i IPv4
    int flag = 0;
    if (setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&flag, sizeof(flag)) < 0) {
        std::cerr << "Błąd podczas ustawiania opcji gniazda\n";
        return 1;
    }

    struct sockaddr_in6 server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin6_family = AF_INET6;
    server_address.sin6_addr = in6addr_any;
    server_address.sin6_port = htons(port);

    // próbuje bindować gniazdo do konkretnego adresu
    if(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t) sizeof(server_address)) < 0){
        std::cerr << "Błąd podczas bindowania gniazda\n";
        return 1;
    }

    // przełączam gniazdo w tryb nasłuchiwania
    if(listen(socket_fd, queue_length) < 0){
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe\n";
        return 1;
    }

    // sprawdzam jaki port został wybrany
    socklen_t length = (socklen_t) sizeof server_address;
    if (getsockname(socket_fd, (struct sockaddr*) &server_address, &length) < 0){
        std::cerr << "Błąd podczas pobierania numeru portu\n";
        return 1;
    }
    std::cout << "Serwer nasłuchuje na porcie: " << ntohs(server_address.sin6_port) << "\n";

    return socket_fd;
}


