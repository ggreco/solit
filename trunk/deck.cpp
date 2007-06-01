#include "deck.h"
#include <algorithm>

Deck::
Deck(int decks) :
    decks_(decks)
{
	init();
}

void
Deck::init()
{
	cards_.clear();

	for (int j = 0; j < decks_; j++) {
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
	std::random_shuffle(cards_.begin(), cards_.end());

	std::random_shuffle(cards_.begin(), cards_.end());
}
