#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <regex>
#include <set>
#include "klient.h"
#include "cards.h"
#include "common.h"
#include <netdb.h>
#include <poll.h>
#include <unordered_map>

int Klient::parseArguments(int argc, char *argv[], std::string& host, uint16_t& port, bool& IPv4, bool& IPv6, char& position, bool& isBot){
    for(int i = 1; i < argc; i++){
        std::string arg = argv[i];

        if(arg == "-h" && i + 1 < argc){
            host = argv[i + 1];
            i++;
        }else if(arg == "-p" && i + 1 < argc){
            port = std::stoi(argv[i + 1]);
            i++;
        }else if(arg == "-4"){
            if(IPv6 == false && IPv4 == false){
                IPv4 = true;
            }
        }else if(arg == "-6"){
            if(IPv4 == false && IPv6 == false){
                IPv6 = true;
            }
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
            return 1;
        }

    }
    if(IPv4 && IPv6){
        std::cerr << "Nie można jednocześnie używać IPv4 i IPv6\n";
        return 1;
    }
    if('\0' == position){
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
    std::cout << "Host: " << host << "\n";
    std::cout << "Port: " << port << "\n";
    std::cout << "IPv4: " << IPv4 << "\n";
    std::cout << "IPv6: " << IPv6 << "\n";
    std::cout << "Pozycja: " << position << "\n";
    std::cout << "Bot: " << isBot << "\n";
    
    return 0;
}
Klient::Klient(const std::string& host, uint16_t port, bool IPv4, bool IPv6,
        char position, bool isBot)
        : host(host), port(port), IPv4(IPv4), IPv6(IPv6), position(position), isBot(isBot), cardSet(), tricks_taken(), socket_fd(0),
        wait_DEAL(true), wait_TOTAL(true), wait_SCORE(true), wait_first_TRICK(true), got_TRICK(false), current_trick(1)
        {}
Klient::~Klient(){
            std::cout << "Instancja klienta została zniszczona\n";
        }

// sprawdza poprawność wiadomości BUSY
// sprawdzam w ten sposób, czy karta jest poprawna
// korzystjąc z wyrażeń regularnych sprawdzam poprawność karty
bool is_valid_card(const std::string& card){
    static const std::regex card_regex("^(10|[2-9]|[JQKA])[CDHS]$");
    return std::regex_match(card, card_regex);
}
// sprawdzam czy otrzymane karty są poprawne
bool is_valid_hand(const std::string& hand) {
    // zbiór do przechowywanie unikalnych kart
    std::set<std::string> unique_cards; // Zbiór do przechowywania unikalnych kart

    // przetwarzanie karty po karcie
    size_t i = 0;
    size_t hand_length = hand.length();
    int card_count = 0;

    // dopóki nie przejdę przez cały string
    while (i < hand_length) {
        std::string card;

        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            card = hand.substr(i, 3); // Karta to "10X"
            i += 3;
        } else if (i + 1 < hand_length) {
            card = hand.substr(i, 2); // Karta to "VX"
            i += 2;
        } else {
            return false; // Niekompletna karta na końcu stringa
        }

        if (!is_valid_card(card)) {
            return false; // Znaleziono niepoprawną kartę
        }

        if (!unique_cards.insert(card).second) {
            return false; // Znaleziono duplikat
        }

        card_count++;

        if (card_count > 13) {
            return false; // Zbyt wiele kart
        }
    }

    return card_count == 13; // Sprawdzamy, czy przetworzono dokładnie 13 kart
}
bool is_valid_card_list(const std::string& hand) {
    // zbiór do przechowywanie unikalnych kart
    std::set<std::string> unique_cards; // Zbiór do przechowywania unikalnych kart

    // przetwarzanie karty po karcie
    size_t i = 0;
    size_t hand_length = hand.length();
    // int card_count = 0;

    // dopóki nie przejdę przez cały string
    while (i < hand_length) {
        std::string card;

        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            card = hand.substr(i, 3); // Karta to "10X"
            i += 3;
        } else if (i + 1 < hand_length) {
            card = hand.substr(i, 2); // Karta to "VX"
            i += 2;
        } else {
            return false; // Niekompletna karta na końcu stringa
        }

        if (!is_valid_card(card)) {
            return false; // Znaleziono niepoprawną kartę
        }

        if (!unique_cards.insert(card).second) {
            return false; // Znaleziono duplikat
        }
    }
    return true;
}
bool Klient::validate_BUSY(const std::string& message){
    if(message.length() > 10 || message.length() < 7){
        return false;
    }
    if(message.substr(0, 4) != "BUSY"){
        return false;
    }
    for(std::string::size_type i = 4; i < message.length() - 2; i++){
        if(message[i] != 'N' && message[i] != 'S' && message[i] != 'E' && message[i] != 'W'){
            return false;
        }
    }
    if(message.substr(4).find(position) == std::string::npos){
        return false;
    }
    // każdy ze znaków N, S , E, W może wystąpić tylko raz
    for(std::string::size_type i = 4; i < message.length(); i++){
        if(message.substr(4).find(message[i]) != message.substr(4).rfind(message[i])){
            return false;
        }
    }
    return true;
}
// sprawdzam poprawność wiadomości DEAL
bool Klient::validate_DEAL(const std::string& message){
    
    // zła długość wiadomości
    if(message.length() < 34 || message.length() > 38){
        return false;
    }
    if(message.substr(0, 4) != "DEAL"){
        return false;
    }
    // sprawdzenie typu rozdania
    if (std::isdigit(message[4])) {
        int value = message[4] - '0'; // Convert char to int
        
        // Check if the value is within the desired range
        if (value < 1 || value > 7) {
            return false;
        }
    } else {
        return false;
    }

    // sprawdzenie pozycji klienta wychodzącego jako pierwszy
    if(message[5] != 'N' && message[5] != 'S' && message[5] != 'E' && message[5] != 'W'){
        return false;
    }

    // sprawdzenie czy lista kart jest poprawna
    if(is_valid_hand(message.substr(6, (message.length() - 8))) == false){
        return false;
    }
 
    return true;
}
bool Klient::validate_TRICK(const std::string& message){
    if(message.length() < 8 || message.length() > 18){
        return false;
    }
    if(message.substr(0, 5) != "TRICK"){
        return false;
    }
    if(!(message[5] == '1' && message[6] <= '3' && message[6] >= '0') && !(message[5] >= '1' && message[5] <= '9' && !isdigit(message[6]))){
        return false;
    }
    int trick_number = 0;
    if(message[5] == '1'){
        trick_number = 10;
        if (isdigit(message[6])){
            trick_number += message[6] - '0';
        }
    }else{
        trick_number = message[5] - '0';
    }
    if(trick_number != current_trick){
        return false;
    }

    // sprawdzenie listy kart, czy zawiera poprawne karty
    if(!is_valid_card_list(message.substr(7 - (trick_number >= 10), message.length() - 8 - (trick_number >= 10)))){
        return false;
    }
    std::vector<std::string> cards_vec = extract_hand(message.substr(7 - (trick_number >= 10), message.length() - 8 - (trick_number >= 10)));
    
    if(cards_vec.size() >= 4){
        return false;
    }

    // muszę jeszcze sprawdzić, czy nie posiadam takiej karty jak te wyłożone na stole
    for(std::vector<std::string>::size_type i = 0; i < cards_vec.size(); i++){
        if(cardSet.isCardInSet(Card::string_to_card(cards_vec[i]))){
            return false;
        }
    }
    return true;
}
bool Klient::validate_TAKEN(const std::string& message){

    // sprawdzenie długości
    if(message.length() < 17 || message.length() > 22){
        return false;
    }

    // sprawdzenie pierwszego słowa
    if(message.substr(0, 5) != "TAKEN"){
        return false;
    }

    // sprawdzenie numeru lewy
    if(!(message[5] == '1' && message[6] <= '3' && message[6] >= '0') && !(message[5] >= '1' && message[5] <= '9' && !isdigit(message[6]))){
        return false;
    }
    int trick_number = 0;
    if(message[5] == '1'){
        trick_number = 10;
        if (isdigit(message[6])){
            trick_number += message[6] - '0';
        }
    }else{
        trick_number = message[5] - '0';
    }
    if(trick_number != current_trick){
        return false;
    }

    if(!is_valid_card_list(message.substr(7 - (trick_number >= 10), message.length() - 9 - (trick_number >= 10)))){
        return false;
    }
    std::vector<std::string> cards_vec = extract_hand(message.substr(7 - (trick_number >= 10), message.length() - 9 - (trick_number >= 10)));
    int in_my_hand = 0;
    for(std::vector<std::string>::size_type i = 0; i < cards_vec.size(); i++){
        if(cardSet.isCardInSet(Card::string_to_card(cards_vec[i]))){
            in_my_hand++;
        }
    }
    if(in_my_hand != 1){
        return false;
    }

    if(message[message.length() - 3] != 'N' && message[message.length() - 3] != 'S' && message[message.length() - 3] != 'E' && message[message.length() - 3] != 'W'){
        return false;
    }
    
    return true;
}
bool Klient::validate_WRONG(const std::string& message){
    // sprawdzenie długości
    if(message.length() < 8 || message.length() > 9){
        return false;
    }
    if(message.substr(0, 5) != "WRONG"){
        return false;
    }
    // sprawdzenie numeru lewy
    if(!(message[5] == '1' && message[6] <= '3' && message[6] >= '0') && !(message[5] >= '1' && message[6] <= '9' && !isdigit(message[6]))){
        return false;
    }
    int trick_number = 0;
    if(message[5] == '1'){
        trick_number = 10;
        if (isdigit(message[6])){
            trick_number += message[6] - '0';
        }
    }else{
        trick_number = message[5] - '0';
    }
    if(trick_number != current_trick){
        return false;
    }

    return true;
}

