
// klasa reprezentująca kartę
class Card {
    public:
        enum class Color {Heart, Diamond, Club, Spade};
        enum class Rank { Two, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace };

        Card(char color, Rank rank)
        : color(color), rank(rank)
        {
            std::cout << "Instancja karty została stworzona\n";
        }
        ~Card(){
            std::cout << "Instancja karty została zniszczona\n";
        }
        Color getColor(){
            return color;
        }
        Rank getRank(){
            return rank;
        }
        std::string toString() const {

        }
    private:
        Color color;
        Rank rank;

        // metoda konwertująca kolor na string
        std::string rankToString(Rank rank){
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
        // metoda konwertująca kolor na string
        std::string colorToString(Color color){
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
};