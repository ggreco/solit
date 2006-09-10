#ifndef CARD_H

#define CARD_H

#include <string>

enum SeedValue {Hearts, Squares, Flowers, Pikes};

class Card
{
    int value_;
    SeedValue seed_;
    int deck_;
    bool covered_;
public:
    Card(int val, SeedValue seed, int deck) : 
        value_(val), seed_(seed), deck_(deck), covered_(true) {}
    bool IsRed() const { return (seed_ == Hearts || seed_ == Squares); };
    bool IsBlack() const { return !IsRed(); }
    int Value() const { return value_; }
    SeedValue Seed() const { return seed_; }
    bool Covered() const { return covered_; }
    void Covered(bool val) { covered_ = val; }
    void Deck(int d) { deck_ = d; }
    bool operator< (const Card &c) const { return (c.deck_ > deck_ ||
                                       (c.deck_ == deck_ && c.seed_ > seed_) ||
                                       (c.deck_ == deck_ && c.seed_ == seed_ && c.value_ > value_ )); }

    std::string str() const;
};


#endif
