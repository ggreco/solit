#include "lowres_renderer.h"
#include <string>
#include <iostream>
#include <math.h>

ResInfo res[] = {
    {320, 240, 20, 30, 2, 2, 8, 8, 1.0, "low"},
    {640, 480, 40, 60, 4, 4, 13, 16, 2.0, "med"}
};

void SdlRenderer::
Clear()
{
    int i;

	card_rend_.ClearBuffers();

    SDL_FillRect(screen_, NULL, background_);

	for (i = 0; i < TOTAL_SEEDS; i++)
        DrawRect(seed_positions_[i], card_size_, white_);

    for (i = 0; i < WIDGET_NUM; i++) {
        if (widgets_[i]) {
            SDL_Rect dst;

            dst.x = widget_positions_[i].X();
            dst.y = widget_positions_[i].Y();

            SDL_BlitSurface(widgets_[i], NULL, screen_, &dst);
        }
    }

    UpdateAll();
}

void SdlRenderer::
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

void SdlRenderer::
UpdateAll()
{
    SDL_UpdateRect(screen_, 0, 0, 0, 0);
    rects_.clear();
}

void SdlRenderer::
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

void SdlRenderer::
Poll()
{
    SDL_Event e;

    if (SDL_PollEvent(&e)) 
        ParseEvent(e);
}

void SdlRenderer::
ParseEvent(const SDL_Event &e)
{
    switch (e.type) {
        case SDL_QUIT:
            exit(0);
            break;
        case SDL_KEYDOWN:
            Renderer::KeyPress(e.key.keysym.sym);
            break;
        case SDL_KEYUP:
            Renderer::KeyRelease(e.key.keysym.sym);
            break;
        case SDL_MOUSEMOTION:
            Renderer::MouseMove(Point(e.motion.x, e.motion.y));
            break;
        case SDL_MOUSEBUTTONDOWN:
            Renderer::PressButton(Point(e.button.x, e.button.y));
            break;                    
        case SDL_MOUSEBUTTONUP:
            Renderer::ReleaseButton(Point(e.button.x, e.button.y));

            if ((SDL_GetTicks() - lastclick_) < 300) {
                Renderer::DoubleClick(Point(e.button.x, e.button.y));
                lastclick_ = 0;
            }
            else
                lastclick_ = SDL_GetTicks();

            break;
    }
}

bool SdlRenderer::
Wait()
{
    for(;;) {
        SDL_Event e;

        if (SDL_WaitEvent(&e))
            ParseEvent(e);
    }
}

#ifdef WINCE
#define OPENFLAGS (SDL_SWSURFACE|SDL_FULLSCREEN)
#else
#define OPENFLAGS SDL_SWSURFACE
#endif

