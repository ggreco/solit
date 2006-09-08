#ifndef LOWRES_RENDERER_H

#define LOWRES_RENDERER_H

#include "SDL.h"
#include "renderer.h"
#include <string>
#include <map>
#include <vector>

class LowresRenderer;

class CardData
{
    Point pos_;
    SDL_Surface *dirty_;

public:
    CardData() : dirty_(NULL) {}
    ~CardData() { if (dirty_) SDL_FreeSurface(dirty_); }
    SDL_Surface *Dirty() { return dirty_; }
    void Dirty(SDL_Surface *value) { dirty_ = value; }
    Point &Pos() { return pos_; }
    void Pos(const Point &p) { pos_ = p; }
};

class LowresCardRenderer : public CardRenderer
{
    private:
        SDL_Surface *source_, *back_;
        LowresRenderer &rend_;
        std::map<Card,CardData> card_datas_;
    public:
        LowresCardRenderer(const std::string &path, LowresRenderer &rend);

        void Draw(const Card &, const Point &);
        void Move(const Card &, const Point &) {}
        void Refresh(const Card &);
        void Optimize();
        void Clear(const Card &);
};

class LowresRenderer : public Renderer
{
    private:
        SDL_Surface *screen_;
        Uint32 white_, black_, background_;
        LowresCardRenderer card_rend_;
        std::vector<SDL_Rect> rects_;

        SDL_Surface * MakeDirty() {
            return SDL_CreateRGBSurface(SDL_SWSURFACE,
                    card_size_.X(), card_size_.Y(), 16,
                    screen_->format->Rmask,
                    screen_->format->Gmask,
                    screen_->format->Bmask,
                    screen_->format->Amask); }

        void DrawRect(const Point &pos, const Point &size, Uint32 color);
        void DrawEmpty(const Point &pos) { DrawRect(pos, card_size_, background_); }
        friend class LowresCardRenderer;

    public:
        void AddDirtyRect(const SDL_Rect &r) { if (r.x < screen_->w && r.y < screen_->h) rects_.push_back(r); }
        void UpdateAll();
        void Update();
        bool Wait();
        LowresRenderer();
        ~LowresRenderer() {};
        CardRenderer *GetCardRenderer() { return &card_rend_; }
};

#endif

