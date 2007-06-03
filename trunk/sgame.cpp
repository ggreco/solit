#include "sgame.h"

SpiderGame::
SpiderGame(int res) :
    Game(res + 2, SPIDER_COLUMNS)
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

			if (cards == 54)
				break;
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


				if ( !selection_.Empty() &&
					  (r.Get().Color() != selection_.Get().Color() ||
					  r.Get().Value() != (selection_.First().Value() + 1))
					)
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
	
    if (status_ == PLAYING_VICTORY) {
        Game::ReleaseButton(p);
		return;
	}
    else if (!selection_.Empty()) {
		rend_->Clear(selection_.Get());

        if (selection_.Size() > 1)
            rend_->Clear(selection_.First());

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
				for (int i = 0; i < 13; i++) {
					rend_->Clear(r.Get());
					used_.Add(r.Get());
					// XXX aggiungere gestione mossa
					r.Remove();
				}
			}

            rend_->Draw(r, pos - Renderer::FirstRow);
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
				for (int i = 0; i < SPIDER_COLUMNS; ++i) {
					Card c = deck_.GetCard();
				
					moves_.Add(deck_, Renderer::DeckPos,
                              rows_[i], Renderer::FirstRow + i, 
                              c);

		            c.Covered(false);
					rows_[i].Add(c);

                    rend_->Draw(rows_[i], i);

				}
	
				rend_->Draw(deck_, Renderer::DeckPos);
            }
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


