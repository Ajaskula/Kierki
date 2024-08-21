#include "common.h"

std::vector<std::string> extract_hand(const std::string& hand) {
    std::vector<std::string> cards; // Wektor do przechowywania kart
    size_t i = 0;
    size_t hand_length = hand.length();

    while (i < hand_length) {
        std::string card;

        // Sprawdzenie, czy karta to "10" + kolor (3 znaki)
        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            card = hand.substr(i, 3); // Wyodrębnij "10X"
            i += 3;
        } else {
            // Każda inna karta to jeden znak liczbowy + jeden znak koloru (2 znaki)
            card = hand.substr(i, 2); // Wyodrębnij "VX"
            i += 2;
        }

        // Dodaj kartę do wektora
        cards.push_back(card);
    }

    return cards; // Zwróć wektor kart
}



// funkcja, zwracająca aktualny czas w formacie ISO 8601
std::string getCurrentTime() {
    // Pobranie aktualnego czasu
    auto now = std::chrono::system_clock::now();

    // Konwersja do czasu kalendarzowego
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Konwersja do struktury tm dla lokalnego czasu
    std::tm local_time = *std::localtime(&now_time);

    // Wyciągnięcie milisekund
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Użycie stringstream do stworzenia formatowanego stringa
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%dT%H:%M:%S")    // Czas bez milisekund
        << '.' << std::setw(3) << std::setfill('0') << now_ms.count();  // Milisekundy

    return oss.str();
}

std::string getServerAddress(int socket_fd) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    // Pobieranie adresu serwera
    if (getpeername(socket_fd, (struct sockaddr*)&addr, &addr_len) != 0) {
        perror("getpeername failed");
        return "";
    }

    char ip_str[INET6_ADDRSTRLEN];
    std::string server_address;

    // Obsługa IPv4
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
        server_address = ip_str;
        server_address += ":" + std::to_string(ntohs(s->sin_port));
    } 
    // Obsługa IPv6
    else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));
        server_address = ip_str;
        server_address += ":" + std::to_string(ntohs(s->sin6_port));
    }

    return server_address;
}
// zwraca lokalny adres gniazda
std::string getLocalAddress(int socket_fd) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);

    // Pobieranie lokalnego adresu
    if (getsockname(socket_fd, (struct sockaddr*)&addr, &addr_len) != 0) {
        perror("getsockname failed");
        return "";
    }

    char ip_str[INET6_ADDRSTRLEN];
    std::string local_address;

    // Obsługa IPv4
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ip_str, sizeof(ip_str));
        local_address = ip_str;
        local_address += ":" + std::to_string(ntohs(s->sin_port));
    } 
    // Obsługa IPv6
    else if (addr.ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ip_str, sizeof(ip_str));
        local_address = ip_str;
        local_address += ":" + std::to_string(ntohs(s->sin6_port));
    }
    return local_address;
}
void raport(const std::string& addr1, const std::string& addr2, const std::string& message){
    std::string str =  "[" + addr1 + "," + addr2 +","+ getCurrentTime() + "] " + message;
    size_t length = str.length();
    std::cout.write(str.c_str(), length);
}