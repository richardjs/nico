#ifndef TILE_H
#define TILE_H

#include "coords.h"
#include "state.h"

#define TILE_SIZE 4
#define TILE_PERMUTATIONS 24 // = TILE_SIZE!

enum TileDirection {
    TILE_EAST = 0,
    TILE_SOUTHEAST,
    TILE_SOUTHWEST,
    TILE_WEST,
    TILE_NORTHWEST,
    TILE_NORTHEAST
};

extern const char* TILE_DIRECTION_CODES[NUM_DIRECTIONS];

struct Tile {
    struct Coords origin;
    enum TileDirection direction;
};

void Tile_coords(const struct Tile* tile, struct Coords coords[]);

void Tile_permutations(const struct Tile* tile, struct Coords permutations[][TILE_SIZE]);

#endif
