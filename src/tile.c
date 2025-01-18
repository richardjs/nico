#include "tile.h"

void tile_coords(const struct Coords* start_coords, enum TileDirection direction, struct Coords coords[])
{
    int i = 0;
    coords[i++] = *start_coords;

    struct Coords walk = *start_coords;

    switch (direction) {
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
