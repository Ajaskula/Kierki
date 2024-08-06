#ifndef COMMON_H
#define COMMON_H

class Card{
    public:
        // konstruktor karty
        Card(char color, char value);
        // destruktor karty
        ~Card();
        // metoda zwracająca kolor karty
        char getColor();
        // metoda zwracająca wartość karty
        char getValue();
    private:
        // atrybuty karty
        char color;
        char value;
};


#endif // COMMON_H