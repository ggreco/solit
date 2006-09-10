#include "renderer.h"
#include <iostream>

void Renderer::
Draw(const Stackable &obj, int pos)
{
    CardRenderer *cr = GetCardRenderer();

    if (obj.Empty())
        return;

    if (pos >= FirstSeedPos && pos <= LastSeedPos) {
        cr->Draw(obj.Get(), seed_positions_[pos - FirstSeedPos]);
    }
    else if (pos >= FirstRow && pos <= LastRow) {
        Draw(reinterpret_cast<const Row &>(obj), pos - FirstRow);
    }
    else switch(pos) {
        case DeckPos:
            if (obj.Empty())
                DrawEmpty(deck_position_);
            else
                cr->Draw(obj.Get(), deck_position_);
            break;
        case CardPos:
            if (obj.Empty())
                DrawEmpty(cards_position_);
            else
                cr->Draw(obj.Get(), cards_position_);
            break;
        default:
            std::cerr << "non so come disegnare in questa posizione:" << pos << "\n";
            break;
    }
}

int Renderer::
GetPosition(const Point &p)
{
    if (inside(p, deck_position_, card_size_))
        return DeckPos;
    else if (inside(p, cards_position_, card_size_))
        return CardPos;
    else { 
		int i;

        for (i = 0; i < 8; ++i)
            if (inside(p, seed_positions_[i], card_size_))
                return FirstSeedPos + i;

        for (i = 0; i < COLUMNS; ++i) {
            if (inside(p, column_positions_[i], Point(card_size_.X(), screen_size_.Y())))
                return FirstRow + i;
        }
    }

    return -1;
}

void Renderer::
Draw(const Row &r, int pos)
{
    CardRenderer *cr = GetCardRenderer();
    Point p = column_positions_[pos];

    for (ConstCardIterator it = r.GetCards().begin(); 
                      it != r.GetCards().end(); ++it) {
        
        cr->Draw(*it, p);

        if (it->Covered())
           p.Y( p.Y() + card_size_.Y() / 4);
        else
           p.Y( p.Y() + card_size_.Y() / 3);
    }
}

