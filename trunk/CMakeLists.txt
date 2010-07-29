cmake_minimum_required(VERSION 2.6)

project(Solit)

set(IMAGES images/low_back.bmp images/low_close.bmp images/low_new.bmp
           images/low_seeds.bmp images/low_undo.bmp images/med_back.bmp
    	   images/med_close.bmp images/med_new.bmp images/med_seeds.bmp
	       images/med_undo.bmp)

find_package(SDL REQUIRED)
       
ADD_EXECUTABLE(iconbuild  iconbuild.cpp)
ADD_CUSTOM_COMMAND(
    OUTPUT images.cpp
    COMMAND iconbuild ${CMAKE_CURRENT_SOURCE_DIR}/images.cpp ${IMAGES})

ADD_EXECUTABLE(solit card.cpp game.cpp images.cpp lowres_renderer.cpp
               renderer.cpp deck.cpp kgame.cpp main.cpp sgame.cpp
               serializer.cpp streamer.cpp)

include_directories(${SDL_INCLUDE_DIR})
target_link_libraries(solit ${SDL_LIBRARY})

