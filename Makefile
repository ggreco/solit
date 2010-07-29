IMGS = images/low_back.bmp images/low_close.bmp images/low_new.bmp \
	   images/low_seeds.bmp images/low_undo.bmp images/med_back.bmp \
	   images/med_close.bmp images/med_new.bmp images/med_seeds.bmp \
	   images/med_undo.bmp

SDLCONFIG = /home/gabry/no-backup/bin/sdl-config

OBJS = main.o lowres_renderer.o deck.o game.o renderer.o card.o \
       images.o sgame.o kgame.o serializer.o streamer.o

CXXFLAGS = -Wall -g `$(SDLCONFIG) --cflags`
TARGET = solit

all: iconbuild $(TARGET)

iconbuild: iconbuild.cpp
	g++ -o iconbuild iconbuild.cpp
	rm -f images.cpp

images.cpp: $(IMGS) iconbuild
	./iconbuild images.cpp $(IMGS)

$(TARGET): $(OBJS)
	g++ -o $@ $(OBJS) `$(SDLCONFIG) --libs`

clean:
	rm -f *.o $(TARGET) images.cpp iconbuild
