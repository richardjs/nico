#include "tile.h"

const char* TILE_DIRECTION_CODES[] = {
    "ee", "se", "sw", "ww", "nw", "ne"
};

void Tile_coords(const struct Tile* tile, struct Coords coords[])
{
    int i = 0;
    coords[i++] = tile->origin;

    struct Coords walk = tile->origin;
    switch (tile->direction) {
    case TILE_EAST:
        Coords_move(&walk, NORTHEAST);
        coords[i++] = walk;
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        Coords_move(&walk, NORTHEAST);
        coords[i++] = walk;
        break;
    case TILE_SOUTHEAST:
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        Coords_move(&walk, NORTHEAST);
        coords[i++] = walk;
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        break;
    case TILE_SOUTHWEST:
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        Coords_move(&walk, NORTHWEST);
        coords[i++] = walk;
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        break;
    case TILE_WEST:
        Coords_move(&walk, NORTHWEST);
        coords[i++] = walk;
        Coords_move(&walk, SOUTH);
        coords[i++] = walk;
        Coords_move(&walk, NORTHWEST);
        coords[i++] = walk;
        break;
    case TILE_NORTHWEST:
        Coords_move(&walk, NORTH);
        coords[i++] = walk;
        Coords_move(&walk, SOUTHWEST);
        coords[i++] = walk;
        Coords_move(&walk, NORTH);
        coords[i++] = walk;
        break;
    case TILE_NORTHEAST:
        Coords_move(&walk, NORTH);
        coords[i++] = walk;
        Coords_move(&walk, SOUTHEAST);
        coords[i++] = walk;
        Coords_move(&walk, NORTH);
        coords[i++] = walk;
        break;
    }
}

void Tile_permutations(const struct Tile* tile, struct Coords permutations[][TILE_SIZE])
{
    struct Coords coords[TILE_SIZE];
    Tile_coords(tile, coords);

    // Heap's algorithm
    // https://en.wikipedia.org/wiki/Heap's_algorithm

    int c[TILE_SIZE] = { 0 };

    int p = 0;
    for (int t = 0; t < TILE_SIZE; t++) {
        permutations[p][t] = coords[t];
    }
    p++;

    int i = 1;
    while (i < TILE_SIZE) {
        if (c[i] < i) {
            struct Coords tmp;
            if (i % 2 == 0) {
                tmp = coords[0];
                coords[0] = coords[1];
                coords[1] = tmp;
            } else {
                tmp = coords[c[i]];
                coords[c[i]] = coords[i];
                coords[i] = tmp;
            }
            for (int t = 0; t < TILE_SIZE; t++) {
                permutations[p][t] = coords[t];
            }
            p++;
            c[i] += 1;
            i = 1;
        } else {
            c[i] = 0;
            i += 1;
        }
    }
}
