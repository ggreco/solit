#include "sgame.h"
#include <stdlib.h>
#include <iostream>

SpiderGame::
SpiderGame(int res) :
    Game(SPIDER_ID, res, SPIDER_COLUMNS)
{
    Point pos(rend_->Spacing().X() / 2, 
            rend_->ScreenHeight() - rend_->CardSize().Y() - rend_->Spacing().Y());

    rend_->PositionDeck(pos);

    pos.Y(rend_->Spacing().Y());

#ifdef XIPHONE
    pos.AddY(rend_->ScreenWidth() == 640 || rend_->ScreenWidth() == 1536 ? 40 : 20);
#endif

    for (int i = 0; i < rend_->Columns(); i++) {
        rend_->PositionColumn(i, pos); 
        pos.AddX(rend_->CardSize().X() + rend_->Spacing().X());
    }
    
    
    pos.set(rend_->ScreenWidth() - rend_->WidgetWidth(QUIT_GAME) - rend_->Spacing().X(),
            rend_->ScreenHeight() - rend_->WidgetHeight(QUIT_GAME) - rend_->Spacing().Y());

    rend_->PositionWidget(QUIT_GAME, pos);

    pos.AddX(-rend_->WidgetWidth(NEW_GAME) - rend_->Spacing().X() * 2);
    rend_->PositionWidget(NEW_GAME, pos);

    pos.AddX(-rend_->WidgetWidth(UNDO_MOVE) - rend_->Spacing().X() * 2);
    rend_->PositionWidget(UNDO_MOVE, pos);

    Game::startup();
    Update();
}

void SpiderGame::
SetupCards()
{
    int cards = 0;

    // let's change the deck to have only two seeds
    for (CardIterator it = deck_.GetCards().begin(); 
                      it != deck_.GetCards().end(); ++it) {
        if (it->Seed() == Squares)
            it->Seed(Hearts);
        else if (it->Seed() == Pikes)
            it->Seed(Flowers);
    }

    while (cards < 54) {
        for (int i = 0; i < SPIDER_COLUMNS; i++) {
            Card card = deck_.GetCard();

            card.Covered(cards < 44);

            rows_[i].Add(card);
        	
			cards++;

			if (cards == 54)
				break;
        }
    }
}

void SpiderGame::
Restart()
{
    Save(); // to be able to load if the user push "undo"

	for (int j = 0; j < rend_->Columns(); j++)
		rows_[j].Clear();

    Game::Restart();
}

void SpiderGame::
Update()
{
    rend_->Clear();

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
PressButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    if (!selection_.Empty() || status_ == PLAYING_VICTORY)
        return;

    if (check_highlight(pos))
        Update();

    if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow) {

        if (!rows_[pos - Renderer::FirstRow].Empty() && !rows_[pos - Renderer::FirstRow].Get().Covered()) {
            std::pair<int, int> rowinfo = rend_->GetRowInfo();
            Row &r = rows_[pos - Renderer::FirstRow];
			int covered = r.HowManyCovered();
            int toget = (r.Size() - covered);

			toget -= (p.Y() - rowinfo.first - covered * rowinfo.second/4) / (rowinfo.second/3);

            if (toget <= 0)
                toget = 1;

            while (toget) {

                if (r.Get().Covered())
                    break;


				if ( !selection_.Empty() &&
					  (r.Get().Color() != selection_.Get().Color() ||
					  r.Get().Value() != (selection_.First().Value() + 1))
					)
					break;

                selection_.AddFirst(r.GetCard(), r, pos);
                toget--;
            }

            Update();
            draw_selection(p);
        }
    }

    rend_->Update();
}

bool SpiderRow::
Completed()
{
	Cards &c = GetCards();
	int val = 1;

	for (RevCardIterator it = c.rbegin(); it != c.rend();
		++it) {
		if (it->Value() != val)
			return false;

		if (it->Color() != Get().Color())
			return false;

		val++;

		if (val == 14)
			return true;
	}	

	return false;
}

void SpiderGame::
ReleaseButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    rend_->Unhighlight();

    if (status_ == PLAYING_VICTORY) {
        Game::ReleaseButton(p);
		return;
	}
    else if (!selection_.Empty()) {
        if (pos >= Renderer::FirstRow && 
                 pos <= Renderer::LastRow &&
                 rows_[pos - Renderer::FirstRow].CanGet(selection_.First())) {
			SpiderRow &r = rows_[pos - Renderer::FirstRow];
            moves_.Add(*selection_.Origin(), selection_.OriginPosition(),
                          r, pos, selection_);

            while (!selection_.Empty()) {
                r.Add(selection_.First());
                selection_.RemoveFirst();
            }

			if (r.Completed()) {
                Stackable temp;

				for (int i = 0; i < 13; i++) {
					used_.AddFirst(r.Get());
                    temp.AddFirst(r.Get());
					r.Remove();
				}
                moves_.Add(r, pos, used_, 
                        Renderer::Discarded, temp);
			}

            Update();
        }
        else {
            while (!selection_.Empty()) {
                selection_.Origin()->Add(selection_.First());
                selection_.RemoveFirst();
            }
            Update();
        }
	}
	else {

        if (pos == Renderer::DeckPos) {
            if (!deck_.Empty()) {
                int linked = rand(); // moves are only a single undo level

				for (int i = 0; i < SPIDER_COLUMNS; ++i) {
					Card c = deck_.GetCard();
				
					moves_.Add(deck_, Renderer::DeckPos,
                              rows_[i], Renderer::FirstRow + i, 
                              c, linked);

		            c.Covered(false);
					rows_[i].Add(c);

				}
            }
            Update();
        }
        else if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow) {
            int p = pos - Renderer::FirstRow;

            if (!rows_[p].Empty()) {
                if (rows_[p].Get().Covered()) {
                    moves_.Add(rows_[p], pos);
                    rows_[p].Get().Covered(false);
                    rend_->Draw(rows_[p], p);
                }
            }
        }
		else
			Game::ReleaseButton(p);
    }

    rend_->Update();

	if (IsCompleted())
		Victory();
}

void SpiderGame::
Victory()
{
    moves_.Clear(); // to avoid crashes if one uses undo
	rend_->Clear();
    status_ = PLAYING_VICTORY;	
	
	Game::Victory(used_, 2, 50);
}


