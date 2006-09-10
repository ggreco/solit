#include "lowres_renderer.h"
#include <iostream>

#ifndef WINCE
int main(int argc, char *argv[])
#else
extern "C" {
int SDL_main(int argc, char *argv[]);
}

int SDL_main(int argc, char *argv[])
#endif
{
	try {
	    Renderer *r = new LowresRenderer();
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

