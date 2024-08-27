#include "common.h"


std::string get_current_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&now_time);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%Y-%m-%dT%H:%M:%S")    // Czas bez milisekund
        << '.' << std::setw(3) << std::setfill('0') << now_ms.count();  // Milisekundy

    return oss.str();
}
std::string get_server_address(int socket_fd) {
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
std::string get_local_address(int socket_fd) {
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
    std::cout << "raport: \n";
    std::string str =  "[" + addr1 + "," + addr2 +","+ get_current_time() + "] " + message;
    size_t length = str.length();
    std::cout.write(str.c_str(), length);
}

bool is_string_correct_card_list(const std::string& hand) {
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

        if (!Card::isStringValidCard(card)) {
            return false; // Znaleziono niepoprawną kartę
        }

        if (!unique_cards.insert(card).second) {
            return false; // Znaleziono duplikat
        }
    }
    return true;
}

bool is_valid_request_to_send_card(const std::string& input) {
    // Wyrażenie regularne do sprawdzenia formatu !karta
    std::regex pattern(R"(^!(10|[2-9]|[JQKA])[CSDH]$)");

    // Sprawdzenie, czy ciąg pasuje do wzorca
    return std::regex_match(input, pattern);
}
std::string get_busy_places_from_BUSY(const std::string& message){
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
std::string extract_card_list_from_taken(const std::string& message) {
    // Regularne wyrażenie do dopasowania komunikatu TAKEN, uwzględniając \r\n na końcu
    std::regex pattern(R"(TAKEN\d+((?:10|[2-9]|[JQKA])[HDCS]+)([NEWS])\r\n$)");
    std::smatch matches;
    std::cout << "FIXME: w extract_card_list message to " << message << std::endl;
    std::cout << "FIXME: length of message is " << message.length() << std::endl;
    if (std::regex_search(message, matches, pattern)) {
        std::string card_list = matches[1].str(); // Wyciągamy listę kart
        std::cout << "FIXME: w extract_card_list" << card_list << std::endl;
        std::string formatted_list;
        card_list = "AS2H3D4C5S6H7D8C9S10HJSQDKC";
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
char get_first_card_color_from_TRICK(const std::string& message) {
    // Zaktualizowane wyrażenie regularne, aby wyodrębnić kolor pierwszej karty
    std::regex pattern(R"(TRICK\d+(?:\d{1,2}|[JQKA])([CSDH]).*\r\n)");
    std::smatch matches;

    if (std::regex_search(message, matches, pattern)) {
        return matches[1].str()[0]; // matches[1] to kolor pierwszej karty
    }

    return '0'; // Zwraca '0', jeśli nie znaleziono dopasowania
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

std::string get_list_of_cards_from_TRICK(const std::string& trick, int current_trick) {

    std::vector<std::string> cards = Card::extractCardsVectorFromCardsStringStr(trick.substr(6 + (current_trick >= 10), trick.length() - 8 - (current_trick >= 10)));
    std::string cards_str;
    for(std::string card : cards){
        cards_str += card + ", ";
    }
    return cards_str.substr(0, cards_str.length() - 2);
}