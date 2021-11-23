#include "graphics.h"

#include "display.h"
#include "game.h"

/*
    each sprite pixel consists of 2 bits
    bit 1 is the value to be drawn
    bit 2 is weather the bit is to be drawn

    each value in the row corresponds to a row, bottom -> top
    rotate 90 counterclockwise for correct representation
*/
uint32_t enspr[16] ={
        0b00000000000000001010100000000000, // TL
        0b00000000000000100101011000000000,
        0b00000000000010011010010110100000,
        0b00000000001001010110100101011000,
        0b00000000100101011001100101011000,
        0b00000000100101101001010101011000,
        0b00000010010110011001101001011000,
        0b00000010011001100101101001011000,
        0b00000010011010100101011001011000,
        0b00001001011001100110011001011000,
        0b00001001011010101001010101011000,
        0b00001001011001101001101001011000,
        0b00001001011010100110101001011000,
        0b00001001011001100101011001011000,
        0b00001001010110101010010110100000,
        0b00000010101000000000101000000000 // TR
    };

uint32_t aspr[16] = {
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000,
        0b11111111111111110000000000000000,
        0b11101010101010110000000000000000,
        0b11101111111010110000000000000000,
        0b11101011101110110000000000000000,
        0b11101011101110110000000000000000,
        0b11101111111010110000000000000000,
        0b11101010101010110000000000000000,
        0b11111111111111110000000000000000,
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000,
        0b00000000000000000000000000000000
};

// sort entities according to distance to use painters
void sort_entities() {
    uint32_t i1, i2, swap;

    for (i1 = 0; i1 < ENTITY_COUNT - 1; i1++)
        for(i2 = 0; i2 < ENTITY_COUNT - 1 - i1; i2++){
            if(entities[e_index[i2]].c_dist < entities[e_index[i2 + 1]].c_dist){
                swap = e_index[i2];
                e_index[i2] = e_index[i2 + 1];
                e_index[i2 + 1] = swap;
            }
        }
    
}

// draws a 16x16 sprite
void draw_sprite(uint32_t *sprite, float x, float y) {
    if(!sprite) return;

    x -= pos_x;
    y -= pos_y;

    // translate to plane/dir coordinates using inverse transform
    const float inv_det = 1.f / (plane_x * dir_y - dir_x * plane_y);
    const float plane_coord = inv_det * (dir_y * x - dir_x * y);
    const float dir_coord = inv_det * (-plane_y * x + plane_x * y);

    // too close to object
    if (dir_coord < 0)
        return;

    // use similar triangle to map values to screen, then center
    // screen pixels are already in -0.5 to 0.5 in plane dimension
    // screen_x = plane_coord / dir_coord * width / 2 + width / 2
    int32_t screen_x = (int32_t)(32.f * (plane_coord / dir_coord + 1));
    
    const float perp_dist = dir_coord * dirlen;
    int32_t size = (int32_t)(SCREEN_HEIGHT / perp_dist);

    const int32_t x0 = screen_x - size / 2;
    int32_t cx = x0;
    if (cx < 0) cx = 0;

    int32_t x_end = cx + size;
    if (x_end >= DRAW_WIDTH) x_end = DRAW_WIDTH - 1;

    const int32_t y0 = SCREEN_HEIGHT / 2 - size / 2; 
    int32_t y_start = y0;
    if (y_start < 0) y_start = 0;

    int32_t y_end = y_start + size;
    if (y_end > SCREEN_HEIGHT) y_end = SCREEN_HEIGHT - 1;

    int32_t cy;

    for (; cx <= x_end; cx++) {
        if (perp_dist > zbuffer[cx])
            continue;

        // avoid floats
        int32_t u = (((cx - x0) * 1024 * 16) / size) / 1024;
        if(u >= 16) continue;

        uint32_t col = sprite[u];
        if(!col) continue; //empty rows

        for (cy = y_start; cy < y_end; ++cy) {
            int32_t v = (((cy - y0) * 1024 * 16) / size) / 1024;
            uint32_t px = (col >> (v * 2)) & 0b11;
            if (!px) continue;

            display_px(cx, cy, px & 1);
        }
    }
}