bool Klient::validate_SCORE(const std::string& message){

    const std::regex pattern(R"(SCORE[NSEW]\d+[NSEW]\d+[NSEW]\d+[NSEW]\d+\r\n)");
    if (!std::regex_match(message, pattern)) {
        return false;
    }

    std::unordered_map<char, int> seat_count = {{'N', 0}, {'S', 0}, {'E', 0}, {'W', 0}};

    // Przejdź przez ciąg i zlicz wystąpienia liter N, S, E, W
    for (char ch : message) {
        if (seat_count.find(ch) != seat_count.end()) {
            seat_count[ch]++;
        }
    }

    // Sprawdź, czy każda litera wystąpiła dokładnie raz
    for (const auto& pair : seat_count) {
        if (pair.second != 1) {
            return false;
        }
    }

    return true;
}

bool Klient::validate_TOTAL(const std::string& message){

    const std::regex pattern(R"(TOTAL[NSEW]\d+[NSEW]\d+[NSEW]\d+[NSEW]\d+\r\n)");
    if (!std::regex_match(message, pattern)) {
        return false;
    }

    std::unordered_map<char, int> seat_count = {{'N', 0}, {'S', 0}, {'E', 0}, {'W', 0}};

    // Przejdź przez ciąg i zlicz wystąpienia liter N, S, E, W
    for (char ch : message) {
        if (seat_count.find(ch) != seat_count.end()) {
            seat_count[ch]++;
        }
    }

    // Sprawdź, czy każda litera wystąpiła dokładnie raz
    for (const auto& pair : seat_count) {
        if (pair.second != 1) {
            return false;
        }
    }

    return true;
}
int Klient::validate_message(const std::string& message){

    if(isBot){
        raport(getServerAddress(socket_fd), getLocalAddress(socket_fd), message);
    }
    if(validate_BUSY(message)){
        return BUSY;
    }
    if(validate_DEAL(message)){
        return DEAL;
    }
    if(validate_TRICK(message)){
        return TRICK;
    }
    if(validate_TAKEN(message)){
        return TAKEN;
    }
    if(validate_WRONG(message)){
        return WRONG;
    }
    if(validate_SCORE(message)){
        return SCORE;
    }
    if(validate_TOTAL(message)){
        return TOTAL;
    }

    return -1;
}

