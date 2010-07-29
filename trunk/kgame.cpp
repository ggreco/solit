#include "kgame.h"
#include "renderer.h"
#include <iostream>
#include <time.h>


KlondikeGame::
KlondikeGame(int res) :
    Game(res, KLONDIKE_COLUMNS, KLONDIKE_TOTAL_SEEDS, true)
{
    // klondike is game 0
    game_id_ = KLONDIKE_ID;

    rend_->PositionWidget(QUIT_GAME, 
            Point(rend_->ScreenWidth() - 
                rend_->WidgetWidth(QUIT_GAME), 0)
            );

    rend_->PositionWidget(NEW_GAME, 
            Point(rend_->ScreenWidth() - rend_->WidgetWidth(NEW_GAME), 
                rend_->ScreenHeight() - rend_->WidgetHeight(NEW_GAME))
            );

    rend_->PositionWidget(UNDO_MOVE, 
            Point(rend_->ScreenWidth() - rend_->WidgetWidth(UNDO_MOVE), 
                (rend_->ScreenHeight() - rend_->WidgetHeight(UNDO_MOVE)) / 2)
            );

    Point pos(rend_->Spacing().X(), rend_->Spacing().Y() / 2);
    Point displacement(rend_->CardSize().X() + rend_->Spacing().X(), 0);
    rend_->PositionDeck(pos);

    pos += displacement;
    rend_->PositionCards(pos);

    pos.AddX(rend_->Spacing().X());

    pos += displacement;
    pos += displacement;

    for (int i = 0; i < rend_->Seeds(); i++) {
        rend_->PositionSeed(i, pos);
        pos += displacement;
    }

    pos.set(rend_->Spacing().X() , rend_->Spacing().Y() * 5 / 2 + rend_->CardSize().Y());

    for (int i = 0; i < rend_->Columns(); i++) {
        rend_->PositionColumn(i, pos);
        pos += displacement;
    }

    Game::startup();
}

void KlondikeGame::
SetupCards()
{
    for (int i = 0; i < KLONDIKE_COLUMNS; i++) {
        for (int j = 0; j <= i; j++) {
            Card card = deck_.GetCard();
            
            if (j == i)
                card.Covered(false);

            rows_[i].Add(card);
        }
    }
}

void KlondikeGame::
Restart()
{
	cards_.Clear();

	for (int i = 0; i < rend_->Seeds(); i++)
		seeds_[i].Clear();

	for (int j = 0; j < rend_->Columns(); j++)
		rows_[j].Clear();

    Game::Restart();
}

bool KlondikeGame::
SeedsFull()
{
	int total = 0;

	for (int i = 0; i < KLONDIKE_TOTAL_SEEDS; ++i)
		total += seeds_[i].Size();

	return (total == (13 * KLONDIKE_TOTAL_SEEDS));
}

void KlondikeGame::Victory()
{
    moves_.Clear(); // to avoid crashes if one uses undo
	rend_->Clear();
    Stackable used;

    status_ = PLAYING_VICTORY;

    int deck = 0, idx = 0;

    int x_offset = 20, y_offset = 40;

    while (data_[idx].x >= 0 ) {
            deck++;

            if (deck == KLONDIKE_TOTAL_SEEDS)
                deck = 0;

    	    rend_->Move(seeds_[deck].Get(), Point(data_[idx].x + x_offset, data_[idx].y + y_offset));

            used.Add(seeds_[deck].Get());

            seeds_[deck].Remove();

            rend_->Poll();

            if (status_ == PLAYING)
                return;

            rend_->Delay(5);
    	    rend_->Update();
            idx++;
    }

	Game::Victory(used, x_offset, y_offset);
}

void KlondikeGame::
AutoComplete()
{
    status_ = AUTOCOMPLETE;

	while (!SeedsFull()) {
		for (int i = 0; i < KLONDIKE_COLUMNS; ++i) {
			if (!rows_[i].Size())
				continue;

			for (int j = 0; j < KLONDIKE_TOTAL_SEEDS; ++j) {
    			if (!rows_[i].Size())
	    			continue;

				if (seeds_[j].CanGet(rows_[i].Get())) {

					rend_->Move(rows_[i].Get(), Renderer::FirstSeedPos + j);

					seeds_[j].Add(rows_[i].GetCard());

					rend_->Draw(seeds_[j], Renderer::FirstSeedPos + j);

                    rend_->Poll();

					rend_->Update();
				}
			}
		}
	}

	Victory();
}

bool KlondikeGame::
IsCompleted()
{
    if (deck_.Size() == 0 &&
       cards_.Size() == 0) {
        for (int i = 0; i < KLONDIKE_COLUMNS; ++i)
            if (rows_[i].Size() &&
                    rows_[i].First().Covered())
                return false;

        return true;
    }

    return false;
}

