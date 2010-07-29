#include "lowres_renderer.h"
#include <string>
#include <iostream>
#include <math.h>

ResInfo res[] = {
    {320, 240, 20, 30, 2, 2, 8, 8, 1.0, 1.0, "low"},
    {480, 320, 26, 45, 6, 3, 8, 8, 1.5, 1.33333, "low"},
    {640, 480, 40, 60, 6, 4, 13, 16, 2.0, 2.0, "med"},
    {240, 320, 20, 30, 4, 2, 8, 8, 1.0, 1.0, "low"},
    {320, 480, 26, 45, 6, 3, 8, 8, 1.33333, 1.5, "low"},
    {480, 640, 40, 60, 8, 4, 13, 16, 2.0, 2.0, "med"},
};

void SdlRenderer::
Clear()
{
    int i;

    SDL_SetRenderDrawColor(background_.r, background_.g, background_.b, background_.unused);
    SDL_RenderClear();

	for (i = 0; i < Seeds(); i++)
        DrawRect(seed_positions_[i], card_size_, white_);

    for (i = 0; i < WIDGET_NUM; i++) {
        if (widgets_[i]) {
            SDL_Rect dst;

            dst.x = widget_positions_[i].X();
            dst.y = widget_positions_[i].Y();
            Point p = WidgetSize((WidgetId)i);
            dst.w = p.X();
            dst.h = p.Y();

            SDL_RenderCopy(widgets_[i], NULL, &dst);
        }
    }
}

void SdlRenderer::
DrawRect(const Point &pos, const Point &size, SDL_Color &color)
{
    SDL_Rect r = { pos.X(), pos.Y(), size.X(), size.Y() };

    SDL_SetRenderDrawColor(color.r, color.g, color.b, color.unused);
    SDL_RenderFillRect(&r);

    r.x++;
    r.y++;

    r.w -= 2;
    r.h -= 2;

    SDL_SetRenderDrawColor(background_.r, background_.g, background_.b, background_.unused);
    SDL_RenderFillRect(&r);
}

void SdlRenderer::
Update()
{
    SDL_RenderPresent();    
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
            Renderer::OnQuit();
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
        case SDL_WINDOWEVENT:
            switch (e.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    Renderer::OnQuit();
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    Renderer::Exposed(); 
                    break;
            }
            break;
    }
}

void SdlRenderer::
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