// konwertuje stringa do talii kart
std::vector<Card> cards_to_set(const std::string& hand) {
    std::vector<Card> card_set;

    size_t i = 0;
    size_t hand_length = hand.length();

    while (i < hand_length) {
        std::string rank_str;
        char color;

        if (i + 2 < hand_length && hand.substr(i, 2) == "10") {
            rank_str = "10"; // Ranga to "10"
            color = hand[i + 2]; // Kolor jest na pozycji 3 (indeks i + 2)
            i += 3;
        } else if (i + 1 < hand_length) {
            rank_str = hand.substr(i, 1); // Ranga to jeden znak (2-9, J, Q, K, A)
            color = hand[i + 1]; // Kolor jest na pozycji 2 (indeks i + 1)
            i += 2;
        } else {
            throw std::invalid_argument("Niekompletna karta w stringu");
        }

        Rank rank = Card::string_to_rank(rank_str);
        card_set.push_back(Card(color, rank));
    }

    return card_set;
}

// wyświetla historię wziętych lew
void Klient::print_trick_history(){
    std::cout << "Historia wziętych lew: ";
    for(const std::string& trick : tricks_taken){
        std::cout << trick << " ";
    }
    std::cout << "\n";
}
std::string Klient::print_busy_places(const std::string& message){
    std::string busy_places;;
    for(std::string::size_type i = 4; i < message.length() - 2; i++){
        busy_places += message[i];
        // pod warunkiem, że nie jest to ostatni znak
        if(i != message.length() - 3){
            busy_places += ", ";
        }
    }
    return busy_places;
}
std::string Klient::print_dealed_cards(const std::string& message){
    std::string dealed_cards;
    std::vector<std::string> cards_vec = extract_hand(message);
    for(std::vector<std::string>::size_type i = 0; i < cards_vec.size(); i++){
        dealed_cards += cards_vec[i];
        if(i != cards_vec.size() - 1){
            dealed_cards += ", ";
        }
    }
    return dealed_cards;
}
// wypisuje karty na ręku
void Klient::print_hand(){
    std::string hand;
    for(const Card& card : cardSet.cards){
        hand+= card.to_string() + ", ";
    }
    hand = hand.substr(0, hand.length() - 2);
    hand += "\n";
    std::cout << hand;
}
void Klient::print_taken_tricks(){
    // std::cout << "Wzięte lewy: ";
    for(const std::string& trick : tricks_taken){
        std::cout << trick << "\n";
    }
    // std::cout << "\n";
}

