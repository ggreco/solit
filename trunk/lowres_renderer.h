#ifndef LOWRES_RENDERER_H

#define LOWRES_RENDERER_H

#include "SDL.h"
#include "renderer.h"
#include <string>
#include <map>
#include <vector>

struct ResInfo
{
    int screen_width, screen_height;
    int card_width, card_height;
    int spacing_x, spacing_y;
    int symbol_width, symbol_height;
    double scaling;
    const char *prefix;
};

class SdlRenderer;

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

class SdlCardRenderer : public CardRenderer
{
    private:
        SDL_Surface *source_, *back_;
        SdlRenderer &rend_;
        std::map<Card,CardData> card_datas_;
        int sw_, sh_;
    public:
        SdlCardRenderer(const std::string &path, SdlRenderer &rend);

        void Draw(const Card &, const Point &);
        void Move(const Card &, const Point &);
        void Refresh(const Card &);
        void Optimize();
        void Clear(const Card &);
		void ClearBuffers() { card_datas_.clear(); }
};

class SdlRenderer : public Renderer
{
    private:
        Uint32 lastclick_;
        ResInfo &res_;
        SDL_Surface *screen_;
        Uint32 white_, black_, background_;
        SdlCardRenderer card_rend_;
        std::vector<SDL_Rect> rects_;
        SDL_Surface *widgets_[WIDGET_NUM];
        SDL_Surface * MakeDirty() {
            return SDL_CreateRGBSurface(SDL_SWSURFACE,
                    card_size_.X(), card_size_.Y(), 16,
                    screen_->format->Rmask,
                    screen_->format->Gmask,
                    screen_->format->Bmask,
                    screen_->format->Amask); }

        void DrawRect(const Point &pos, const Point &size, Uint32 color);
        void DrawEmpty(const Point &pos) { DrawRect(pos, card_size_, background_); }
        friend class SdlCardRenderer;
        void ParseEvent(const SDL_Event &);

    public:
		void Clear();
        void AddDirtyRect(const SDL_Rect &r) { if (r.x < screen_->w && r.y < screen_->h) rects_.push_back(r); }
        void UpdateAll();
        void Update();
        void Delay(int ms) { SDL_Delay(ms); }
        void Poll();
        bool Wait();
        SdlRenderer(int);
        ~SdlRenderer() {};
        CardRenderer *GetCardRenderer() { return &card_rend_; }
};

#endif

