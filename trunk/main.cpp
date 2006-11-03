#include "lowres_renderer.h"
#include <iostream>

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
	    Renderer *r = new SdlRenderer(RENDERER_SET);
	    Game game(r);

		r->Update();
		r->Wait();
	}
	catch (std::string &error) {
		std::cerr << "Error: " << error << std::endl;
	}
	catch (...) {
		std::cerr << "Unhandled exception!\n";
	}


    return 0;
}