std::string extract_cards_from_TRICK(const std::string& input) {
    // Wyrażenie regularne do zidentyfikowania części z kartami
    std::regex pattern(R"(TRICK\d+((?:\d{1,2}|[JQKA])[CSDH]+)\r\n)");
    std::smatch matches;

    // Sprawdź, czy ciąg pasuje do wzorca i wyciągnij listę kart
    if (std::regex_search(input, matches, pattern) && matches.size() > 1) {
        std::string card_list = matches[1].str();
        std::string result;

        // Iteruj przez listę kart i formatuj je poprawnie
        for (size_t i = 0; i < card_list.size();) {
            std::string card;

            // Sprawdź, czy karta to "10"
            if (i + 1 < card_list.size() && card_list[i] == '1' && card_list[i + 1] == '0') {
                card = card_list.substr(i, 3); // karta "10X"
                i += 3;
            } else {
                card = card_list.substr(i, 2); // karta "VX"
                i += 2;
            }

            // Dodaj kartę do wyniku
            if (!result.empty()) {
                result += ", ";
            }
            result += card;
        }

        return result;
    }

    return ""; // Zwróć pusty ciąg, jeśli format nie pasuje
}

bool is_valid_card_format(const std::string& input) {
    // Wyrażenie regularne do sprawdzenia formatu !karta
    std::regex pattern(R"(^!(10|[2-9]|[JQKA])[CSDH]$)");

    // Sprawdzenie, czy ciąg pasuje do wzorca
    return std::regex_match(input, pattern);
}

char get_first_card_color(const std::string& message) {
    // Zaktualizowane wyrażenie regularne, aby wyodrębnić kolor pierwszej karty
    std::regex pattern(R"(TRICK\d+(?:\d{1,2}|[JQKA])([CSDH]).*\r\n)");
    std::smatch matches;

    if (std::regex_search(message, matches, pattern)) {
        return matches[1].str()[0]; // matches[1] to kolor pierwszej karty
    }

    return '0'; // Zwraca '0', jeśli nie znaleziono dopasowania
}

// Funkcja pomocnicza do dzielenia listy kart
std::string format_card_list(const std::string& card_list) {
    std::ostringstream formatted;
    for (size_t i = 0; i < card_list.length(); i += 2) {
        if (i > 0) {
            formatted << ", ";
        }
        formatted << card_list.substr(i, 2);  // Bierzemy dwuznakową kartę
    }
    return formatted.str();
}

