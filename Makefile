all:
	gcc -std=c11 src/main.c -o build/SandScape -I"include/SDL2" -L"lib" -Wall -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf