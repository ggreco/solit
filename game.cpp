#include "game.h"
#include "lowres_renderer.h"
#include "deck.h"
#include <iostream>
#include "time.h"

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
ReleaseButton(const Point &p)
{
	int pos = rend_->GetPosition(p);

	if (pos >= Renderer::FirstWidget && pos <= Renderer::LastWidget) {
            pos -= Renderer::FirstWidget;

            switch (pos) {
                case QUIT_GAME:
                    exit(0);
                    break;
                case UNDO_MOVE:
                    UndoMove();
                    break;
                case NEW_GAME:
                    Restart();
                    break;
            }
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
