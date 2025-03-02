#ifndef TILE_H
#define TILE_H

#include "coords.h"
#include "state.h"

#define TILE_SIZE 4
#define TILE_PERMUTATIONS 24 // = TILE_SIZE!

#define NUM_TILE_DIRECTIONS 12

enum TileDirection {
    // Starting from one of the end hexes
    TILE_END_EAST = 0,
    TILE_END_SOUTHEAST,
    TILE_END_SOUTHWEST,
    TILE_END_WEST,
    TILE_END_NORTHWEST,
    TILE_END_NORTHEAST,

    // Starting from the middle hexes
    TILE_MID_NORTH,
    TILE_MID_NORTHEAST,
    TILE_MID_SOUTHEAST,
    TILE_MID_SOUTH,
    TILE_MID_SOUTHWEST,
    TILE_MID_NORTHWEST
};

extern const char* TILE_DIRECTION_CODES[NUM_TILE_DIRECTIONS];

struct Tile {
    struct Coords origin;
    enum TileDirection direction;
};

void Tile_coords(const struct Tile* tile, struct Coords coords[]);

void Tile_permutations(const struct Tile* tile, struct Coords permutations[][TILE_SIZE]);

#endif
