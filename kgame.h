#ifndef KGAME_H

#define KGAME_H

#define KLONDIKE_COLUMNS 12
#define KLONDIKE_TOTAL_DECKS 2
#define KLONDIKE_TOTAL_SEEDS (KLONDIKE_TOTAL_DECKS * 4)

#include "game.h"

class KlondikeGame : public Game
{

    Stackable cards_;
    Row  rows_[KLONDIKE_COLUMNS];
    Seed seeds_[KLONDIKE_TOTAL_SEEDS];
    bool IsCompleted();

    void PressButton(const Point &);
    void ReleaseButton(const Point &);
    void DoubleClick(const Point &);
	void KeyRelease(char key);
	void AutoComplete();
    void Update();
    void Victory();
    void Restart();
	bool SeedsFull();
public:
    KlondikeGame(int id);
};

#endif

