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

    // wysłanie wiadmości powitalnej do serwera
    send_message(socket_fd, message);

    // w tym miejscu klient czeka na odpowiedź od serwera
    // możliwe odpowiedzi to:
    // - busy i zakończenie połączenia, jeśli miejsce przy stole jest zajęte
    // zamknięcie połączenia, jeśli serwer uznał, że wiadomość od klienta była błędna
    // Deal i rozpoczęcie rozgrywki
    // wszystkie inne rzeczy są ignorowane

    const size_t buffer_size = 1024;
    char buffer[buffer_size];

    ssize_t bytes_received = recv(socket_fd, buffer, buffer_size - 1, 0);

    // jeśli wystąpił błąd podczas 
    if(bytes_received == -1){
        std::cerr << "Błąd podczas odbierania wiadomości\n";
        close(socket_fd);
        return -1;
    }else if(bytes_received == 0){
        std::cerr << "Serwer zamknął połączenie\n";
        close(socket_fd);
        return -1;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Otrzymano wiadomość: " << buffer << "\n";

    // otrzymałem jakąś wiadmość, teraz chcę ją przeanalizować

    std::string accumulated_data;
    size_t pos = 0;

    // dopóki w buforze nie znajdę braku dopasowania
    while((pos = accumulated_data.find("\r\n")) != std::string::npos){

        message = 
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