void raycast_map(){
    int x;
    int p_side;
    int p_height;
    float p_perpdist;
    for (x = 0; x < DRAW_WIDTH; x++) {
        float scan_x = 2 * x / (float)DRAW_WIDTH - 1;
        float ray_x = dir_x + plane_x * scan_x;
        float ray_y = dir_y + plane_y * scan_x;

        // line rasterizer, avoids float conversion in loop
        int tile_x = (int) pos_x;
        int tile_y = (int) pos_y;

        float step_x = (ray_x == 0.0) ? 1e20f : 1 / ray_x;
        float step_y = (ray_y == 0.0) ? 1e20f : 1 / ray_y;

        if(step_x < 0) step_x = -step_x;
        if(step_y < 0) step_y = -step_y;

        int tiledir_x;
        int tiledir_y;
        float edgedist_x;
        float edgedist_y;

        // compute distances to reach next x/y grid line
        // this is faster than using 
        // edgedist_x = fabsf(pos_x - tile_x) + ((ray_x < 0) ? step_x : 0.f);
        // tiledir_x = ((ray_x < 0) ? -1 : 1);

        if(ray_x < 0){
            tiledir_x = -1;
            edgedist_x = (pos_x - tile_x) * step_x;
        }else{
            tiledir_x = 1;
            edgedist_x = (tile_x + 1.f - pos_x) * step_x;
        }

        if(ray_y < 0) {
            tiledir_y = -1;
            edgedist_y = (pos_y - tile_y) * step_y;
        }else{
            tiledir_y = 1;
            edgedist_y = (tile_y + 1.f - pos_y) * step_y;
        }

        // use side to remove last jump and draw corners
        int side;

        while(!map[tile_y][tile_x]) {
            if(edgedist_x < edgedist_y) {
                edgedist_x += step_x;
                tile_x += tiledir_x;
                
                side = 0;
            } else {
                edgedist_y += step_y;
                tile_y += tiledir_y;

                side = 1;
            }
        }

        
        float perp_dist; 
        if(side == 0) perp_dist = edgedist_x - step_x;
        else          perp_dist = edgedist_y - step_y;

        if (perp_dist < 0){
            display_setpx(x, 0);
            display_setpx(x, SCREEN_HEIGHT - 1);
            
            continue;
        }

        int h_offset = ((int)(SCREEN_HEIGHT / perp_dist)) / 2;
        const int center = SCREEN_HEIGHT / 2;

        if (center - h_offset <= 0)
            h_offset = center - 1;

        display_setpx(x, center + h_offset);
        display_setpx(x, center - h_offset);

        float diff = perp_dist - p_perpdist;
        if (diff < 0) diff = -diff;

        // corner
        if(x != 0 && (p_side != side || diff > 0.9f))
        {
            int y;
            if(p_height > h_offset) h_offset = p_height + 1;

            for (y = 0; y < h_offset; y++){
                display_setpx(x, center - y);
                display_setpx(x, center + y);
            }
        }

        zbuffer[x] = perp_dist;

        p_side = side;
        p_height = h_offset;
        p_perpdist = perp_dist;
    }
}


void show_noise(){
    uint32_t* scr_buf = (uint32_t*)screen_buffer;

    int i;
    for(i = 0; i < DRAW_WIDTH/4; i++){
        scr_buf[i +  0] ^= (frame * 1112516) ^ 0xABCABCAB;
        scr_buf[i + 32] ^= (frame * 1112516) ^ 0xABCABCAB;
        scr_buf[i + 64] ^= (frame * 1112516) ^ 0xABCABCAB;
        scr_buf[i + 96] ^= (frame * 1112516) ^ 0xABCABCAB;
    }
}

void draw_gun(int offset){
    screen_buffer[SCREEN_WIDTH * 3 + 30 + offset] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 31 + offset] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 32 + offset] = 0x00;
    screen_buffer[SCREEN_WIDTH * 3 + 33 + offset] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 34 + offset] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 35 + offset] = 0x00;
    screen_buffer[SCREEN_WIDTH * 3 + 36 + offset] = 0xFF;

    display_setpx(31 + offset, 23);
    display_setpx(32 + offset, 23);

    display_setpx(34 + offset, 23);
    display_setpx(35 + offset, 23);
}

void draw_fire(int offset) {
    display_setpx(30 + offset, 21);
    display_setpx(29 + offset, 20);
    display_setpx(29 + offset, 20);

    display_setpx(31 + offset, 19);
    display_setpx(31 + offset, 18);

    display_setpx(33 + offset, 21);
    display_setpx(33 + offset, 20);
    display_setpx(34 + offset, 20);
}