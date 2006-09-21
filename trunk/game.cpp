#include "game.h"
#include "renderer.h"
#include <iostream>
#include <time.h>

Game::Game(Renderer *r) :
    rend_(r)
{
    srand(time(NULL));
    r->SetActionManager(this);

    deck_.Deal();

	Update();

}

void Game::
Restart()
{
	cards_.Clear();
	for (int i = 0; i < TOTAL_SEEDS; i++)
		seeds_[i].Clear();

	for (int j = 0; j < COLUMNS; j++)
		rows_[j].Clear();

	rend_->Clear();

	deck_.init();
    deck_.Deal();

	Update();
	rend_->UpdateAll();
}

void Game::
MouseMove(const Point &p)
{
//    std::cerr << "Move to " << p.X() << "," << p.Y() << "\n";
    if (!selection_.Empty()) {
        if (selection_.Size() > 1)
            rend_->Clear(selection_.Get());

        rend_->Draw(selection_.First(), p );

        if (selection_.Size() > 1)
            rend_->Draw(selection_.Get(), Point(p.X(), p.Y() + 15));

        rend_->Update();
    }
}

bool Game::
SeedsFull()
{
	int total = 0;

	for (int i = 0; i < TOTAL_SEEDS; ++i)
		total += seeds_[i].Size();

	return (total == (13 * TOTAL_SEEDS));
}

void Game::Victory()
{
	rend_->Clear();

	//  XXX implementare animazione vittoria.
}

void Game::AutoComplete()
{
	while (!SeedsFull()) {
		for (int i = 0; i < COLUMNS; ++i) {
			if (!rows_[i].Size())
				continue;

			for (int j = 0; j < TOTAL_SEEDS; ++j) {
    			if (!rows_[i].Size())
	    			continue;

				if (seeds_[j].CanGet(rows_[i].Get())) {

                    std::cerr << "Moving " << rows_[i].Get().str() << " on "
                              << seeds_[j].Get().str() << std::endl;

					rend_->Move(rows_[i].Get(), Renderer::FirstSeedPos + j);

					seeds_[j].Add(rows_[i].GetCard());

					rend_->Draw(seeds_[j], Renderer::FirstSeedPos + j);

					rend_->Update();
				}
			}
		}
	}

	Victory();
}

void Game::
KeyRelease(char key)
{
	switch(key) {
		case 'a':
			if (deck_.Size() == 0 &&
				cards_.Size() == 0) {
				for (int i = 0; i < COLUMNS; ++i)
					if (rows_[i].Size() &&
						rows_[i].First().Covered())
						return;	
					
				AutoComplete();
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
PressButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    if (!selection_.Empty())
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

void Game::
ReleaseButton(const Point &p)
{
    int pos = rend_->GetPosition(p);

    if (!selection_.Empty()) {

        rend_->Clear(selection_.Get());

        if (selection_.Size() > 1)
            rend_->Clear(selection_.First());

        if (pos >= Renderer::FirstSeedPos &&
            pos <= Renderer::LastSeedPos &&
            selection_.Size() == 1 &&
            seeds_[pos - Renderer::FirstSeedPos].CanGet(selection_.Get())) {

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

                c.Covered(false);
                cards_.Add(c);
            }
            else {
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
                    rows_[p].Get().Covered(false);
                    rend_->Draw(rows_[p], p);
                }
            }
        }
    }

    rend_->Update();
}

void Game::
DoubleClick(const Point &p)
{
    Stackable *o = NULL;

    int pos = rend_->GetPosition(p);

    if (pos >= Renderer::FirstRow && pos <= Renderer::LastRow)
        o = &(rows_[pos - Renderer::FirstRow]);
    else if (pos == Renderer::CardPos) 
        o = &cards_;

    if (!o || o->Empty())
        return;
    
    for (int i = 0; i < TOTAL_SEEDS; i++) {
        if (seeds_[i].CanGet(o->Get())) {
            seeds_[i].Add(o->Get());
            o->Remove();

            rend_->Draw(seeds_[i], Renderer::FirstSeedPos + i);

            rend_->Update();

            return;
        }
    }
}


void Game::Update()
{
    for (int i = 0; i < COLUMNS; i++) {
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

    for (int k = 0; k < TOTAL_SEEDS; ++k)
        rend_->Draw(seeds_[k], Renderer::FirstSeedPos + k);
}
