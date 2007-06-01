IMGS = images/low_back.bmp images/low_close.bmp images/low_new.bmp \
	   images/low_seeds.bmp images/low_undo.bmp images/med_back.bmp \
	   images/med_close.bmp images/med_new.bmp images/med_seeds.bmp \
	   images/med_undo.bmp 


OBJS = main.o lowres_renderer.o deck.o game.o renderer.o card.o \
       images.o sgame.o kgame.o

CXXFLAGS = -g `sdl-config --cflags`
TARGET = solit

all: iconbuild $(TARGET)

iconbuild: iconbuild.cpp
	g++ -o iconbuild iconbuild.cpp
	-rm images.cpp

images.cpp: $(IMGS)
	./iconbuild images.cpp $(IMGS)

$(TARGET): $(OBJS)
	g++ -o $@ $(OBJS) `sdl-config --libs`

clean:
	rm -f *.o $(TARGET) images.cpp iconbuild
