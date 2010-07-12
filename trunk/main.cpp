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
        int gametype = 1;
        int renderset = RENDERER_SET;

        if (argc >= 2)
            gametype = atoi(argv[1]);
        if (argc == 3)
            renderset = atoi(argv[2]);

        Game *game = NULL;

        switch (gametype) {
            case 0:
        	    game = new KlondikeGame(renderset);
                break;
            case 1:
        	    game = new SpiderGame(renderset);
                break;
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

