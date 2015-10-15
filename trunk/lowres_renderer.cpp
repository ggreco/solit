#include "lowres_renderer.h"
#include <string>
#include <iostream>
#include <math.h>
//#include "SDL_opengles.h"

ResInfo ressize[2][7] = {
    {
        {320, 240, 20, 30, 2, 2, 8, 8, 1.0, 1.0, "low"},
        {480, 320, 26, 45, 6, 3, 8, 8, 1.5, 1.33333, "low"},
        {640, 480, 40, 60, 6, 4, 13, 16, 2.0, 2.0, "med"},
        {960, 640, 52, 90, 12, 6, 13, 16, 3.0, 2.66666, "high"},
        {1024, 768, 64, 96, 16 , 8, 13, 16, 3.2 , 3.2 ,"high"},
        {1136, 640, 64, 90, 16 , 8, 13, 16, 3.2 , 3.0 ,"high"},
        {2048, 1536, 64, 90, 16 , 8, 13, 16, 3.2 , 3.0 ,"high"}
    },
    {
        {240, 320, 20, 30, 4, 2, 8, 8, 1.0, 1.0, "low"}, // pocketpc
        {320, 480, 26, 45, 6, 3, 8, 8, 1.33333, 1.5, "low"}, // iphone
        {480, 640, 40, 60, 8, 4, 13, 16, 2.0, 2.0, "med"}, // pc
        {640, 960, 52, 90, 12, 6, 13, 16, 2.66666, 3.0, "high"}, // iphone 4
        {768, 1024, 64, 96, 13 , 8, 13, 16, 2.66666 , 3.2 ,"high"}, // ipad
        {640, 1136, 52, 90, 12, 6, 13, 16, 2.66666, 3.0, "high"}, // iphone 5
        {1536, 2048, 128, 192, 26, 16, 13, 16, 5.33333, 6.4, "high"} // new ipad
    }
};

void SdlRenderer::
Clear()
{
    int i;

    SDL_SetRenderDrawColor(renderer_, background_.r, background_.g, background_.b, background_.a);
    SDL_RenderClear(renderer_);

	for (i = 0; i < Seeds(); i++)
        DrawRect(seed_positions_[i], card_size_, white_);

    if (highlighted_ != -1) {
        SDL_Rect r = {
                        column_positions_[highlighted_].X() - spacing_.X() / 2,  // X
                        column_positions_[highlighted_].Y() - spacing_.Y() / 2,  // Y
                        card_size_.X() + spacing_.X(), // Width
                        column_heights_[highlighted_] + spacing_.Y() // Height
                     }; // W e H

        SDL_SetRenderDrawColor(renderer_, highlight_color_.r, highlight_color_.g, 
                    highlight_color_.b, highlight_color_.a);
        SDL_RenderFillRect(renderer_, &r);

        int start_y = column_positions_[highlighted_].Y() + column_heights_[highlighted_] + spacing_.Y() / 2;
        int len = screen_size_.Y() - start_y;
        float actual_b = highlight_color_.b;
        float step = (float)highlight_color_.b / (float)len;
        float actual_w = r.w;
        float actual_x = r.x;
        float w_step = (actual_w / 2.0f) / (float)len;

        for (int i = 0; i < len; ++i) {
            r.y = start_y + i;
            r.h = 1;

            actual_w -= w_step;
            actual_x += w_step / 2.0f;
            actual_b -= step;

            if (actual_w <= 0.0f)
                break;

            // if two rows are equal I do them in a single blit
            if ( ((int)actual_w) == ((int)(actual_w + w_step)) &&
                 ((int)actual_b) == ((int)(actual_b + step)) ) {
                i++;
                r.h = 2;
            }

            r.x = actual_x;
            r.w = actual_w;


            SDL_SetRenderDrawColor(renderer_, 
                                   highlight_color_.r, highlight_color_.g, (unsigned char)actual_b, 
                                   highlight_color_.a);
            SDL_RenderFillRect(renderer_, &r);
        }
    }

    for (i = 0; i < WIDGET_NUM; i++) {
        if (widgets_[i]) {
            SDL_Rect dst;

            dst.x = widget_positions_[i].X();
            dst.y = widget_positions_[i].Y();
            Point p = WidgetSize((WidgetId)i);
            dst.w = p.X();
            dst.h = p.Y();

            SDL_RenderCopy(renderer_, widgets_[i], NULL, &dst);
        }
    }
}

