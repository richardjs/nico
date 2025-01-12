#ifndef TILE_H
#define TILE_H

#include "coords.h"
#include "state.h"

enum Rotation {
    H = 0,
    L,
    R
}

enum TileDirection {
    TILE_EAST = 0,
    TILE_SOUTHEAST,
    TILE_SOUTHWEST,
    TILE_WEST,
    TILE_NORTHWEST,
    TILE_NORTHEAST
}

struct Tile {
    struct Coords origin;
    enum Rotation rotation;
};

#endif