void KlondikeGame::
PressButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    if (!selection_.Empty() || status_ == PLAYING_VICTORY)
        return;

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

                selection_.AddFirst(r.GetCard(), r, pos);
                toget--;
            }

            Update();

            rend_->Draw(selection_.First(), p);

            if (selection_.Size() > 1)
                rend_->Draw(selection_.Get(), Point(p.X(), p.Y() + 15));
        }
    }
    else if (pos >= Renderer::FirstSeedPos && pos <= Renderer::LastSeedPos) {
        if (!seeds_[pos - Renderer::FirstSeedPos].Empty()) {
            selection_.Add(seeds_[pos - Renderer::FirstSeedPos].Get(), seeds_[pos - Renderer::FirstSeedPos], pos);
            seeds_[pos - Renderer::FirstSeedPos].Remove();
            Update();
            rend_->Draw(selection_.Get(), p);
        }
    }
    else if (pos == Renderer::CardPos) {
        if (!cards_.Empty()) {
            selection_.Add(cards_.Get(), cards_, pos);
            cards_.Remove();
            Update();
            rend_->Draw(selection_.Get(), p);
        }
    }

    rend_->Update();
}

void KlondikeGame::
ReleaseButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    if (status_ == PLAYING_VICTORY || status_ == AUTOCOMPLETE) {
        Game::ReleaseButton(p);
		return;
    }
    else if (!selection_.Empty()) {
        if (pos >= Renderer::FirstSeedPos &&
            pos <= Renderer::LastSeedPos &&
            selection_.Size() == 1 &&
            seeds_[pos - Renderer::FirstSeedPos].CanGet(selection_.Get())) {

            moves_.Add(*selection_.Origin(), selection_.OriginPosition(),
                          seeds_[pos - Renderer::FirstSeedPos], pos,
                          selection_);

            seeds_[pos - Renderer::FirstSeedPos].Add(selection_.Get());
            selection_.Remove();
            Update();
//            rend_->Draw(seeds_[pos - Renderer::FirstSeedPos], pos);

		
			if (SeedsFull()) {
				// handle victory condition!
				Victory();
			}
        }
        else if (pos >= Renderer::FirstRow && 
                 pos <= Renderer::LastRow &&
                 rows_[pos - Renderer::FirstRow].CanGet(selection_.First())) {

            moves_.Add(*selection_.Origin(), selection_.OriginPosition(),
                          rows_[pos - Renderer::FirstRow], pos,
                          selection_);

            while (!selection_.Empty()) {
                rows_[pos - Renderer::FirstRow].Add(selection_.First());
                selection_.RemoveFirst();
            }
            Update();
//            rend_->Draw(rows_[pos - Renderer::FirstRow], pos - Renderer::FirstRow);
        }
        else {
            while (!selection_.Empty()) {
                selection_.Origin()->Add(selection_.First());
                selection_.RemoveFirst();
            }
//            rend_->Draw(*selection_.Origin(), selection_.OriginPosition());
            Update();
        }
    }
    else {

        if (pos == Renderer::DeckPos) {
            if (!deck_.Empty()) {
                Card c = deck_.GetCard();

                moves_.Add(deck_, Renderer::DeckPos,
                              cards_, Renderer::CardPos, 
                              c);

                c.Covered(false);
                cards_.Add(c);
            }
            else {
                moves_.Add(cards_, Renderer::CardPos,
                              deck_, Renderer::DeckPos, 
                              cards_);

                while (!cards_.Empty()) {
                    deck_.Add(cards_.Get());
                    cards_.Remove();
                    deck_.Get().Covered(true);
                }
            }
           // rend_->Draw(cards_, Renderer::CardPos);
           // rend_->Draw(deck_, Renderer::DeckPos);
            Update();
        }
        else if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow) {
            int p = pos - Renderer::FirstRow;
            if (!rows_[p].Empty()) {
                if (rows_[p].Get().Covered()) {
                    moves_.Add(rows_[p], pos);
                    rows_[p].Get().Covered(false);
//                    rend_->Draw(rows_[p], p);
                }
            }
            Update();
        }
		else
			Game::ReleaseButton(p);
    }

    rend_->Update();

    if (IsCompleted())
        AutoComplete();
}

void KlondikeGame::
DoubleClick(const Point &p)
{
    Stackable *o = NULL;

    int pos = rend_->GetPosition(p);

    if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow)
        o = &(rows_[pos - Renderer::FirstRow]);
    else if (pos == Renderer::CardPos) 
        o = &cards_;

    if (!o || o->Empty() || status_ == PLAYING_VICTORY)
        return;
    
    for (int i = 0; i < KLONDIKE_TOTAL_SEEDS; i++) {
        if (seeds_[i].CanGet(o->Get())) {
            moves_.Add(*o, pos,
                       seeds_[i], Renderer::FirstSeedPos + i, 
                       o->Get());

            seeds_[i].Add(o->Get());
            o->Remove();

            Update();

//            rend_->Draw(seeds_[i], Renderer::FirstSeedPos + i);

            rend_->Update();

            if (SeedsFull())
                Victory();
            else if (IsCompleted())
                AutoComplete();

            return;
        }
    }
}


void KlondikeGame::Update()
{
    rend_->Clear();

    for (int i = 0; i < KLONDIKE_COLUMNS; i++) 
        rend_->Draw(rows_[i], i);

    rend_->Draw(deck_, Renderer::DeckPos);
    rend_->Draw(cards_, Renderer::CardPos);

    for (int k = 0; k < KLONDIKE_TOTAL_SEEDS; ++k)
        rend_->Draw(seeds_[k], Renderer::FirstSeedPos + k);
}


