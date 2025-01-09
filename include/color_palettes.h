#ifndef COLOR_PALETTES_H
#define COLOR_PALETTES_H

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

extern const Color sandPalette[];
extern const Color waterPalette[];
extern const Color stonePalette[];
extern const Color lavaPalette[];
extern const Color steamPalette[];

#define SAND_PALETTE_SIZE 6
#define WATER_PALETTE_SIZE 6
#define STONE_PALETTE_SIZE 6
#define LAVA_PALETTE_SIZE 6
#define STEAM_PALETTE_SIZE 6

#endif /* COLOR_PALETTES_H_*/