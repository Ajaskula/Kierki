#include<string>
#include<vector>
#include<iostream>
#ifndef COMMON_H
#define COMMON_H


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
        std::string toString() const;
    private:
        // atrybuty karty
        char color;
        Rank rank;

        // metoda konwertująca wartość karty na string
        // metody z konst nie zmieniają stanu obiektu
        std::string rankToString(Rank rank) const;
        Rank string_to_rank(const std::string& rank) const;
        std::string colorToString(Color color) const;
        Rank getRank() const;
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
    private:
        // atrybuty talii kart
        std::vector<Card> cards;
        
        void initializeFullDeck();
};

#endif // COMMON_H