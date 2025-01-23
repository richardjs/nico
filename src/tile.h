#ifndef TILE_H
#define TILE_H

#include "coords.h"
#include "state.h"

#define TILE_SIZE 4

/*
    /1,0\       /0,0\           /1,0\
/0,1\___/2,0\   \___/1,0\   /0,1\___/
\___/1,1\___/   /0,1\___/   \___/1,1\
    \___/       \___/1,1\   /0,2\___/
                    \___/   \___/
Type H^         Type L^     Type R^
*/

enum Rotation {
    H = 0,
    L,
    R
};

enum TileDirection {
    TILE_EAST = 0,
    TILE_SOUTHEAST,
    TILE_SOUTHWEST,
    TILE_WEST,
    TILE_NORTHWEST,
    TILE_NORTHEAST
};

struct Tile {
    struct Coords origin;
    enum TileDirection direction;
};

void Tile_coords(const struct Tile* tile, struct Coords coords[]);

#endif
