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
        : port(port), file(file), timeout(timeout), connected_players(0), gameplay(file), queue_length(5), is_E_connected(false), is_N_connected(false), is_S_connected(false), is_W_connected(false), current_trick(0)
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

int Server::run(){

    // int socket_fd_ipv4 = setupServerSocketIPv4();
    // if(socket_fd_ipv4 < 0){
    //     std::cerr << "Błąd podczas tworzenia gniazda IPv4\n";
    //     return 1;
    // }
    int socket_fd_ipv6 = setupServerSocketIPv6();
    if(socket_fd_ipv6 < 0){
        std::cerr << "Błąd podczas tworzenia gniazda IPv6\n";
        return 1;
    }

    // ustawiam timeout
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    // inicjalizacji struktury pollfd
    struct pollfd poll_descriptors[6];

    // dwa pierwsze sockety nasłuchują na połączenia
    // poll_descriptors[0].fd = socket_fd_ipv4;
    // poll_descriptors[0].events = POLLIN;
    poll_descriptors[0].fd = socket_fd_ipv6;
    poll_descriptors[0].events = POLLIN;

    // pozostałe sockety, bądą obsługiwać połączenia z klientami
    for(int i = 2; i < 6; i++){
        poll_descriptors[i].fd = -1;
        poll_descriptors[i].events = POLLIN;
    }
    bool finish = false;
    // wykonuje dopóki nie pojawi się 4 graczy
    while(1){
        // std::cout << "Czekam na graczy\n";
        int poll_status = poll(poll_descriptors, 6, timeout);
      // jeśi pojawił się jakiś gracz
      if(poll_descriptors[0].revents & POLLIN){
          int client_fd = accept(poll_descriptors[0].fd, NULL, NULL);
          if(client_fd < 0){
              std::cerr << "Błąd podczas akceptowania połączenia\n";
              return 1;
          }
          std::cout << "Połączono z klientem\n";
          std::string message = "BUSYNESS\r\n";
          int bytes_sent = send(client_fd, message.c_str(), message.length(), 0);
          // sendmessage(client_fd, message);
        //   close(client_fd);

      }
    }
    // do{

    //     // zerujemy zdarzenie, które zaszły na deskryptorach
    //     for(int i = 0; i < 6; i++){
    //         poll_descriptors[i].revents = 0;
    //     }

    //     // czekamy na zdarzenia na deskryptorach
    //     int poll_status = poll(poll_descriptors, 6, timeout);

    //     if(poll_status < 0){
    //         std::cerr << "Błąd podczas wywołania poll\n";
    //         return 1;

    //     }

        // to znaczy, żę pojawiły się jakieś zdarzenia
        // if(poll_status > 0){

        //     // jeśli pojawiło się zdarzenia na głównym sockecie IPv4
        //     if(poll_descriptors[0].revents & POLLIN){

        //         // jeśli rozgrywka trwa, to od razu odsyłam busy
        //         if(connected_players == 4){

        //             int client_fd = accept(poll_descriptors[0].fd, NULL, NULL);
        //             if(client_fd < 0){
        //                 std::cerr << "Błąd podczas akceptowania połączenia\n";
        //                 return 1;
        //             }
        //             std::string message = "BUSYNESW\r\n";
        //             // sendmessage(client_fd, message);
        //             close(client_fd);

        //         }else if(/*w tablicy deskryptorów jest mniej niż 4*/){
        //             // szukam wolnego miejsca w tablicy deskryptorów
        //             // na każdym z tych deskryptorów ustawiam timeout
        //             // albo go akceptuje albo odsyłam 

        //         }

        //     }

            // jeśli pojawiła się jakaś wiadomość, na którymś z deskryptorów
            // być może mogę zamknąć wtedy połączenie, a być możę będę miał już gotowego gracza
    //     }

    // }while(!finish);



    // close(socket_fd_ipv4);
    close(socket_fd_ipv6);
    return 0;
}

int Server::setupServerSocketIPv4(){
    std::cout << "Próbuje utworzyć gniazdo\n";
    
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        std::cerr << "Błąd podczas tworzenia gniazda\n";
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    // próbuje bindować gniazdo do konkretnego adresu
    if(bind(socket_fd, (struct sockaddr*)&server_address, (socklen_t) sizeof(server_address)) < 0){
        std::cerr << "Błąd podczas bindowania gniazda\n";
        return 1;
    }

    if(listen(socket_fd, queue_length) < 0){
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe\n";
        return 1;
    }


    // sprawdźmy jaki port został wybrany
    socklen_t length = (socklen_t) sizeof server_address;
    if (getsockname(socket_fd, (struct sockaddr*) &server_address, &length) < 0){
        std::cerr << "Błąd podczas pobierania numeru portu\n";
        return 1;
    }
    std::cout << "Serwer nasłuchuje na porcie: " << ntohs(server_address.sin_port) << "\n";

    return socket_fd;
}


int Server::setupServerSocketIPv6(){
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


