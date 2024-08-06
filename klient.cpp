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


