#include "game.h"
#include "lowres_renderer.h"
#include "deck.h"
#include <iostream>

Game::Game(int id, int cols, int seeds, bool card_slot ) :
    deck_(2), status_(PLAYING)
{
    rend_ = new SdlRenderer(id, cols, seeds, card_slot);

    srand(time(NULL));
    rend_->SetActionManager(this);

    deck_.Deal();
}

// STUB, do better :)
void Game::
Victory()
{
    std::cerr << "U win!!!!\n";
}

void Game::
MouseMove(const Point &p)
{
    if (!selection_.Empty()) {
        if (selection_.Size() > 1)
            rend_->Clear(selection_.Get());

        rend_->Draw(selection_.First(), p );

        if (selection_.Size() > 1)
            rend_->Draw(selection_.Get(), Point(p.X(), p.Y() + 15));

        rend_->Update();
    }
}

void Game::
UndoMove()
{
    moves_.Revert(rend_);
}

void Game::
Restart()
{
    moves_.Clear();

	rend_->Clear();

	deck_.init();
    deck_.Deal();

	Update();
	rend_->UpdateAll();

    status_ = PLAYING;
}
