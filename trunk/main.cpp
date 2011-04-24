#include "lowres_renderer.h"
#include <iostream>
#include "kgame.h"
#include "sgame.h"

#ifdef XIPHONE
#define RENDERER_SET 1
#elif defined(WINCE)
#define RENDERER_SET 0
#else
#define RENDERER_SET 2
#endif

#ifndef WINCE
int main(int argc, char *argv[])
#else
extern "C" int SDL_main(int argc, char *argv[])
#endif
{
	try {
#ifdef ISKLONDIKE
        int gametype = KLONDIKE_ID; 
#else
        int gametype = SPIDER_ID; 
#endif
        int renderset = RENDERER_SET;

#ifndef XIPHONE
        if (argc >= 2)
            gametype = atoi(argv[1]);
        if (argc == 3)
            renderset = atoi(argv[2]);
#endif
        Game *game = NULL;

        std::cerr << "Starting game type " << gametype << "\n";
    
        switch (gametype) {
            case KLONDIKE_ID:
        	    game = new KlondikeGame(renderset);
                break;
            case SPIDER_ID:
        	    game = new SpiderGame(renderset);
                break;
            default:
                std::cerr << "Unknown game " << gametype << "\n";
                return 0;
        }
        if (game) {
    		game->Rend()->Update();
	    	game->Rend()->Wait();
        }
	}
	catch (std::string &error) {
		std::cerr << "Error: " << error << std::endl;
	}
	catch (...) {
		std::cerr << "Unhandled exception!\n";
	}


    return 0;
}

