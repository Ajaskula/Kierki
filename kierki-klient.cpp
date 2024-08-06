#include <iostream>
#include <vector>
#include <string>
#include "klient.h"

int main(int argc, char *argv[]){

    string host;
    uint16_t port = 0;
    bool IPv4 = true;
    bool IPv6 = false;
    char position = '\0';
    bool isBot = false;
    
    // parsuje argumenty, na podstawie 
    if(parseArguments(argc, argv, host, port, IPv4, IPv6, position, isBot)){
        return 1;
    }

    // tworzę instancję klienta na podstawie podanych argumentów
    Klient klient(host, port, IPv4, IPv6, position, isBot);
    // uruchamiam klienta
    klient.run();

    // ewentualna jakaś obsługa błędów





    return 0;
}