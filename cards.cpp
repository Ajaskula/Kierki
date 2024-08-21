#include "cards.h"

Card::Card(char color, Rank rank) : color(color), rank(rank) {
}
Card::~Card(){
    std::cout << "Instancja karty została zniszczona\n";
}
// return rank of the card
Rank Card::getRank() const {
    return rank;
}
// return color of the card
char Card::getColor() const {
    return color;
}
// return string representation of the rank
std::string Card::rank_to_string(Rank rank) const {
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
            return "J";
        case Rank::Queen:
            return "Q";
        case Rank::King:
            return "K";
        case Rank::Ace:
            return "A";
        default:
            return "Unknown";
    }
}

// return rank of the card from string
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

// return card from string
Card Card::string_to_card(const std::string& card){
    return Card(card[card.length()-1], Card::string_to_rank(card.substr(0, card.length() - 1)));
}
// return string representation of the card
std::string Card::to_string() const {
    return rank_to_string(rank) + color;
}


std::string CardSet::print_cards_on_hand(){
    std::string cards_on_hand = "";
    for(auto card : cards){
        cards_on_hand += card.to_string() + ", ";
    }
    return cards_on_hand.substr(0, cards_on_hand.length() - 2);
}

Card CardSet::get_card_of_color(char color){
    for(auto card : cards){
        if(color == '0' || card.getColor() == color){
            return card;
        }
    }
    if(!cards.empty()){
        return cards[0];
    }
    return Card('0', Rank::Unknown);
}

// konstruktor talii z pełnym deckiem
CardSet::CardSet(bool fullDeck) : cards() {
    if(fullDeck){
        initializeFullDeck();
    }
}
bool CardSet::isCardInSet(const Card& card) const {
    for(auto c : cards){
        if(c.getColor() == card.getColor() && c.getRank() == card.getRank()){
            return true;
        }
    }
    return false;
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

void CardSet::add_cards(const std::string& cards){
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
