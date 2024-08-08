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
void Klient::run(){
            if(connect()){
                std::cerr << "Nie udało się połączyć z serwerem\n";
            }
        }
bool Klient::connect(){
            std::cout << "Próba połączenia z serwerem " << host << " na porcie " << port << "\n";
            return false;
        }

int run(){
    int socket_fd = connect();
    if(socket_fd == -1){
        std::cerr << "Nie udało się nawiązać połączenia\n";
        return -1;
    }

    std::cout << "Połączenie nawiązane\n";

    // logika działania klienta

    // zamykanie połączenie
    close(socket_fd);
}

// funkcja nawiązująca połączenie przez klienta
int Klient::connect(){
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
            for(struct addrinfo *p = res; p != NULL; p = p->ai_next){

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
            
            freeaddrinfo(res);
            return -1;
}


