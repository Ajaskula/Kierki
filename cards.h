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
        // return vector of cards from string
        static std::vector<std::string> extractCardsVectorFromCardsStringStr(const std::string& hand);
        // return true if the string is valid card
        static bool isStringValidCard(const std::string& card);
        // return vector of cards from string
        static std::vector<Card> extractCardsVectorFromCardsString(const std::string& hand);

    private:
        char color;
        Rank rank;
};

class CardSet{
    public:
        CardSet(bool fullDeck = false);
        ~CardSet();
        // return card of the given color or first card if color is not given
        // if there is no card of the given color, return first card
        Card getCardOfColor(char color = '0');
        // return list representation of the cards on hand
        std::string getCardsOnHand();
        // add to the set cards from the string
        void addCardsFromCardsString(const std::string& cards);
        // remove card from the set
        void removeCard(Card card);
        // return true if the card is in the set
        // return false otherwise
        bool isCardInSet(const Card& card) const;

        std::vector<Card> cards;
    private:
        //add card to the set
        void addCard(Card card);
        // initialize full deck of cards
        void initializeFullDeck();
};

// class representing a deal
class Deal{
    public:
        Deal(char type, char firstPlayer, const std::string& dealN, const std::string& dealE, const std::string& dealS, const std::string& dealW);
        Deal();
        ~Deal();

        char type;
        char firstPlayer;
        std::string dealN;
        std::string dealE;
        std::string dealS;
        std::string dealW;
        // return type of the deal
        char getType();
        // return first player of the deal
        char getFirstPlayer();
    private:
};

// class representing a gameplay
class Gameplay{
    public:
        Gameplay(const std::string& filename);
        ~Gameplay();
        // return deal of the given number
        Deal getDeal(int dealNumber);
        // return number of deals
        int getNumberOfDeals();

    private:
        std::string filename;
        std::vector<Deal> dealsToPlay;
        // create deals from file
        void createDealsFromFile();

};



#endif