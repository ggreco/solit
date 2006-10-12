#include "lowres_renderer.h"
#include <iostream>

extern void init_memory_images();

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
		init_memory_images();

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

