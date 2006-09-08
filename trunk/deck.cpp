#include "deck.h"

Deck::
Deck()
{
    for (int j = 0; j < 2; j++) {
        for (int i = 1; i < 14; i++) {
            Card c1(i, Hearts, j);
            Card c2(i, Squares, j);
            Card c3(i, Pikes, j);
            Card c4(i, Flowers, j);

            Add(c1); Add(c2); Add(c3); Add(c4);
        }
    }
}

void Deck::
Deal()
{
    int s = cards_.size();
    Card *temp[s];
    Cards new_;
    int counter  = 0;

    for (int i = 0; i < s; ++i)
        temp[i] = NULL;

    for (CardIterator it = cards_.begin(); 
            it != cards_.end(); ++it) {
        int pos = random() % s;

        if (counter < (s/2)) {
            while (temp[pos])
                pos = random() % s;
        }
        else {
            pos = 0;

            while (temp[pos])
                pos++;
        }

        temp[pos] = &(*it);
        counter++;
    }

    for (int i = 0; i < s; ++i)
        new_.push_back(*temp[i]);

    cards_ = new_;
}
