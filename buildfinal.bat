windres winresource.rc -o winresource.o
gcc -m64 -DGLEW_STATIC -static main.c chroma.c chromagl.c util.c  winresource.o -O -fno-exceptions -I./inc -L./lib  -lmingw32 -lglew32 -lopengl32 -lSDL2main -lSDL2 -lgl-matrix -lole32 -loleaut32 -limm32 -lwinmm -lversion -luuid -ladvapi32 -lsetupapi -lshell32 -ldinput8 -mwindows -o chromadrencher.exe
strip -s chromadrencher.exe
