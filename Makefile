OBJS = main.o lowres_renderer.o deck.o game.o renderer.o card.o

CXXFLAGS = -g `sdl-config --cflags`
TARGET = solit

$(TARGET): $(OBJS)
	g++ -o $@ $(OBJS) `sdl-config --libs`

clean:
	rm -f *.o
