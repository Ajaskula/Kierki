#include <iostrim>
#include "Server.h"
#include "common.h"

int parseArguments(int argc, char* argv[], uint16_t& port, std::string& file, int& timeout){

    // przechodzę przez wszystkie argumenty
    for(int i = 0; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-p" && i + 1 < argc){
            port = std::stoi(argv[i + 1]);
        
        } else if(arg == "-f" && i + 1 < argc){
            file = argv[i + 1];
        }else if(arg == "-t" && i + 1 < argc){
            timeout = std::stoi(argv[i+1]);
        }else{
            cerr << "Nieznany parametr: " << arg << "\n";
        }
    }

    // sprawdza czy został podany plik
    if(file.empty()){
        std:cerr << "Błąd parametr -f jest wymagany\n";
        return 1;
    }

    if(port == 0){
        std::cout << "Numer portu zostanie wybrany domyślnie przez funkcję bind.\n";
    }else{
        std::cout << "Port: " << port << "\n";
    }
    return 0;
}


Server::Server(uint16_t port, const std::string& file, int timeout)
        : port(port), file(file), timeout(timeout), cardSet(true)
        {   
            std::cout << "Instancja serwera została stworzona\n";
        }

Server::~Server(){
            std::cout << "Instancja serwera została zniszczona\n";
        }

// funkcja sprawdzająca poprawność wiadomości IAM
bool Server::validate_IAM(const std::string& message){

    if(message.length() != 4){
        return false;
    }
    if(message[3] != 'N' && message[3] != 'S' && message[3] != 'W' && message[3] != 'E'){
        return false;
    }

    return true;
}