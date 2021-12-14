#include "sink_pathing.h"
#include "game.h"
#include "meth.h"

uint8_t sink_dist[MAP_WIDTH][MAP_HEIGHT];
int ptile_x, ptile_y;

// basically djikstra
void rec_dst(int x, int y, uint8_t dst) {
    if (map[y][x])
        return;

    if (sink_dist[y][x] <= dst)
        return;

    sink_dist[y][x] = dst;
    const uint8_t nd = (dst + 4) & 0b11111100;
    rec_dst(x + 1, y + 0, nd | 0);
    rec_dst(x - 1, y + 0, nd | 1);
    rec_dst(x + 0, y + 1, nd | 2);
    rec_dst(x + 0, y - 1, nd | 3);
}

// prebakes directions and distances to a tile
void create_sink(float x, float y) {
    const int tile_x = (int) x;
    const int tile_y = (int) y;
    if (tile_x == ptile_x && tile_y == ptile_y)
        return;
    
    uint8_t a, b;
    for(a = 0; a < MAP_WIDTH; a++)
        for(b = 0; b < MAP_HEIGHT; b++)
            sink_dist[b][a] = 255;

    rec_dst(tile_x, tile_y, 0);

    ptile_x = tile_x;
    ptile_y = tile_y;
}

// moves entity towards target
void follow_sink(entity_t* e, float mvspeed) {
    const int tile_x = (int) e->x;
    const int tile_y = (int) e->y;

    float target_x = ((float)tile_x) + 0.5f;
    float target_y = ((float)tile_y) + 0.5f;

    if(sink_dist[tile_y][tile_x] >> 2)
    switch (sink_dist[tile_y][tile_x] & 0x3)
    {
        case 0: target_x -= 1.f; break;
        case 1: target_x += 1.f; break;
        case 2: target_y -= 1.f; break;
        case 3: target_y += 1.f; break;
    }

    target_x -= e->x;
    target_y -= e->y;
    NORMALIZE(target_x, target_y, mvspeed);

    e->x += target_x;
    e->y += target_y;
}