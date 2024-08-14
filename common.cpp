

// implementacja metod klasowych klasy Card
Card::Card(char color, Rank rank) : color(color), rank(rank) {
    std::cout << "Instancja karty została stworzona\n";
}
Card::~Card(){
    std::cout << "Instancja karty została zniszczona\n";
}

// konwersja wartości karty na string
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

// zamienia podany string na wartość typu rank
std::string_to_rank(const std::string& rank) const {
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

void addCards(const std::string& cards){
    for(int i = 0; i < cards.length(); i+=2){
        Card card(static_cast<Card::Color>(cards[i]), static_cast<Card::Rank>(cards[i+1]));
        addCard(card);
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