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

class SdlCardRenderer : public CardRenderer
{
    private:
        SDL_TextureID source_;
        SDL_TextureID back_;
        SdlRenderer &rend_;
        int sw_, sh_;
    public:
        SdlCardRenderer(const std::string &path, SdlRenderer &rend);

        SDL_TextureID Back() { return back_; }
        void Draw(const Card &, const Point &);
        void Move(const Card &, const Point &);
};

class SdlRenderer : public Renderer
{
    private:
        SDL_WindowID screen_;
        Uint32 lastclick_;
        ResInfo &res_;
        int w_, h_;
        SDL_Color white_, black_, background_;
        SdlCardRenderer *card_rend_;
        SDL_TextureID widgets_[WIDGET_NUM];

        void DrawRect(const Point &pos, const Point &size, SDL_Color & color);
        void DrawEmpty(const Point &pos) { DrawRect(pos, card_size_, background_); }
        friend class SdlCardRenderer;
        void ParseEvent(const SDL_Event &);

    public:
		void Clear();
        void Update();
        void Delay(int ms) { SDL_Delay(ms); }
        void Poll();
        bool Wait();
        SdlRenderer(int id, int cols, int seeds, bool card_slot);
        ~SdlRenderer() {};
        CardRenderer *GetCardRenderer() { return card_rend_; }

		SDL_TextureID load_image(const std::string &);

        Point WidgetSize(WidgetId id) {
            int w, h;
            SDL_QueryTexture(widgets_[id], NULL, NULL, &w, &h);
            return Point(w, h);
        }
        size_t WidgetWidth(WidgetId id) const { 
            int w, h;
            SDL_QueryTexture(widgets_[id], NULL, NULL, &w, &h);
            return w; 
        }
        size_t WidgetHeight(WidgetId id) const { 
            int w, h;
            SDL_QueryTexture(widgets_[id], NULL, NULL, &w, &h);
            return h; 
        }
        size_t ScreenWidth() const { return w_; }
        size_t ScreenHeight() const { return h_; }
};

#endif

