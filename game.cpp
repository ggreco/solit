#include "game.h"
#include "lowres_renderer.h"
#include "deck.h"
#include <iostream>
#include "time.h"
#ifndef WIN32
#include <sys/stat.h>
#endif

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

        if (FILE *hs = fopen(ScoreName().c_str(), "w")) {
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
    if (FILE *hs = ::fopen(ScoreName().c_str(), "r")) {
        int a, b, c;

        while(::fscanf(hs, "%d %d %d\n", &a, &b, &c) == 3)
            scores_[a] = std::pair<int, int>(b, c);
       
        ::fclose(hs);
    }
}

Game::Game(int id, int res, int cols, int seeds, bool card_slot ) :
    status_(PLAYING), deck_(2), restarted_(false)
{
    game_id_ = id;

    for (int i = 0; i < NUMBER_OF_GAMES; i++)
        scores_[i] = std::pair<int, int>(0, 0);

    rend_ = new SdlRenderer(id, res, cols, seeds, card_slot);

    ::srand((int)::time(NULL));
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
            OnQuit();
			break;
		case 'n':
			Restart();
			break;
	}
}

bool Game::
check_highlight(int pos)
{
    bool need_update;

    if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow) {
        pos -= Renderer::FirstRow;

        need_update = (pos != rend_->Highlighted());

        rend_->Highlight(pos);
    }
    else {
        need_update = (rend_->Highlighted() != -1);
        rend_->Unhighlight();
    }

    return need_update;
}

void Game::
draw_selection(const Point &p) {
    if (!selection_.Empty()) {
        rend_->Draw(selection_.First(), p );

        if (selection_.Size() > 1)
            rend_->Draw(selection_.Get(), Point(p.X(), p.Y() + rend_->CardSize().Y()/3));
    }
}

void Game::
MouseMove(const Point &p)
{
    if (status_ == PLAYING_VICTORY)
        return;

	int pos = rend_->GetPosition(p);

    bool need_draw = (check_highlight(pos) || !selection_.Empty());

    if (need_draw) {
        Update();
        draw_selection(p);
        rend_->Update();
    }
}

void Game::
ReleaseButton(const Point &p)
{
	int pos = rend_->GetPosition(p);

    rend_->Unhighlight();

	if (pos >= Renderer::FirstWidget && pos <= Renderer::LastWidget) {
            pos -= Renderer::FirstWidget;

            switch (pos) {
                case QUIT_GAME:
                    OnQuit();
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
OnQuit() {
    check_scores();
    if (status_ != PLAYING_VICTORY)
        Save();
    else
        ::remove(GameName().c_str());

    exit(0);
}

void Game::
startup()
{
    read_scores();

    if (FILE *f = ::fopen(GameName().c_str(), "rb")) {
        std::cerr << "Loading game in progress...";
        SerialLoader s(f);

        s & *this;
        std::cerr << "OK.\n";
        ::fclose(f);
    }
    else
        SetupCards();

    Update();
    rend_->Update();
}

std::string Game::
ScoreName() {
    std::string filename = DirName();

    filename += ".scores";

    return filename;
}

std::string Game::
DirName() {
#ifdef XIPHONE
    std::string path = SDL_GetBasePath();
    return path + "/../Documents/";
#elif defined(WIN32)
    return "./";
#else
    if (const char *d = SDL_getenv("HOME")) {
        std::string s = d;

        s.append("/.solit");
        ::mkdir(s.c_str(), 0755);

        s.push_back('/');
        return s;
    }
    else
        return "./";
#endif
}

std::string Game::
GameName() {
    static const char *gamename[NUMBER_OF_GAMES] = { "klondike", "spider" };
    std::string filename = DirName();

    if (game_id_ >= 0 && game_id_ < NUMBER_OF_GAMES)
        filename.append(gamename[game_id_]);
    else
        std::cerr << "Invalid game id!\n";

    filename.append(SAVEGAME_NAME);

    return filename;
}

void Game::
Save() {
    if (FILE *f = ::fopen(GameName().c_str(), "wb")) {
        std::cerr << "Writing status to disk...";
        SerialSaver s(f);
        s & *this;
        std::cerr << "Done.\n";
        ::fclose(f);
    }
    else
        std::cerr << "Unable to open " << GameName() << " for writing!\n";
}

void Game::
UndoMove()
{
    if (restarted_ && moves_.Empty())
        startup();
    else
        moves_.Revert();

    restarted_ = false;

    Update();
    rend_->Update();
}

void Game::
check_scores()
{
    if (status_ == PLAYING_VICTORY)
        scores_[game_id_ ].first++;
    else if (moves_.Size())
        scores_[game_id_ ].second++;

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

    restarted_ = true;
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
