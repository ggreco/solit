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

HighScoreMap Game::scores_;
int Game::game_id_ = -1;

void Game::update_scores()
{
    if (game_id_ != -1) {
        std::cerr << "Updating highscores\n";

        if (FILE *hs = fopen(".scores", "w")) {
            for (HighScoreMap::iterator it = scores_.begin();
                    it != scores_.end(); ++it) {
                fprintf(hs, "%08d %08d %08d\n", it->first, it->second.first, 
                                                     it->second.second);
            }
            fclose(hs);
        }
    }
}

void Game::read_scores()
{
    if (FILE *hs = fopen(".scores", "r")) {
        int a, b, c;

        while(fscanf(hs, "%d %d %d\n", &a, &b, &c) == 3)
            scores_[a] = std::pair<int, int>(b, c);
       
        fclose(hs);
    }
    else { // init scores to 0
        for (int i = 0; i < 4; i++)
            scores_[i] = std::pair<int, int>(0, 0);
    }

    atexit(update_scores);
}

Game::Game(int id, int cols, int seeds, bool card_slot ) :
    deck_(2), status_(PLAYING)
{
    game_id_ = id;

    read_scores();

    rend_ = new SdlRenderer(id, cols, seeds, card_slot);

    srand(time(NULL));
    rend_->SetActionManager(this);

    deck_.Deal();
}

void Game::
Victory(Stackable &used, int x_offset, int y_offset)
{
    bool flag = true;
	int idx;
   
    std::cerr << "Entro in victory, " << used.Size() << " carte\n";

    for (;;) {
        idx = 0;
        CardIterator it = used.GetCards().begin();

        while (data_[idx].x >= 0 && it != used.GetCards().end()) {
            it->Covered(flag);
            rend_->Move(*it, Point(data_[idx].x + x_offset, 
                        data_[idx].y + y_offset));

            idx++;
            it++;

            for (int i = 0; i < 4; ++i) {
                rend_->Poll();

                if (status_ == PLAYING)
                    return;

                rend_->Delay(20);
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
				rend_->Clear();
				status_ = PLAYING_VICTORY;
				
				Stackable fake;

				while (!deck_.Empty()) { // we need a lot of cards
                    Card c = deck_.GetCard();
					fake.Add(c);
					fake.Add(c);
                }

				Victory(fake, 2, 40); // it's good both for 4:3 and 3:4
			}
            break;
		case 'q':
            check_scores();
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
        Update();

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
                    check_scores();
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
    moves_.Revert();
    Update();
    rend_->Update();
}

void Game::
check_scores()
{
    if (status_ == PLAYING_VICTORY)
        scores_[game_id_ / 2].first++;
    else if (moves_.Size())
        scores_[game_id_ / 2].second++;

    update_scores();
}

void Game::
Restart()
{
    check_scores();

    moves_.Clear();

	rend_->Clear();

	deck_.init();
    deck_.Deal();

    SetupCards();

    Update();

    status_ = PLAYING;
    rend_->Update();
}

void MoveList::
Revert()
{
    if (moves_.empty())
        return;

    Move &last = moves_.top();

    int linked = last.linked;

    if (!last.dest) {
        last.source->Get().Covered(true);
    }
    else {
        while (last.cards.Size()) {
            last.source->Add(last.cards.First());
            last.dest->Remove();
            last.cards.RemoveFirst();
        }
    }

    moves_.pop();

    if (linked != -1 && !moves_.empty() &&
        moves_.top().linked == linked) {
        Revert();
    }
}

void Game::
Exposed()
{
    Update();
    Rend()->Update();
}
