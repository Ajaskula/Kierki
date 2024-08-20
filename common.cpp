#include "common.h"
// implementacja metod klasowych klasy Card
Card::Card(char color, Rank rank) : color(color), rank(rank) {
    std::cout << "Instancja karty została stworzona\n";
}
Card::~Card(){
    std::cout << "Instancja karty została zniszczona\n";
}

// konwersja wartości karty na string
std::string Card::rankToString(Rank rank) const {
    switch(rank){
        case Rank::Two:
            return "2";
        case Rank::Three:
            return "3";
        case Rank::Four:
            return "4";
        case Rank::Five:
            return "5";
        case Rank::Six:
            return "6";
        case Rank::Seven:
            return "7";
        case Rank::Eight:
            return "8";
        case Rank::Nine:
            return "9";
        case Rank::Ten:
            return "10";
        case Rank::Jack:
            return "Jack";
        case Rank::Queen:
            return "Queen";
        case Rank::King:
            return "King";
        case Rank::Ace:
            return "Ace";
        default:
            return "Unknown";
    }
}

// zamienia podany string na wartość typu rank
Rank Card::string_to_rank(const std::string& rank) {
    if(rank == "2"){
        return Rank::Two;
    }else if(rank == "3"){
        return Rank::Three;
    }else if(rank == "4"){
        return Rank::Four;
    }else if(rank == "5"){
        return Rank::Five;
    }else if(rank == "6"){
        return Rank::Six;
    }else if(rank == "7"){
        return Rank::Seven;
    }else if(rank == "8"){
        return Rank::Eight;
    }else if(rank == "9"){
        return Rank::Nine;
    }else if(rank == "10"){
        return Rank::Ten;
    }else if(rank == "J"){
        return Rank::Jack;
    }else if(rank == "Q"){
        return Rank::Queen;
    }else if(rank == "K"){
        return Rank::King;
    }else if(rank == "A"){
        return Rank::Ace;
    }else{
        return Rank::Unknown;
    }
}

std::string Card::to_string() const {
    return rankToString(rank) + " of " + color;
}
Rank Card::getRank() const {
    return rank;
}
char Card::getColor() const {
    return color;
}

// konstruktor talii z pełnym deckiem
CardSet::CardSet(bool fullDeck) : cards() {
    if(fullDeck){
        initializeFullDeck();
    }
}

CardSet::~CardSet(){
    std::cout << "Instancja talii została zniszczona\n";
}

void CardSet::initializeFullDeck(){
    char colors[] = {'H', 'D', 'C', 'S'};
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 13; j++){
            cards.push_back(Card(colors[i], static_cast<Rank>(j)));
        }
    }
}

// dodaje kartę do zbioru kart
void CardSet::addCard(Card card){
    cards.push_back(card);
}


std::vector<std::string> CardsSet::extract_hand(const std::string& hand) {
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

    return cards; // Zwróć wektor 13 kart
}

void CardsSet::add_cards(const std::string& cards){
    std::vector<std::string> extracted_cards = extract_hand(cards);
    for(auto card : extracted_cards){
        addCard(Card(card[card.length()-1], Card::string_to_rank(card.substr(0, card.length() - 1))));
    }
}


// usuwa kartę z talii
void CardSet::removeCard(Card card){
    for(auto it = cards.begin(); it != cards.end(); it++){
        if(it->getColor() == card.getColor() && it->getRank() == card.getRank()){
            cards.erase(it);
            break;
        }
    }
}

// zwraca ostatnią kartę z talii
Card CardSet::getCard() const {
    return cards.back();
}
// zwraca rozmiar kolekcji kart
int CardSet::getSize() const {
    return cards.size();
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