std::string convert_taken_message(const std::string& taken_message) {
    // Ignorujemy końcowe \r\n, usuwamy "TAKEN"
    std::string clean_message = taken_message.substr(5, taken_message.length() - 7);

    // Wyciąganie numeru lewy
    size_t start = 0;
    size_t end = clean_message.find_first_not_of("0123456789");
    std::string trick_number = clean_message.substr(start, end - start);

    // Wyciąganie listy kart
    start = end;
    end = clean_message.find_first_of("NSWE", start);
    std::string card_list = clean_message.substr(start, end - start);
    
    // Formatowanie listy kart
    std::string formatted_card_list = format_card_list(card_list);

    // Wyciąganie pozycji gracza
    std::string player_position = clean_message.substr(end);

    // Tworzenie wynikowego komunikatu
    std::ostringstream result;
    result << "A trick " << trick_number << " is taken by " << player_position
           << ", cards " << formatted_card_list << ".";

    return result.str();
}
std::string convert_score_message(const std::string& message) {
    // Tworzymy wynikowy strumień tekstowy
    std::ostringstream result;

    // Dodajemy nagłówek
    result << "The scores are:\n";

    // Usuwamy prefiks "SCORE"
    std::string clean_message = message.substr(5, message.length() - 7);

    // Przetwarzamy poszczególne pary <miejsce><punkty>
    for (size_t i = 0; i < clean_message.length(); i += 3) {
        std::string position = clean_message.substr(i, 1);
        std::string points = clean_message.substr(i + 1, 2);  // Pobieramy dokładnie 2 znaki

        // Usuwamy wiodące zera z punktów
        int numeric_points = std::stoi(points);  // Konwertujemy na liczbę, co automatycznie usunie wiodące zera
        points = std::to_string(numeric_points); // Konwertujemy z powrotem na string

        // Dodajemy miejsce i punkty do wynikowego komunikatu
        result << position << " | " << points << "\n";
    }

    return result.str();
}
std::string convert_total_message(const std::string& message) {
    // Tworzymy wynikowy strumień tekstowy
    std::ostringstream result;

    // Dodajemy nagłówek
    result << "The total scores are:\n";

    // Usuwamy prefiks "TOTAL" i końcowe "\r\n"
    std::string clean_message = message.substr(5, message.length() - 7);

    // Przetwarzamy poszczególne pary <miejsce><punkty>
    for (size_t i = 0; i < clean_message.length(); i += 3) {
        std::string position = clean_message.substr(i, 1);
        std::string points = clean_message.substr(i + 1, 2);  // Pobieramy dokładnie 2 znaki

        // Usuwamy wiodące zera z punktów
        int numeric_points = std::stoi(points);  // Konwertujemy na liczbę, co automatycznie usunie wiodące zera
        points = std::to_string(numeric_points); // Konwertujemy z powrotem na string

        // Dodajemy miejsce i punkty do wynikowego komunikatu
        result << position << " | " << points << "\n";
    }

    return result.str();
}
void Klient::perform_taken(const std::string& message){
    
    // jeśli ja biorę lewę to dodaje do wziętę karty do historii lew
    std::string card_list = extract_card_list_from_taken(message);
    if(position == message[message.length() - 3]){
        // TODO dołóż kartę do historii lew
        tricks_taken.push_back(card_list);
    }
    std::vector<std::string> card_vector = extract_card_vector_from_taken(message);
    // usuwam z ręki kartę którą wziąłem
    for(std::string card : card_vector){
        cardSet.removeCard(Card::string_to_card(card));
    }

}
std::string extract_card_list_from_taken(const std::string& message) {
    // Regularne wyrażenie do dopasowania komunikatu TAKEN
    std::regex pattern(R"(TAKEN\d+((?:10|[2-9]|[JQKA])[HDCS]+)([NEWS])$)");
    std::smatch matches;

    if (std::regex_search(message, matches, pattern)) {
        std::string card_list = matches[1].str(); // Wyciągamy listę kart
        std::string formatted_list;
        
        // Przetwarzanie kart i formatowanie listy
        for (size_t i = 0; i < card_list.length(); i += 2) {
            if (i > 0) {
                formatted_list += ", ";
            }
            formatted_list += card_list.substr(i, (card_list[i] == '1') ? 3 : 2);
            i += (card_list[i] == '1'); // Jeśli karta to "10", zwiększamy indeks o 1 więcej
        }

        return formatted_list;
    }

    return ""; // Zwracamy pusty string, jeśli komunikat nie pasuje do wzorca
}
std::vector<std::string> extract_card_vector_from_taken(const std::string& message) {
    // Regularne wyrażenie do dopasowania komunikatu TAKEN
    std::regex pattern(R"(TAKEN\d+((?:10|[2-9]|[JQKA])[HDCS]+)([NEWS])$)");
    std::smatch matches;
    std::vector<std::string> card_vector;

    if (std::regex_search(message, matches, pattern)) {
        std::string card_list = matches[1].str(); // Wyciągamy listę kart

        // Przetwarzanie kart i dodawanie ich do wektora
        for (size_t i = 0; i < card_list.length(); i += 2) {
            std::string card = card_list.substr(i, (card_list[i] == '1') ? 3 : 2);
            card_vector.push_back(card);
            i += (card_list[i] == '1'); // Jeśli karta to "10", zwiększamy indeks o 1 więcej
        }
    }

    return card_vector;
}


