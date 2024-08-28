#ifndef CARDS_H
#define CARDS_H

#include <string>
#include <iostream>
#include <vector>
#include "common.h"
#include <fstream>


enum class Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace, Unknown };

class Card{
    public:
        Card(char color, Rank rank);
        ~Card();
        // return rank of the card
        Rank getRank() const;
        // return color of the card
        char getColor() const;
        // return string representation of the rank
        std::string rankToString(Rank rank) const;
        // return rank of the card from string
        static Rank stringToRank(const std::string& rank);
        // return card from string
        static Card stringToCard(const std::string& card);
        // return string representation of the card
        std::string toString() const;
        static std::vector<std::string> extractCardsVectorFromCardsStringStr(const std::string& hand);
        static bool isStringValidCard(const std::string& card);
        static std::vector<Card> extractCardsVectorFromCardsString(const std::string& hand);

    private:
        char color;
        Rank rank;
};

class CardSet{
    public:
        CardSet(bool fullDeck = false);
        ~CardSet();
        // return list representation of the cards on hand
        std::string getCardsOnHand();
        // return card of the given color or first card if color is not given
        // if there is no card of the given color, return first card
        Card getCardOfColor(char color = '0');
        // return true if the card is in the set
        // return false otherwise
        bool isCardInSet(const Card& card) const;
        //add card to the set
        void addCard(Card card);
        // remove card from the set
        void removeCard(Card card);
        // add to the set cards from the string
        void addCardsFromCardsString(const std::string& cards);
        std::vector<Card> cards;
    private:
        // initialize full deck of cards
        void initializeFullDeck();
};

// klasa rozdania, definiowana poprzez elementy pliku
class Deal{
    public:
        Deal(char type, char firstPlayer, const std::string& dealN, const std::string& dealE, const std::string& dealS, const std::string& dealW);
        ~Deal();

        char type;
        char firstPlayer;
        std::string dealN;
        std::string dealE;
        std::string dealS;
        std::string dealW;
        char getType();
        char getFirstPlayer();
    private:
};

// klasa rozgrywki, definiowana
// poprzez plik
class Gameplay{
    // konstruktor i destruktor rozgrywki
    public:
        Gameplay(const std::string& filename);
        ~Gameplay();
        // zwraca rozdanie o wskazanym numerze
        Deal getDeal(int dealNumber);
        int getNumberOfDeals();

    private:
        // nazwa pliku, na podstawie, którego będzie tworzona rozgrywka
        std::string filename;
        std::vector<Deal> dealsToPlay;
        void createDealsFromFile();

};



#endif