SDL_Surface *SdlRenderer::
load_image(const std::string &base)
{
	extern SDL_RWops *load_or_grab_image(const std::string &);
	std::string path = std::string("./") + res_.prefix + "_" + base + ".bmp";

//	std::cerr << "Loading " << path << std::endl;

	if (SDL_RWops *rw = load_or_grab_image(path)) {
		SDL_Surface *dest = SDL_LoadBMP_RW(rw, 1); // rwops is closed by loadbmp

		return dest;
	}

	return NULL;
}
SdlRenderer::
SdlRenderer(int type) :
    lastclick_(0), res_(res[type]), card_rend_(".", *this)
{
    int i;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::string("Unable to initialize SDL.");

    for (i = 0; i < WIDGET_NUM; ++i)
        widgets_[i] = NULL;

    atexit(SDL_Quit);

    screen_size_ = Point(res_.screen_width, res_.screen_height);

    scaling_ = res_.scaling;

    if (!(screen_ = SDL_SetVideoMode(screen_size_.X(), screen_size_.Y(), 
				16, OPENFLAGS)))
        throw std::string("Unable to open display.");

	if (widgets_[QUIT_GAME] = load_image("close") ) {
        widget_positions_[QUIT_GAME].set(screen_->w - widgets_[QUIT_GAME]->w, 0,
                                     widgets_[QUIT_GAME]->w, widgets_[QUIT_GAME]->h);
    }

    if (widgets_[UNDO_MOVE] = load_image("undo") ) {
            widget_positions_[UNDO_MOVE].set(screen_->w - widgets_[UNDO_MOVE]->w, 
                                         (screen_->h - widgets_[UNDO_MOVE]->h) / 2,
                                         widgets_[UNDO_MOVE]->w, widgets_[UNDO_MOVE]->h);
    }

    if (widgets_[NEW_GAME] = load_image("new") ) {
            widget_positions_[NEW_GAME].set(screen_->w - widgets_[NEW_GAME]->w, 
                                        screen_->h - widgets_[NEW_GAME]->h,
                                        widgets_[NEW_GAME]->w, widgets_[NEW_GAME]->h);
    }

    background_ =  SDL_MapRGB(screen_->format,
                        0, 200, 0);
    white_ = SDL_MapRGB(screen_->format,
                        255, 255, 255);
    black_ = SDL_MapRGB(screen_->format,
                        0, 0, 0);

    SDL_Rect r = { 0, 0, screen_->w, screen_->h};
    SDL_SetClipRect(screen_, &r);

    deck_position_ = Point(res_.spacing_x / 2, res_.spacing_y / 2);
    card_size_ = Point(res_.card_width, 
                       res_.card_height);

    cards_position_ = Point(deck_position_.X() + card_size_.X() + res_.spacing_x,
                            deck_position_.Y());

    card_rend_.Optimize();

    for (i = 0; i < TOTAL_SEEDS; i++) {
        seed_positions_[i] = Point(cards_position_.X() + res_.spacing_x + 
                                   (i + 2) * (card_size_.X() + res_.spacing_x),
                                   deck_position_.Y());
    }

    for (i = 0; i < COLUMNS; i++) {
        column_positions_[i] = Point(deck_position_.X() + 
                                   i * (card_size_.X() + res_.spacing_x),
                                   deck_position_.Y() + res_.spacing_y * 2 + card_size_.Y());
    }

	Clear();
}

SdlCardRenderer::
SdlCardRenderer(const std::string &path, SdlRenderer &rend) :
    rend_(rend)
{
    if (! (source_ = rend_.load_image("seeds")))
        throw std::string("Unable to load cards GFX");

    if (!(back_ = rend_.load_image("back")))
        throw std::string("Unable to load card background GFX");

    sw_ = rend_.res_.symbol_width;
    sh_ = rend_.res_.symbol_height;
}

void SdlCardRenderer::
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

void SdlCardRenderer::
Refresh(const Card &card)
{ 
    std::map<Card,CardData>::iterator it;

    if ((it = card_datas_.find(card)) != card_datas_.end()) 
        Draw(card, it->second.Pos());
}

void SdlCardRenderer::
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

void SdlCardRenderer::
Move(const Card &card, const Point &pos)
{
    std::map<Card,CardData>::iterator it;
	Point delta, start;

    if ((it = card_datas_.find(card)) != card_datas_.end()) {
		start = it->second.Pos();

		delta = pos - start;

		double steps = sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());

        double x = start.X(), y = start.Y();

        for (int i = 0; i < (int)steps; i+=4) {
            x += ((double)delta.X()) / (steps / 4.0);
            y += ((double)delta.Y()) / (steps / 4.0);

			start.set((int)x, (int)y);

			Draw(card,  start);

			rend_.Update();

			SDL_Delay(5);
		}
	}


	Draw(card, pos);
}

void SdlCardRenderer::
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

    r.x = pos.X() + rend_.res_.spacing_x;
    r.y = pos.Y() + rend_.res_.spacing_y;

    SDL_Rect s = { (card.Value() != 1) ? sw_ * card.Value() :
                 14 * sw_, card.IsBlack() ? 0 : sh_, sw_, sh_ };

    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ - rend_.res_.spacing_y / 2;

    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    if (card.IsRed())
        s.x = 15 * sw_ + ((card.Seed() == Hearts) ? 0 : sw_);
    else
        s.x = 15 * sw_ + ((card.Seed() == Flowers) ? 0 : sw_);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.res_.spacing_y;
    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

    r.x = pos.X() + rend_.res_.spacing_x;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ -  rend_.res_.spacing_y / 2;
    SDL_BlitSurface(source_, &s,rend_.screen_, &r);

}