void SdlRenderer::
DrawRect(const Point &pos, const Point &size, SDL_Color &color)
{
    SDL_Rect r = { pos.X(), pos.Y(), size.X(), size.Y() };

    SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer_, &r);

    r.x++;
    r.y++;

    r.w -= 2;
    r.h -= 2;

    SDL_SetRenderDrawColor(renderer_, background_.r, background_.g, background_.b, background_.a);
    SDL_RenderFillRect(renderer_, &r);
}

void SdlRenderer::
Update()
{
    SDL_RenderPresent(renderer_);    
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
            Renderer::MouseMove(Point(e.motion.x * ws_, e.motion.y * hs_));
            break;
        case SDL_MOUSEBUTTONDOWN:
            Renderer::PressButton(Point(e.button.x * ws_, e.button.y * hs_));
            break;                    
        case SDL_MOUSEBUTTONUP:
            Renderer::ReleaseButton(Point(e.button.x * ws_, e.button.y * hs_));

            if ((SDL_GetTicks() - lastclick_) < 300) {
                Renderer::DoubleClick(Point(e.button.x * ws_, e.button.y * hs_));
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
                case SDL_WINDOWEVENT_RESIZED: {
                        int x, y;
                        SDL_GetWindowSize(screen_, &x, &y);
                        std::cerr << "Received " << x << "x" << y << "\n";
                    }
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


SDL_Texture *SdlRenderer::
load_image(const std::string &base)
{
	extern SDL_RWops *load_or_grab_image(const std::string &);
	std::string path = std::string("./") + res_.prefix + "_" + base + ".bmp";

	if (SDL_RWops *rw = load_or_grab_image(path)) {
        if (SDL_Surface *dest = SDL_LoadBMP_RW(rw, 1)) { // rwops is closed by loadbmp
            SDL_Texture *id = SDL_CreateTextureFromSurface(renderer_, dest);

            if (id == NULL)
                std::cerr << "SDL Error: " << SDL_GetError() << '\n';

            SDL_FreeSurface(dest);
            return id;
        }
	}
	return 0;
}

SdlRenderer::
SdlRenderer(int id, int res, int cols, int seeds, bool card_slot) :
    Renderer(cols, seeds, card_slot),
    lastclick_(0), res_(ressize[id][res])
{
    int i;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::string("Unable to initialize SDL.");

    for (i = 0; i < WIDGET_NUM; ++i)
        widgets_[i] = 0;

// debug code to see pixel formats available on different platforms.
    int available_displays = SDL_GetNumDisplayModes(0);
    bool found = false;
    
    for (int i = 0; i < available_displays; ++i)  {
        SDL_DisplayMode mode;
        SDL_GetDisplayMode(0, i, &mode);
        std::cerr << i << ") " << mode.w << 'x' << mode.h << '/' << SDL_GetPixelFormatName(mode.format) << '\n';
        
#ifdef XIPHONE
        if (mode.h == 568) {
            res_ = ressize[id][5];
            break;
        }
        else if (mode.h == 1024) {
            res_ = ressize[id][4];
            found = true;
            break;
        }
        else if (mode.h == 480) {
            res_ = ressize[id][3];
            break;
        }
#else
        for (int j = 0; j < sizeof(ressize[0])/sizeof(ressize[0][0]); ++j)
            if (mode.w == ressize[id][j].screen_width &&
                mode.h == ressize[id][j].screen_height) {
                if (!found || res_.screen_width < mode.w) {
                    res_ = ressize[id][j];
                    found = true;
                }
            }
#endif
    }
    
    atexit(SDL_Quit);

    screen_size_ = Point(res_.screen_width, res_.screen_height);

    std::cerr << "Res:" << screen_size_.X() << 'x' << screen_size_.Y() << '\n';
    if (!(screen_ = SDL_CreateWindow("Solit", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     screen_size_.X(), screen_size_.Y(), found ? 0 : SDL_WINDOW_ALLOW_HIGHDPI)))
        throw std::string("Unable to open display.");

    SDL_GetWindowSize(screen_, &w_, &h_);
    std::cerr << "Window size set to: " << w_ << 'x' << h_ << '\n';
    
    
    if (!(renderer_ = SDL_CreateRenderer(screen_, -1 /*selected*/, 0)))
        throw std::string("Unable to create renderer");
    
    int w, h;
    SDL_GetRendererOutputSize(renderer_, &w, &h);
    
    for (size_t i = 0; i < sizeof(ressize[0])/sizeof(ressize[0][0]); ++i) {
        if (ressize[id][i].screen_width == w &&
            ressize[id][i].screen_height == h) {
            res_ = ressize[id][i];
            break;
        }
    }
    x_scaling_ = res_.xscaling;
    y_scaling_ = res_.yscaling;
    screen_size_ = Point(res_.screen_width, res_.screen_height);

    
	if (!(widgets_[QUIT_GAME] = load_image("close") )) 
        throw std::string("Unable to load close image");

    if (!(widgets_[UNDO_MOVE] = load_image("undo") ))
        throw std::string("Unable to load undo image");

    if (!(widgets_[NEW_GAME] = load_image("new") ))
        throw std::string("Unable to load close image");

    card_rend_ = new SdlCardRenderer(".", *this);


    ws_ = (float)w / (float)w_;
    hs_ = (float)h / (float)h_;

    std::cerr << "Renderer output size: " << w << 'x' << h << '\n';
    background_.r = 0; background_.g = 200; background_.b = 0; background_.a = SDL_ALPHA_OPAQUE;
    memset(&white_, 255, sizeof(white_));
    memset(&black_, 0, sizeof(black_));
    black_.a = SDL_ALPHA_OPAQUE;

    // highlight color is like background with a blue component added
    highlight_color_ = background_;
    highlight_color_.b = 240;

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
        SDL_RenderCopy(rend_.renderer(), back_, NULL, &r);
        return;
    }

    // draw the black border of the card
    SDL_SetRenderDrawColor(rend_.renderer(), rend_.black_.r, rend_.black_.g, 
                           rend_.black_.b, rend_.black_.a);
    SDL_RenderFillRect(rend_.renderer(), &r);

    r.x++;
    r.y++;
    r.w -= 2;
    r.h -= 2;

    // draw the white square
    SDL_SetRenderDrawColor(rend_.renderer(), rend_.white_.r, rend_.white_.g, 
                           rend_.white_.b, rend_.white_.a);
    SDL_RenderFillRect(rend_.renderer(), &r);

    r.x = pos.X() + rend_.res_.spacing_x / 2 + 1;
    r.y = pos.Y() + rend_.res_.spacing_y;
    r.w = sw_;
    r.h = sh_;

    SDL_Rect s = { (card.Value() != 1) ? sw_ * card.Value() :
                 14 * sw_, card.IsBlack() ? 0 : sh_, sw_, sh_ };

    // put the number on the top left
    SDL_RenderCopy(rend_.renderer(), source_, &s, &r);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ - rend_.res_.spacing_y / 2;

    // and on the bottom right
    SDL_RenderCopy(rend_.renderer(), source_, &s, &r);

    if (card.IsRed())
        s.x = 15 * sw_ + ((card.Seed() == Hearts) ? 0 : sw_);
    else
        s.x = 15 * sw_ + ((card.Seed() == Flowers) ? 0 : sw_);

    r.x = pos.X() + rend_.card_size_.X() - sw_ - rend_.res_.spacing_x / 2;
    r.y = pos.Y() + rend_.res_.spacing_y;
    SDL_RenderCopy(rend_.renderer(), source_, &s, &r);

    r.x = pos.X() + rend_.res_.spacing_x;
    r.y = pos.Y() + rend_.card_size_.Y() - sh_ -  rend_.res_.spacing_y / 2;
    SDL_RenderCopy(rend_.renderer(), source_, &s, &r);

}