int Klient::run(){

    socket_fd = connect_to_server();
    if(socket_fd == -1){
        std::cerr << "Nie udało się nawiązać połączenia\n";
        return 1;
    }
    std::string local_address = getLocalAddress(socket_fd);
    std::string server_address = getServerAddress(socket_fd);

    std::string message = std::string("IAM") + position + "\r\n";
    send_message(socket_fd, message);
    
    struct pollfd fds[2]; fds[0].fd = socket_fd;  fds[0].events = POLLIN; fds[1].fd = STDIN_FILENO;  fds[1].events = POLLIN;
    char buffer[1024]; memset(buffer, 0, 1024); size_t buffer_index = 0;

    while(true){

        for(int i = 0; i < 2; i++){ fds[i].revents = 0; }
        int poll_count = poll(fds, 2, -1); // Czekamy na zdarzenie
        if (poll_count < 0) {std::cerr << "Poll error" << std::endl;close(socket_fd);return 1;}

        // odczytanie wiadomości od serwera
        if(fds[0].revents & POLLIN){
            ssize_t bytes_received = read(socket_fd, &buffer[buffer_index], 1);

            if(bytes_received == -1){
                std::cerr << "Błąd podczas odbierania wiadomości\n";
                close(socket_fd);
                return 1;
            }else if(bytes_received == 0){
                std::cerr << "Serwer zamknął połączenie z klientem\n";
                close(socket_fd);
                if(wait_TOTAL || wait_SCORE){
                    std::cout<< "rozgrywka zakończona nieprawidłowo\n";
                    return 1;
                }else{
                    std::cout << "rozgrywka zakończona poprawnie\n";
                    return 0;
                }
            }
            char received_char = buffer[buffer_index];
            buffer_index++;
            // jeśli odebrany bajt to znak nowej linni
            if(received_char == '\n'){
                if(buffer_index > 1 && buffer[buffer_index - 2] == '\r'){
                    std::string message(buffer, buffer_index);
                    if(isBot){
                        raport(server_address, local_address, message);
                    }
                    // jeśli jestem na etapie oczekiwania na wiadomość DEAL lub BUSY
                    if(wait_DEAL){
                        if(validate_message(message) == BUSY){
                            if(!isBot){
                                std::cout << "Place busy, list of busy places received: " + print_busy_places(message) + ".\n";
                            }

                        }else if(validate_message(message) == DEAL){
                            if(!isBot){
                                std::cout << "New deal " << message[4] << ": staring place " << message[5] << ", your cards: " << print_dealed_cards(message.substr(6, message.length() - 8)) <<".\n";
                            }
                            // dodaje karty do tali
                            cardSet.add_cards(message.substr(6, message.length() - 8));
                            wait_DEAL = false;
                            wait_SCORE = true;
                            wait_TOTAL = true;
                            wait_first_TRICK = true;
                            got_TRICK = false;
                            current_trick = 1;
                        }
                    }else if(current_trick < 14){

                        if(validate_message(message) == TRICK){
                            got_TRICK = true;
                            wait_first_TRICK = false;

                            // wypisuje instrukcję dla zawodnika
                            if(!isBot){
                                std::cout << "Trick: (" << current_trick << ") " + extract_cards_from_TRICK(message) + "\n";
                                std::cout << "Available: " + cardSet.print_cards_on_hand() + "\n";
                            }else{
                                
                                Card card = cardSet.get_card_of_color(get_first_card_color(message));
                                std::string to_send = "TRICK" + std::to_string(current_trick) + card.rank_to_string(card.getRank()) + card.getColor() + "\r\n";
                                send_message(socket_fd, to_send);
                            }
                        }else if(validate_message(message) == TAKEN){
                            
                            // tylko jeśli czekam wciąż na pierwszego tricka
                            if(wait_first_TRICK || got_TRICK){
                                if(!isBot){
                                    std::cout << convert_taken_message(message) << "\n";
                                }
                                //TODO implement perform TAKEN (jeśli ja biorę lewę to biorę, ale dodatkowo usuwam na pewno kartę z ręki)
                                current_trick++;
                                got_TRICK = false;
                            
                            }else if(validate_message(message) == WRONG){
                                if(!isBot){
                                    std::cout << "Wrong message received in trick " << message[5] << ".\n";
                                }
                            }
                    
                    // tutaj powinienem otrzymać SCORE i TOTAL
                    // tutaj jest okej, bo mogę to otrzymać jedynie gdy curr_trick == 14
                        }else{
                            if(validate_message(message) == SCORE){
                                // w przypadku tej implementacj też chyba nic nie robię
                                if(!isBot){
                                    std::cout << convert_score_message(message) << "\n";
                                }
                                wait_SCORE = false;

                            }else if(validate_message(message) == TOTAL){
                                if(!isBot){
                                    std::cout << convert_total_message(message) << "\n";
                                }
                                wait_TOTAL = false;
                            }
                            if(!wait_TOTAL && !wait_SCORE){
                                wait_DEAL = true;
                            }
                        }
                    // otrzymałem faktycznie wiadomość
                    buffer_index = 0;
                    memset(buffer, 0, 1024);
                    }
                }
            }
            // jeśli przekroczyłem limit długości wiadomości
            if(buffer_index == MESSAGE_LIMIT){
                buffer[buffer_index] = '\r';
                buffer[buffer_index + 1] = '\n';
                if(isBot){
                    raport(server_address, local_address, buffer);
                }
                buffer_index = 0;
                memset(buffer, 0, 1024);
            }
        }
        // odczytanie wiadomości z stdin
        if(fds[1].revents & POLLIN){
            std::string message;
            std::cin >> message;
            if("cards" == message){
                print_hand();
            }
            if("tricks" == message){
                print_taken_tricks();
            }
            if(is_valid_card_format(message)){
                std::string to_send = "TRICK" + std::to_string(current_trick) + message.substr(1) + "\r\n";
                send_message(socket_fd, to_send);
            }

        }
    }
    return 0;
}

