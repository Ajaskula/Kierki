#ifndef COMMON_H
#define COMMON_H

#include<string>
#include<vector>
#include<iostream>
#include<ctime>
#include<chrono>
#include<sstream>
#include<iomanip>
#include <arpa/inet.h>    // Dla inet_ntop
#include <netinet/in.h>   // Dla sockaddr_in
#include <sys/socket.h>   // Dla socket, connect
#include <unistd.h> 

std::string getCurrentTime();
std::string getServerAddress(int socket_fd);
std::string getLocalAddress(int socket_fd);
void raport(const std::string& addr1, const std::string& addr2, const std::string& message);

enum class Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace, Unknown };

class Card{
    public:
        enum class Color {Heart, Diamond, Club, Spade};
        // konstruktor karty
        Card(char color, Rank rank);
        // destruktor karty
        ~Card();
        // metoda zwracająca kolor karty
        char getColor() const;
        // metoda zwracająca wartość karty
        Rank getValue() const;
        // metoda konwertująca karty na string
        std::string to_string() const;
        Rank getRank() const;
        static Rank string_to_rank(const std::string& rank);
        void CardsSet::add_cards(const std::string& cards);

    private:
        // atrybuty karty
        char color;
        Rank rank;

        // metoda konwertująca wartość karty na string
        // metody z konst nie zmieniają stanu obiektu
        std::string rankToString(Rank rank) const;
        std::string colorToString(Color color) const;
        std::vector<std::string> CardsSet::extract_hand(const std::string& hand);

};


class CardSet{
    public:
        // konstruktor zbioru kart 
        CardSet(bool fullDeck = false);
        // destruktor zbioru kart
        ~CardSet();
        // metoda dodająca kartę do talii
        void addCard(Card card);
        void addCards(const std::string& cards);
        // metoda usuwająca kartę z talii
        void removeCard(Card card);
        // metoda zwracająca kartę z talii
        Card getCard() const;
        // metoda zwracająca ilość kart w talii
        int getSize() const;
        std::vector<Card> cards;
        bool isCardInSet(const Card& card) const;
    private:
        // atrybuty talii kart
        
        void initializeFullDeck();
};

#endif // COMMON_H