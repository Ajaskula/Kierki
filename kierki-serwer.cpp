#include <iostrim>
#include <string>
#include "server.h"

int main(int argc, char *argv[]){

    int port = 0;
    std::string file;
    int timeout = 5;

    if(parseArguments(argc, argv, port, file, timeout) != 0){
        return 1;
    }

    // inicjalizacja instancji serwera
    Server server(port, file, timeout);

    server.run();




    return 0;
}