cmake_minimum_required(VERSION 3.23)

project(ChromaDrencher)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
find_package(GLEW REQUIRED)

add_subdirectory(gl-matrix)

add_executable(ChromaDrencher WIN32
	chroma.c chroma.h
	chromagl.c chromagl.h
	main.c
	util.c util.h
	winresource.rc
)

if(TARGET SDL2::SDL2main)
	target_link_libraries(ChromaDrencher PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(ChromaDrencher
	PRIVATE SDL2::SDL2
	PRIVATE GLEW::glew_s
	PRIVATE gl-matrix
)