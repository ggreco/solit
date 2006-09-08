#include "lowres_renderer.h"
#include <string>
#include <iostream>

void LowresRenderer::
DrawRect(const Point &pos, const Point &size, Uint32 color)
{
    SDL_Rect r = { pos.X(), pos.Y(), size.X(), size.Y() };

    AddDirtyRect(r);
    SDL_FillRect(screen_, &r, color);

    r.x++;
    r.y++;

    r.w -= 2;
    r.h -= 2;

    SDL_FillRect(screen_, &r, background_);
}

void LowresRenderer::
UpdateAll()
{
    SDL_UpdateRect(screen_, 0, 0, 0, 0);
    rects_.clear();
}

void LowresRenderer::
Update()
{
    if (!rects_.empty()) {
        for (std::vector<SDL_Rect>::iterator it = rects_.begin(); it != rects_.end(); ++it) {
            if ((it->w + it->x) > screen_->w)
                it->w = screen_->w - it->x;
            if ((it->h + it->y) > screen_->h)
                it->h = screen_->h - it->y;
        }

        SDL_UpdateRects(screen_, rects_.size(), &rects_[0]);
        rects_.clear();
    }
}

bool LowresRenderer::
Wait()
{
    for(;;) {
        SDL_Event e;

        if (SDL_WaitEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    return false;
                    break;
                case SDL_MOUSEMOTION:
                    Renderer::MouseMove(Point(e.motion.x, e.motion.y));
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    Renderer::PressButton(Point(e.button.x, e.button.y));
                    break;                    
                case SDL_MOUSEBUTTONUP:
                    Renderer::ReleaseButton(Point(e.button.x, e.button.y));
                    break;
            }
        }
    }
}

LowresRenderer::
LowresRenderer() :
    card_rend_(".", *this)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::string("Unable to initialize SDL.");

    atexit(SDL_Quit);

    screen_size_ = Point(320, 240);

    if (!(screen_ = SDL_SetVideoMode(screen_size_.X(), screen_size_.Y(), 16, SDL_SWSURFACE)))
        throw std::string("Unable to open display.");

    background_ =  SDL_MapRGB(screen_->format,
                        0, 200, 0);
    white_ = SDL_MapRGB(screen_->format,
                        255, 255, 255);
    black_ = SDL_MapRGB(screen_->format,
                        0, 0, 0);

    SDL_FillRect(screen_, NULL, background_);

    UpdateAll();

    SDL_Rect r = { 0, 0, 320, 240};
    SDL_SetClipRect(screen_, &r);

    deck_position_ = Point(1,1);
    card_size_ = Point(20, 30);

    cards_position_ = Point(deck_position_.X() + card_size_.X() + 2,
                            deck_position_.Y());

    card_rend_.Optimize();

    for (int i = 0; i < 8; i++) {
        seed_positions_[i] = Point(cards_position_.X() + 2 + 
                                   (i + 2) * (card_size_.X() + 2),
                                   deck_position_.Y());

        DrawRect(seed_positions_[i], card_size_, white_);
    }

    for (int i = 0; i < COLUMNS; i++) {
        column_positions_[i] = Point(1 + 
                                   i * (card_size_.X() + 2),
                                   deck_position_.Y() + 4 + card_size_.Y());
    }

}

LowresCardRenderer::
LowresCardRenderer(const std::string &path, LowresRenderer &rend) :
    rend_(rend)
{
    std::string file = path + "/low_seeds.bmp";

    source_ = SDL_LoadBMP(file.c_str());

    if (!source_)
        throw std::string("Unable to load cards GFX");

    file = path + "/low_back.bmp";

    back_ = SDL_LoadBMP(file.c_str());

    if (!back_)
        throw std::string("Unable to load card background GFX");
}

void LowresCardRenderer::
Optimize()
{
    SDL_Surface *temp = SDL_DisplayFormat(source_);

    if (temp) {
        SDL_FreeSurface(source_);
        source_ = temp;
    }

    temp = SDL_DisplayFormat(back_);

    if (temp) {
        SDL_FreeSurface(back_);
        back_ = temp;
    }
}

void LowresCardRenderer::
Refresh(const Card &card)
{ 
    std::map<Card,CardData>::iterator it;

    if ((it = card_datas_.find(card)) != card_datas_.end()) 
        Draw(card, it->second.Pos());
}

void LowresCardRenderer::
Clear(const Card &card)
{
    std::map<Card,CardData>::iterator it;

    if ((it = card_datas_.find(card)) != card_datas_.end()) {
        SDL_Rect r;

        r.x = it->second.Pos().X();
        r.y = it->second.Pos().Y();

        r.w = it->second.Dirty()->w;
        r.h = it->second.Dirty()->h;

        rend_.AddDirtyRect(r);

        SDL_BlitSurface(it->second.Dirty(), NULL, rend_.screen_, &r);

        card_datas_.erase(it);
    }
}

void LowresCardRenderer::
Draw(const Card &card, const Point &pos)
{
    std::map<Card,CardData>::iterator it;

    if ((it = card_datas_.find(card)) != card_datas_.end()) {
        SDL_Rect r;

        r.x = it->second.Pos().X();
        r.y = it->second.Pos().Y();

        r.w = it->second.Dirty()->w;
        r.h = it->second.Dirty()->h;

        rend_.AddDirtyRect(r);

        SDL_BlitSurface(it->second.Dirty(), NULL, rend_.screen_, &r);

        it->second.Pos(pos);
    }

    SDL_Rect r = { pos.X(), pos.Y(), 
                   rend_.card_size_.X(),
                   rend_.card_size_.Y() };

    if (it == card_datas_.end()) {
        card_datas_[card].Dirty(rend_.MakeDirty());

        it = card_datas_.find(card);
        it->second.Pos(pos);
    }

    SDL_BlitSurface(rend_.screen_, &r, it->second.Dirty(), NULL);

    rend_.AddDirtyRect(r);

    if (card.Covered()) {
        SDL_BlitSurface(back_, NULL, rend_.screen_, &r);
        return;
    }

    SDL_FillRect(rend_.screen_, &r, rend_.black_);

    r.x++;
    r.y++;

    r.w -= 2;
    r.h -= 2;

    SDL_FillRect(rend_.screen_, &r, rend_.white_);

    r.x++;
    r.y++;

    SDL_Rect s = { (card.Value() != 1) ? 8 * card.Value() :
                 14 * 8, card.IsBlack() ? 0 : 8, 8, 8 };

    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    r.x = pos.X() + rend_.card_size_.X() - 9;
    r.y = pos.Y() + rend_.card_size_.Y() - 9;

    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    if (card.IsRed())
        s.x = 15 * 8 + ((card.Seed() == Hearts) ? 0 : 8);
    else
        s.x = 15 * 8 + ((card.Seed() == Flowers) ? 0 : 8);

    r.x = pos.X() + rend_.card_size_.X() - 9;
    r.y = pos.Y() + 2;
    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    r.x = pos.X() + 2;
    r.y = pos.Y() + rend_.card_size_.Y() - 9;
    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

}

