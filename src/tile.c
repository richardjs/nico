#include "tile.h"

const char* TILE_DIRECTION_CODES[] = {
    "nn", "ne", "se", "ss", "sw", "nw"
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
        Coords_move(&walk, SOUTHWEST);
        coords[i++] = walk;
        Coords_move(&walk, NORTH);
        coords[i++] = walk;
        break;
    }
}
