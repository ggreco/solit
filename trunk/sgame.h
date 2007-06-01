#ifndef SGAME_H
#define SGAME_H

#include "game.h"

#define SPIDER_COLUMNS 10
#define SPIDER_TOTAL_DECKS 2
#define SPIDER_TOTAL_SEEDS (SPIDER_TOTAL_DECKS * 4)

class SpiderGame : public Game
{
    Deck deck_;

    Row  rows_[SPIDER_COLUMNS];

    void PressButton(const Point &);
    void ReleaseButton(const Point &);
	void KeyRelease(char key);
    bool IsCompleted();
    void Restart();
    void Update();
public:
    SpiderGame(int id);
};

#endif
