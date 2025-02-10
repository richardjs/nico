#include "tile.h"
#include "coords.h"

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

    // TODO This could be a proper permutation algorithm, instead of
    // this... rustic solution. On the other hand, unlike some
    // permutation algorithms, it's comprehensible at first glance.
    int i = 0;
    for (int c1 = 0; c1 < TILE_SIZE; c1++) {
        for (int c2 = 0; c2 < TILE_SIZE; c2++) {
            if (c1 == c2)
                continue;
            for (int c3 = 0; c3 < TILE_SIZE; c3++) {
                if ((c3 == c1) || (c3 == c2))
                    continue;
                for (int c4 = 0; c4 < TILE_SIZE; c4++) {
                    if ((c4 == c1) || (c4 == c2) || (c4 == c3))
                        continue;

                    permutations[i][0] = coords[c1];
                    permutations[i][1] = coords[c2];
                    permutations[i][2] = coords[c3];
                    permutations[i++][3] = coords[c4];
                }
            }
        }
    }
}
