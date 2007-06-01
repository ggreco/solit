#include "lowres_renderer.h"
#include <iostream>
#include "kgame.h"
#include "sgame.h"

#ifndef WINCE

#define RENDERER_SET 1
int main(int argc, char *argv[])
#else

#define RENDERER_SET 0

extern "C" {
int SDL_main(int argc, char *argv[]);
}

int SDL_main(int argc, char *argv[])
#endif
{
	try {
        int gametype = 0;

        if (argc == 2)
            gametype = atoi(argv[1]);

        Game *game = NULL;

        switch (gametype) {
            case 0:
        	    game = new KlondikeGame(RENDERER_SET);
                break;
            case 1:
        	    game = new SpiderGame(RENDERER_SET);
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

