windres winresource.rc -o winresource.o
gcc -m64 -DDEBUG -g -static main.c chroma.c chromagl.c util.c winresource.o -O -fno-exceptions -I./inc -L./lib  -lmingw32 -lglew32 -lopengl32 -lSDL2main -lSDL2 -lgl-matrix -lgdi32 -lole32 -loleaut32 -limm32 -lwinmm -lversion -o chromadrencher.exe
::strip -s chromadrencher.exe
