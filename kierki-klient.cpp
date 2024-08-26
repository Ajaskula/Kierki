#include "common.h"
#include "klient.h"
#include "cards.h"

int main(int argc, char *argv[]){


    // tutaj spróbuje zaimplementować metodę parseArguments
    std::cout << "Uruchomienie klienta" << std::endl;

    std::string host;
    uint16_t port = 0;
    bool IPv4 = false;
    bool IPv6 = false;
    char position = '\0';
    bool isBot = false;

    if(Klient::parseArguments(argc, argv, host, port, IPv4, IPv6, position, isBot) == 1){
        std::cout << "Podane parametry są niepoprawne\n";
        return 1;
    }

    Klient klient(host, port, IPv4, IPv6, position, isBot);
    int ret_value = klient.run();
    std::cout << "Zakończenie klienta" << std::endl;


    return ret_value;
}