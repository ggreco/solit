#include "lowres_renderer.h"

int main()
{
    Renderer *r = new LowresRenderer();

    Game game(r);

    r->Update();
    r->Wait();

    return 0;
}