void Klient::send_message(int socket_fd, const std::string &message){
    size_t length = message.length();
    ssize_t bytes_sent = send(socket_fd, message.c_str(), length, 0);

    if(bytes_sent == -1){
        std::cerr << "Błąd podczas wysyłania wiadomości\n";
        close(socket_fd);
        exit(1);
    }
    if(isBot){
        raport(getLocalAddress(socket_fd), getServerAddress(socket_fd), message);
    }
}
int Klient::connect_to_server(){

            std::cout << "Próba połączenia z serwerem " << host << " na porcie " << port << "\n";
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof (hints)); 

            if(IPv4){
                hints.ai_family = AF_INET;
            }else if(IPv6){
                hints.ai_family = AF_INET6;
            }else{
                hints.ai_family = AF_UNSPEC;
            }
            hints.ai_socktype = SOCK_STREAM;

            if(getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0){
                std::cerr << "Nie udało się uzyskać adresów\n";
                return -1;
            }

            int socket_fd;
            for(struct addrinfo *p = res; p != NULL; p = p->ai_next){
                
                socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if(socket_fd == -1){
                    continue;
                }
                if(connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1){
                    close(socket_fd);
                    continue;
                }
                freeaddrinfo(res);
                return socket_fd;
            }
            
            freeaddrinfo(res);
            return -1;
}

