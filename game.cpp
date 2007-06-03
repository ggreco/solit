#include "game.h"
#include "lowres_renderer.h"
#include "deck.h"
#include <iostream>
#include "time.h"

Game::PosData Game::data_[] = 
{
    {0 , 0}, {40, 0}, {10, 15}, {30, 15}, {20, 40}, {20,60}, // Y
    {90, 0}, {110, 0}, {80, 20}, {120,20}, {80, 40}, {120, 40}, {90, 60}, {110, 60}, // O
    {160, 0}, {205, 0}, {160, 20}, {205,20}, {160, 40}, {205, 40}, {175, 60}, {190, 60}, // U
    {0 , 100}, {60, 100}, {5, 130}, {55, 130}, {10,160}, {50, 160},  {25, 145}, {40, 145}, // W
    {100 , 100}, {100, 130}, {100, 160}, // I
    {140 , 100}, {140, 130}, {140, 160},  {180 , 100}, {180, 130}, {180, 160}, {155, 120}, {165, 140}, // N
    {215, 100}, {215, 120}, {215, 160}, // !
    {-1, -1}
};

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
Victory(Stackable &used, int x_offset, int y_offset)
{
    bool flag = true;
	int idx;
    
    for (;;) {
        idx = 0;
        CardIterator it = used.GetCards().begin();

        while (data_[idx].x >= 0 ) {
            it->Covered(flag);
    	    rend_->Move(*it, Point(data_[idx].x + x_offset, 
								   data_[idx].y + y_offset));

            idx++;
            it++;

            int i = 0;

            while (i < 4) {
                rend_->Poll();

                if (status_ == PLAYING)
                    return;

                rend_->Delay(20);
                i++;
            }

    	    rend_->Update();
        }

        flag = !flag;
    }

}

void Game::
KeyRelease(char key)
{
	switch(key) {
        case 'w':
			{
				//deck_.Clear();
				rend_->Clear();
				status_ = PLAYING_VICTORY;
				
				Stackable fake;

				while (!deck_.Empty())
					fake.Add(deck_.GetCard());

				Victory(fake, 2, 40); // it's good both for 4:3 and 3:4
			}
            break;
		case 'q':
			exit(0);
			break;
		case 'n':
			Restart();
			break;
	}
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
					if (status_ == PLAYING)
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
