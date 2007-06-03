#include "kgame.h"
#include "renderer.h"
#include <iostream>
#include <time.h>


KlondikeGame::
KlondikeGame(int res) :
    Game(res, KLONDIKE_COLUMNS, KLONDIKE_TOTAL_SEEDS, true)
{
	Update();
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

typedef struct
{
    int x;
    int y;
} posdata;

posdata data[] = 
{
    {0 , 0}, {40, 0}, {10, 15}, {30, 15}, {20, 40}, {20,60}, // Y
    {90, 0}, {110, 0}, {80, 20}, {120,20}, {80, 40}, {120, 40}, {90, 60}, {110, 60}, // O
    {160, 0}, {205, 0}, {160, 20}, {205,20}, {160, 40}, {205, 40}, {175, 60}, {190, 60}, // U
    {0 , 100}, {60, 100}, {5, 130}, {55, 130}, {10,160}, {50, 160},  {25, 145}, {40, 145}, // W
    {100 , 100}, {100, 130}, {100, 160}, // I
    {140 , 100}, {140, 130}, {140, 160},  {180 , 100}, {180, 130}, {180, 160}, {155, 120}, {165, 140}, // N
    {220, 100}, {220, 120}, {220, 160}, // !
    {-1, -1}
};

void KlondikeGame::Victory()
{
    moves_.Clear(); // to avoid crashes if one uses undo
	rend_->Clear();
    Stackable used;

    status_ = PLAYING_VICTORY;

    int deck = 0, idx = 0;

    int x_offset = 20, y_offset = 40;

    while (data[idx].x >= 0 ) {
            deck++;

            if (deck == KLONDIKE_TOTAL_SEEDS)
                deck = 0;

    	    rend_->Move(seeds_[deck].Get(), Point(data[idx].x + x_offset, data[idx].y + y_offset));

            used.Add(seeds_[deck].Get());

            seeds_[deck].Remove();

            rend_->Poll();

            if (status_ == PLAYING)
                return;

            rend_->Delay(5);
    	    rend_->Update();
            idx++;
    }

    bool flag = true;

    for (;;) {
        idx = 0;
        CardIterator it = used.GetCards().begin();

        while (data[idx].x >= 0 ) {
            it->Covered(flag);
    	    rend_->Move(*it, Point(data[idx].x + x_offset, data[idx].y + y_offset));

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
KeyRelease(char key)
{
	switch(key) {
		case 'a':
            if (IsCompleted())
				AutoComplete();

			break;
        case 'w':
			{
            deck_.Clear();

          	for (int j = 0; j < 2; j++) {
               for (int i = 1; i < 14; i++) {
                    Card c1(i, Hearts, j);
                    Card c2(i, Squares, j);
                    Card c3(i, Pikes, j);
                    Card c4(i, Flowers, j);

                    c1.Covered(false);
                    c2.Covered(false);
                    c3.Covered(false);
                    c4.Covered(false);

                    seeds_[j * 4].Add(c1); 
                    seeds_[1 + j * 4].Add(c2); 
                    seeds_[2 + j * 4].Add(c3); 
                    seeds_[3 + j * 4].Add(c4);

                    for (int k = 0; k < KLONDIKE_TOTAL_SEEDS; ++k)
                        rend_->Draw(seeds_[k], Renderer::FirstSeedPos + k);
                }
            }

            Victory();
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

                rend_->Clear(r.Get());
                selection_.AddFirst(r.GetCard(), r, pos);
                toget--;
            }

            rend_->Draw(selection_.First(), p);

            if (selection_.Size() > 1)
                rend_->Draw(selection_.Get(), Point(p.X(), p.Y() + 15));
        }
    }
    else if (pos >= Renderer::FirstSeedPos && pos <= Renderer::LastSeedPos) {
        if (!seeds_[pos - Renderer::FirstSeedPos].Empty()) {
            selection_.Add(seeds_[pos - Renderer::FirstSeedPos].Get(), seeds_[pos - Renderer::FirstSeedPos], pos);
            seeds_[pos - Renderer::FirstSeedPos].Remove();
            rend_->Draw(selection_.Get(), p);
        }
    }
    else if (pos == Renderer::CardPos) {
        if (!cards_.Empty()) {
            selection_.Add(cards_.Get(), cards_, pos);
            cards_.Remove();
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
        pos -= Renderer::FirstWidget;

        switch (pos) {
            case QUIT_GAME:
                exit(0);
                break;
            case NEW_GAME:
                Restart();
                break;
        }
    }
    else if (!selection_.Empty()) {

        rend_->Clear(selection_.Get());

        if (selection_.Size() > 1)
            rend_->Clear(selection_.First());

        if (pos >= Renderer::FirstSeedPos &&
            pos <= Renderer::LastSeedPos &&
            selection_.Size() == 1 &&
            seeds_[pos - Renderer::FirstSeedPos].CanGet(selection_.Get())) {

            moves_.Add(*selection_.Origin(), selection_.OriginPosition(),
                          seeds_[pos - Renderer::FirstSeedPos], pos,
                          selection_);

            seeds_[pos - Renderer::FirstSeedPos].Add(selection_.Get());
            selection_.Remove();
            rend_->Draw(seeds_[pos - Renderer::FirstSeedPos], pos);

		
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
            rend_->Draw(rows_[pos - Renderer::FirstRow], pos - Renderer::FirstRow);
        }
        else {
            while (!selection_.Empty()) {
                selection_.Origin()->Add(selection_.First());
                selection_.RemoveFirst();
            }
            rend_->Draw(*selection_.Origin(), selection_.OriginPosition());
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
            rend_->Draw(cards_, Renderer::CardPos);
            rend_->Draw(deck_, Renderer::DeckPos);
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

            rend_->Draw(seeds_[i], Renderer::FirstSeedPos + i);

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
    for (int i = 0; i < KLONDIKE_COLUMNS; i++) {
        for (int j = 0; j <= i; j++) {
            Card card = deck_.GetCard();
            
            if (j == i)
                card.Covered(false);

            rows_[i].Add(card);
        }

        rend_->Draw(rows_[i], i);
    }

    rend_->Draw(deck_, Renderer::DeckPos);
    rend_->Draw(cards_, Renderer::CardPos);

    for (int k = 0; k < KLONDIKE_TOTAL_SEEDS; ++k)
        rend_->Draw(seeds_[k], Renderer::FirstSeedPos + k);
}

void MoveList::
Revert(Renderer *rend)
{
    if (moves_.empty())
        return;

    Move &last = moves_.top();

    if (!last.dest) {
        last.source->Get().Covered(true);
        rend->Draw(*last.source, last.source_pos);

    }
    else {
        for (RevCardIterator it = last.cards.GetCards().rbegin(); 
                      it != last.cards.GetCards().rend(); ++it)
           rend->Clear(*it);
        
        while (last.cards.Size()) {
            last.source->Add(last.cards.First());
            last.dest->Remove();
            last.cards.RemoveFirst();
        }

        rend->Draw(*last.source, last.source_pos);
        rend->Draw(*last.dest, last.dest_pos);
    }

    moves_.pop();
}