SDL_Texture *SdlRenderer::
load_image(const std::string &base)
{
	extern SDL_RWops *load_or_grab_image(const std::string &);
	std::string path = std::string("./") + res_.prefix + "_" + base + ".bmp";
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(&mode);

//    std::cerr << "pixelformat: " << mode.format << '\n';
	if (SDL_RWops *rw = load_or_grab_image(path)) {
        if (SDL_Surface *dest = SDL_LoadBMP_RW(rw, 1)) { // rwops is closed by loadbmp
            int bpp;
            Uint32  Rmask, Gmask, Bmask, Amask;
            SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);

//            std::cerr << "bpp: " << bpp << '\n';
//            std::cerr << "r:" << (void *)Rmask << " g:" << (void *)Gmask << " b:" << (void *)Bmask << '\n';

            if (SDL_Surface *temp = SDL_CreateRGBSurface(0 , dest->w, dest->h, bpp,
                        Rmask, Gmask, Bmask, Amask
                        )) {
                SDL_BlitSurface(dest, NULL, temp, NULL);
                SDL_Texture *id = SDL_CreateTextureFromSurface(mode.format, temp);

                if (id == 0)
                    std::cerr << "SDL Error: " << SDL_GetError() << '\n';

                SDL_SetTextureScaleMode(id, SDL_TEXTURESCALEMODE_FAST);

                SDL_FreeSurface(temp);
                SDL_FreeSurface(dest);
                return id;
            }
            SDL_FreeSurface(dest);
        }
	}
	return 0;
}
SdlRenderer::
SdlRenderer(int type, int cols, int seeds, bool card_slot) :
    Renderer(cols, seeds, card_slot),
    lastclick_(0), res_(res[type])
{
    int i;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::string("Unable to initialize SDL.");

    for (i = 0; i < WIDGET_NUM; ++i)
        widgets_[i] = 0;

    atexit(SDL_Quit);

    screen_size_ = Point(res_.screen_width, res_.screen_height);

    x_scaling_ = res_.xscaling;
    y_scaling_ = res_.yscaling;

    std::cerr << screen_size_.X() << ',' << screen_size_.Y() << '\n';
    if (!(screen_ = SDL_CreateWindow("Solit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     screen_size_.X(), screen_size_.Y(), 0)))
        throw std::string("Unable to open display.");

    w_ = screen_size_.X();
    h_ = screen_size_.Y();

    int n = SDL_GetNumRenderDrivers();
    int selected = -1;

    for (int i = 0; i < n; ++i) {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(i, &info);        

        std::cerr << "Renderer " << i << ") " << info.name 
                  << " scale modes: " << info.scale_modes << '\n';

        if (selected < 0)
            if (info.scale_modes || !strcmp(info.name, "opengl") ) 
                selected = i;
    }
    std::cerr << "Selected: " << selected << '\n';

    if (SDL_CreateRenderer(screen_, selected, 0) == -1) 
        throw std::string("Unable to create accelerated renderer");

    if (SDL_SelectRenderer(screen_) == -1)
        throw std::string("Unable to select render target");

	if (!(widgets_[QUIT_GAME] = load_image("close") )) 
        throw std::string("Unable to load close image");

    if (!(widgets_[UNDO_MOVE] = load_image("undo") ))
        throw std::string("Unable to load undo image");

    if (!(widgets_[NEW_GAME] = load_image("new") ))
        throw std::string("Unable to load close image");

    card_rend_ = new SdlCardRenderer(".", *this);

    background_.r = 0; background_.g = 200; background_.b = 0; background_.unused = SDL_ALPHA_OPAQUE;
    memset(&white_, 255, sizeof(white_));
    memset(&black_, 0, sizeof(black_));
    black_.unused = SDL_ALPHA_OPAQUE;

    card_size_ = Point(res_.card_width, res_.card_height);

    spacing_ = Point(res_.spacing_x, res_.spacing_y);

    SDL_ShowWindow(screen_);
}

SdlCardRenderer::
SdlCardRenderer(const std::string &path, SdlRenderer &rend) :
    rend_(rend)
{
    if (!(source_ = rend_.load_image("seeds")))
        throw std::string("Unable to load cards GFX");

    if (!(back_ = rend_.load_image("back")))
        throw std::string("Unable to load card background GFX");

    sw_ = rend_.res_.symbol_width;
    sh_ = rend_.res_.symbol_height;
}

void SdlCardRenderer::
Move(const Card &card, const Point &pos)
{

    /* TODO
    std::map<Card,CardData>::iterator it;
	Point delta, start;
    if ((it = card_datas_.find(card)) != card_datas_.end()) {
		start = it->second.Pos();

		delta = pos - start;

		double steps = sqrt(delta.X() * delta.X() + delta.Y() * delta.Y());

        double x = start.X(), y = start.Y();

        for (int i = 0; i < (int)steps; i+=5) {
            x += ((double)delta.X()) / (steps / 5.0);
            y += ((double)delta.Y()) / (steps / 5.0);

            if (x < 0.0)
                x = 0;

            if (y < 0.0)
                y = 0;

			start.set((int)x, (int)y);

			Draw(card,  start);

			rend_.Update();

			SDL_Delay(5);
		}
	}
*/

	Draw(card, pos);
}

void SdlCardRenderer::
Draw(const Card &card, const Point &pos)
{
    SDL_Rect r = { pos.X(), pos.Y(), 
                   rend_.card_size_.X(),
                   rend_.card_size_.Y() };

    if (card.Covered()) {
        SDL_RenderCopy(back_, NULL, &r);
        return;
    }

    // draw the black border of the card
    SDL_SetRenderDrawColor(rend_.black_.r, rend_.black_.g, rend_.black_.b, rend_.black_.unused);
    SDL_RenderFillRect(&r);

    r.x++;
    r.y++;
    r.w -= 2;
    r.h -= 2;

    // draw the white square
    SDL_SetRenderDrawColor(rend_.white_.r, rend_.white_.g, rend_.white_.b, rend_.white_.unused);
    SDL_RenderFillRect(&r);

    r.x = pos.X() + rend_.res_.spacing_x / 2 + 1;
    r.y = pos.Y() + rend_.res_.spacing_y;
    r.w = sw_;
    r.h = sh_;

    SDL_Rect s = { (card.Value() != 1) ? sw_ * card.Value() :
                 14 * sw_, card.IsBlack() ? 0 : sh_, sw_, sh_ };

    // put the number on the top left
    SDL_RenderCopy(source_, &s, &r);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ - rend_.res_.spacing_y / 2;

    // and on the bottom right
    SDL_RenderCopy(source_, &s, &r);

    if (card.IsRed())
        s.x = 15 * sw_ + ((card.Seed() == Hearts) ? 0 : sw_);
    else
        s.x = 15 * sw_ + ((card.Seed() == Flowers) ? 0 : sw_);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.res_.spacing_y;
    SDL_RenderCopy(source_, &s, &r);

    r.x = pos.X() + rend_.res_.spacing_x;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ -  rend_.res_.spacing_y / 2;
    SDL_RenderCopy(source_, &s, &r);

}

