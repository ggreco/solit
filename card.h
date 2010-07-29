#ifndef CARD_H

#define CARD_H

#include <string>
#include "serializer.h"

enum SeedValue {Hearts, Squares, Flowers, Pikes};
enum CardColor {Red, Black};

class Card : public Serializable
{
    int value_;
    int seed_;
    int deck_;
    bool covered_;

    void serialize(Serializer &s) {
        s & value_;
        s & seed_;
        s & deck_;
        s & covered_;
    }
public:
    Card() : value_(-1) {} // used only by serializer
    Card(int val, SeedValue seed, int deck) : 
        value_(val), seed_(seed), deck_(deck), covered_(true) {}
    bool IsRed() const { return (seed_ == Hearts || seed_ == Squares); };
    bool IsBlack() const { return !IsRed(); }
	CardColor Color() const { return ( (seed_ == Hearts || seed_ == Squares) ? Red : Black); }
    int Value() const { return value_; }
    SeedValue Seed() const { return (SeedValue)seed_; }
    bool Covered() const { return covered_; }
    void Covered(bool val) { covered_ = val; }
    void Deck(int d) { deck_ = d; }
    bool operator< (const Card &c) const { return (c.deck_ > deck_ ||
                                       (c.deck_ == deck_ && c.seed_ > seed_) ||
                                       (c.deck_ == deck_ && c.seed_ == seed_ && c.value_ > value_ )); }

    std::string str() const;
};


#endif
