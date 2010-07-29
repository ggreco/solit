#ifndef KGAME_H

#define KGAME_H

#define KLONDIKE_COLUMNS 12
#define KLONDIKE_TOTAL_DECKS 2
#define KLONDIKE_TOTAL_SEEDS (KLONDIKE_TOTAL_DECKS * 4)

#include "game.h"

class KlondikeRow : public Row
{
public:
        bool CanGet(const Card &c) 
        {
            if (Empty())
                return (c.Value() == 13);

            if (Get().Covered())
                return false;

            return (((Get().IsBlack() && c.IsRed()) ||
                     (Get().IsRed() && c.IsBlack()) ) &&
                      Get().Value() == (c.Value() + 1));
        }
};

class KlondikeGame : public Game
{

    Stackable cards_;
    KlondikeRow  rows_[KLONDIKE_COLUMNS];
    Seed seeds_[KLONDIKE_TOTAL_SEEDS];
    bool IsCompleted();

    void PressButton(const Point &);
    void ReleaseButton(const Point &);
    void DoubleClick(const Point &);
	void AutoComplete();
    void Update();
    void SetupCards();
    void Victory();
    void Restart();
	bool SeedsFull();

    void serialize(Serializer &s) {
        Game::serialize(s);

        s & cards_;
        s & rows_;
        s & seeds_;
    }
public:
    KlondikeGame(int id);
};

#endif

