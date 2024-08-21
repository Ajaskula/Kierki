#ifndef CARDS_H
#define CARDS_H

#include <string>
#include <iostream>
#include <vector>
#include "common.h"


enum class Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace, Unknown };

class Card{
    public:
        Card(char color, Rank rank);
        ~Card();
        char getColor() const;
        Rank getRank() const;
        static Rank string_to_rank(const std::string& rank);
        std::string rank_to_string(Rank rank) const;
        static Card string_to_card(const std::string& card);
        std::string to_string() const;

    private:
        char color;
        Rank rank;
};

class CardSet{
    public:
        CardSet(bool fullDeck = false);
        ~CardSet();
        void addCard(Card card);
        void addCards(const std::string& cards);
        void removeCard(Card card);
        Card getCard() const;
        int getSize() const;
        std::vector<Card> cards;
        bool isCardInSet(const Card& card) const;
        void add_cards(const std::string& cards);
        std::string print_cards_on_hand();
        Card get_card_of_color(char color = '0');
        // std::vector<std::string> extract_hand(const std::string& hand);
    private:
        void initializeFullDeck();
};





#endif