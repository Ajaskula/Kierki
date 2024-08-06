

// implementacja metod klasowych klasy Card
Card::Card(char color, Rank rank) : color(color), rank(rank) {
    std::cout << "Instancja karty została stworzona\n";
}
Card::~Card(){
    std::cout << "Instancja karty została zniszczona\n";
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
std::string Card::colorToString(Color color) const {
    switch(color){
        case Color::Heart:
            return "Heart";
        case Color::Diamond:
            return "Diamond";
        case Color::Club:
            return "Club";
        case Color::Spade:
            return "Spade";
        default:
            return "Unknown";
    }
}

std::string Card::toString() const {
    return rankToString(rank) + " of " + colorToString(color);
}
Card::Rank Card::getRank() const {
    return rank;
}
Card::Color Card::getColor() const {
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
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 13; j++){
            cards.push_back(Card(static_cast<Card::Color>(i), static_cast<Card::Rank>(j)));
        }
    }
}

// dodaje kartę do zbioru kart
void CardSet::addCard(Card card){
    cards.push_back(card);
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