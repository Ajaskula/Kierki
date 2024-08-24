#include <iostream>
#include <string>
#include "server.h"

int main(int argc, char *argv[]){

    uint16_t port = 0;
    std::string file = "";
    int timeout = 5;

    if(Server::parseArguments(argc, argv, port, file, timeout) == 1){
        return 1;
        std::cerr << "Podane parametry są niepoprawne\n";
    }
    Server server(port, file, timeout);
    
    // return ret_value;
    return 0;
}