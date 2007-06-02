#include "sgame.h"

SpiderGame::
SpiderGame(int res) :
    Game(res + 2, SPIDER_COLUMNS), deck_(2)
{
    Update();
}

void SpiderGame::
Restart()
{
	for (int j = 0; j < rend_->Columns(); j++)
		rows_[j].Clear();

    Game::Restart();
}

void SpiderGame::
Update()
{
    int cards = 0;

    while (cards < 54) {
        for (int i = 0; i < SPIDER_COLUMNS; i++) {
            Card card = deck_.GetCard();

            card.Covered(cards < 44);

            rows_[i].Add(card);
        	
	    cards++;
        }
    }

    for (int i = 0; i < SPIDER_COLUMNS; i++)
        rend_->Draw(rows_[i], i);

    rend_->Draw(deck_, Renderer::DeckPos);
}

bool SpiderGame::
IsCompleted()
{
    if (deck_.Size() == 0) {
        for (int i = 0; i < SPIDER_COLUMNS; ++i)
            if (rows_[i].Size())
                return false;

        return true;
    }

    return false;
}

void SpiderGame::
PressButton(const Point &)
{
}

void SpiderGame::
ReleaseButton(const Point &)
{
}

void SpiderGame::
KeyRelease(char key)
{
}

