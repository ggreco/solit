#ifndef DECK_H

#define DECK_H

#include "stackable.h"
#include "card.h"

class Deck : public Stackable
{
    public:
        Deck();
        void Deal();
        Card GetCard() { Card c = Get(); Remove(); return c; }
};

#endif


