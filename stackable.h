#ifndef STACKABLE_H

#define STACKABLE_H

#include <stack>
#include "card.h"

typedef std::deque<Card> Cards;
typedef std::deque<Card>::iterator CardIterator;
typedef std::deque<Card>::const_iterator ConstCardIterator;

class Stackable
{
protected:
    Cards cards_;
public:
    int Size() const { return cards_.size(); }
    bool Empty() const { return cards_.empty(); }
    Cards &GetCards() { return cards_; }
    const Cards &GetCards() const { return cards_; }
    void Add(const Card &c) { cards_.push_back(c); }
    void AddFirst(const Card &c) { cards_.push_front(c); }
    const Card &Get() const { return cards_.back(); }
    Card &Get() { return cards_.back(); }
    Card &First() { return cards_.front(); }
    void Remove() { cards_.pop_back(); }
    void RemoveFirst() { cards_.pop_front(); }
};

#endif
