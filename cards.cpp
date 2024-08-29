#include "cards.h"
#include <regex>

Card::Card(char color, Rank rank) : color(color), rank(rank) {
}
Card::~Card(){
}
Rank Card::getRank() const {
    return rank;
}
char Card::getColor() const {
    return color;
}
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
Rank Card::stringToRank(const std::string& rank) {
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
Card Card::stringToCard(const std::string& card){
    return Card(card[card.length()-1], Card::stringToRank(card.substr(0, card.length() - 1)));
}
std::string Card::toString() const {
    return rankToString(rank) + color;
}
std::vector<std::string> Card::extractCardsVectorFromCardsStringStr(const std::string& hand) {
    std::vector<std::string> cards;
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
bool Card::isStringValidCard(const std::string& card){
    static const std::regex card_regex("^(10|[2-9]|[JQKA])[CDHS]$");
    return std::regex_match(card, card_regex);
}
std::vector<Card> Card::extractCardsVectorFromCardsString(const std::string& hand) {
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

        Rank rank = Card::stringToRank(rank_str);
        card_set.push_back(Card(color, rank));
    }

    return card_set;
}


CardSet::CardSet(bool fullDeck) : cards() {
    if(fullDeck){
        initializeFullDeck();
    }
}
CardSet::~CardSet(){}

std::string CardSet::getCardsOnHand(){
    std::string cards_on_hand = "";
    for(auto card : cards){
        cards_on_hand += card.toString() + ", ";
    }
    return cards_on_hand.substr(0, cards_on_hand.length() - 2);
}
Card CardSet::getCardOfColor(char color){
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
bool CardSet::isCardInSet(const Card& card) const {
    for(auto c : cards){
        if(c.getColor() == card.getColor() && c.getRank() == card.getRank()){
            return true;
        }
    }
    return false;
}
void CardSet::initializeFullDeck(){
    char colors[] = {'H', 'D', 'C', 'S'};
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 13; j++){
            addCard(Card(colors[i], static_cast<Rank>(j)));
        }
    }
}
void CardSet::addCard(Card card){
    cards.push_back(card);
}
void CardSet::removeCard(Card card){
    for(auto it = cards.begin(); it != cards.end(); it++){
        if(it->getColor() == card.getColor() && it->getRank() == card.getRank()){
            cards.erase(it);
            break;
        }
    }
}
void CardSet::addCardsFromCardsString(const std::string& cards){
    std::vector<std::string> extracted_cards = Card::extractCardsVectorFromCardsStringStr(cards);
    for(auto card : extracted_cards){
        addCard(Card(card[card.length()-1], Card::stringToRank(card.substr(0, card.length() - 1))));
    }
}

Gameplay::Gameplay(const std::string& filename) : filename(filename), dealsToPlay() {
    
    createDealsFromFile();
}
Gameplay::~Gameplay(){
}

// funkcja, która tworzy rozgrywkę na podstawie pliku
// wczytuje odpowiednie rozdania
void Gameplay::createDealsFromFile(){
    std::cout << "Creating deals from file: " << filename << std::endl;
    std::ifstream file(filename);
    std::string line;

    while(std::getline(file, line)){
        if(line.empty()){
            continue;
        }
        char type = line[0];
        char firstPlayer = line[1];

        std::string dealN, dealE, dealS, dealW;
        std::getline(file, dealN);
        std::getline(file, dealE);
        std::getline(file, dealS);
        std::getline(file, dealW);

        Deal deal(type, firstPlayer, dealN, dealE, dealS, dealW);
        dealsToPlay.push_back(deal);
    }
}

// Deal::Deal(const Deal& other)
//     : type(other.type), firstPlayer(other.firstPlayer), dealN(other.dealN), dealE(other.dealE), dealS(other.dealS), dealW(other.dealW) {
//     std::cout << "Copy constructor called" << std::endl;
// }
Deal::Deal() : type(' '), firstPlayer(' '), dealN(""), dealE(""), dealS(""), dealW("") {
        std::cout << "Default constructor called" << std::endl;
        }
Deal::Deal(char type, char firstPlayer, const std::string& dealN, const std::string& dealE, const std::string& dealS, const std::string& dealW) : type(type), firstPlayer(firstPlayer), dealN(dealN), dealE(dealE), dealS(dealS), dealW(dealW) {
    std::cout << "Creating deal of type: " << type << " with first player: " << firstPlayer << std::endl;
    std::cout << "Deal for N: " << dealN << std::endl;
    std::cout << "Deal for E: " << dealE << std::endl;
    std::cout << "Deal for S: " << dealS << std::endl;
    std::cout << "Deal for W: " << dealW << std::endl;
}
Deal::~Deal(){
}
Deal Gameplay::getDeal(int dealNumber){
    return dealsToPlay[dealNumber];
}
int Gameplay::getNumberOfDeals(){
    return dealsToPlay.size();
}

char Deal::getType(){
    return type;
}
char Deal::getFirstPlayer(){
    return firstPlayer;
}