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

// Stolen font
const uint8_t const font[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 94, 0, 0, 0, 0,
	0, 0, 4, 3, 4, 3, 0, 0,
	0, 36, 126, 36, 36, 126, 36, 0,
	0, 36, 74, 255, 82, 36, 0, 0,
	0, 70, 38, 16, 8, 100, 98, 0,
	0, 52, 74, 74, 52, 32, 80, 0,
	0, 0, 0, 4, 3, 0, 0, 0,
	0, 0, 0, 126, 129, 0, 0, 0,
	0, 0, 0, 129, 126, 0, 0, 0,
	0, 42, 28, 62, 28, 42, 0, 0,
	0, 8, 8, 62, 8, 8, 0, 0,
	0, 0, 0, 128, 96, 0, 0, 0,
	0, 8, 8, 8, 8, 8, 0, 0,
	0, 0, 0, 0, 96, 0, 0, 0,
	0, 64, 32, 16, 8, 4, 2, 0,
	0, 62, 65, 73, 65, 62, 0, 0,
	0, 0, 66, 127, 64, 0, 0, 0,
	0, 0, 98, 81, 73, 70, 0, 0,
	0, 0, 34, 73, 73, 54, 0, 0,
	0, 0, 14, 8, 127, 8, 0, 0,
	0, 0, 35, 69, 69, 57, 0, 0,
	0, 0, 62, 73, 73, 50, 0, 0,
	0, 0, 1, 97, 25, 7, 0, 0,
	0, 0, 54, 73, 73, 54, 0, 0,
	0, 0, 6, 9, 9, 126, 0, 0,
	0, 0, 0, 102, 0, 0, 0, 0,
	0, 0, 128, 102, 0, 0, 0, 0,
	0, 0, 8, 20, 34, 65, 0, 0,
	0, 0, 20, 20, 20, 20, 0, 0,
	0, 0, 65, 34, 20, 8, 0, 0,
	0, 2, 1, 81, 9, 6, 0, 0,
	0, 28, 34, 89, 89, 82, 12, 0,
	0, 0, 126, 9, 9, 126, 0, 0,
	0, 0, 127, 73, 73, 54, 0, 0,
	0, 0, 62, 65, 65, 34, 0, 0,
	0, 0, 127, 65, 65, 62, 0, 0,
	0, 0, 127, 73, 73, 65, 0, 0,
	0, 0, 127, 9, 9, 1, 0, 0,
	0, 0, 62, 65, 81, 50, 0, 0,
	0, 0, 127, 8, 8, 127, 0, 0,
	0, 0, 65, 127, 65, 0, 0, 0,
	0, 0, 32, 64, 64, 63, 0, 0,
	0, 0, 127, 8, 20, 99, 0, 0,
	0, 0, 127, 64, 64, 64, 0, 0,
	0, 127, 2, 4, 2, 127, 0, 0,
	0, 127, 6, 8, 48, 127, 0, 0,
	0, 0, 62, 65, 65, 62, 0, 0,
	0, 0, 127, 9, 9, 6, 0, 0,
	0, 0, 62, 65, 97, 126, 64, 0,
	0, 0, 127, 9, 9, 118, 0, 0,
	0, 0, 38, 73, 73, 50, 0, 0,
	0, 1, 1, 127, 1, 1, 0, 0,
	0, 0, 63, 64, 64, 63, 0, 0,
	0, 31, 32, 64, 32, 31, 0, 0,
	0, 63, 64, 48, 64, 63, 0, 0,
	0, 0, 119, 8, 8, 119, 0, 0,
	0, 3, 4, 120, 4, 3, 0, 0,
	0, 0, 113, 73, 73, 71, 0, 0,
	0, 0, 127, 65, 65, 0, 0, 0,
	0, 2, 4, 8, 16, 32, 64, 0,
	0, 0, 0, 65, 65, 127, 0, 0,
	0, 4, 2, 1, 2, 4, 0, 0,
	0, 64, 64, 64, 64, 64, 64, 0,
	0, 0, 1, 2, 4, 0, 0, 0,
	0, 0, 48, 72, 40, 120, 0, 0,
	0, 0, 127, 72, 72, 48, 0, 0,
	0, 0, 48, 72, 72, 0, 0, 0,
	0, 0, 48, 72, 72, 127, 0, 0,
	0, 0, 48, 88, 88, 16, 0, 0,
	0, 0, 126, 9, 1, 2, 0, 0,
	0, 0, 80, 152, 152, 112, 0, 0,
	0, 0, 127, 8, 8, 112, 0, 0,
	0, 0, 0, 122, 0, 0, 0, 0,
	0, 0, 64, 128, 128, 122, 0, 0,
	0, 0, 127, 16, 40, 72, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0,
	0, 120, 8, 16, 8, 112, 0, 0,
	0, 0, 120, 8, 8, 112, 0, 0,
	0, 0, 48, 72, 72, 48, 0, 0,
	0, 0, 248, 40, 40, 16, 0, 0,
	0, 0, 16, 40, 40, 248, 0, 0,
	0, 0, 112, 8, 8, 16, 0, 0,
	0, 0, 72, 84, 84, 36, 0, 0,
	0, 0, 8, 60, 72, 32, 0, 0,
	0, 0, 56, 64, 32, 120, 0, 0,
	0, 0, 56, 64, 56, 0, 0, 0,
	0, 56, 64, 32, 64, 56, 0, 0,
	0, 0, 72, 48, 48, 72, 0, 0,
	0, 0, 24, 160, 160, 120, 0, 0,
	0, 0, 100, 84, 84, 76, 0, 0,
	0, 0, 8, 28, 34, 65, 0, 0,
	0, 0, 0, 126, 0, 0, 0, 0,
	0, 0, 65, 34, 28, 8, 0, 0,
	0, 0, 4, 2, 4, 2, 0, 0,
	0, 120, 68, 66, 68, 120, 0, 0,
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
    int screen_x = (int)(32.f * (plane_coord / dir_coord + 1));
    
    const float perp_dist = (dir_coord + 1.f) * dirlen;
    int size = (int)(SCREEN_HEIGHT / perp_dist);

    const int x0 = screen_x - size / 2;
    int cx = x0;
    if (cx < 0) cx = 0;

    int x_end = cx + size;
    if (x_end >= DRAW_WIDTH) x_end = DRAW_WIDTH - 1;

    const int y0 = SCREEN_HEIGHT / 2 - size / 2; 
    int y_start = y0;
    if (y_start < 0) y_start = 0;

    int y_end = y_start + size;
    if (y_end > SCREEN_HEIGHT) y_end = SCREEN_HEIGHT - 1;

    int cy;

    for (; cx <= x_end; cx++) {
        if (perp_dist > zbuffer[cx])
            continue;

        // avoid floats
        int u = (((cx - x0) * 1024 * 16) / size) / 1024;
        if(u >= 16) continue;

        uint32_t col = sprite[u];
        if(!col) continue; //empty rows

        for (cy = y_start; cy < y_end; ++cy) {
            int v = (((cy - y0) * 1024 * 16) / size) / 1024;
            uint32_t px = (col >> (v * 2)) & 0b11;
            if (!px) continue;

            display_px(cx, cy, px & 1);
        }
    }
}

void raycast_map(){
    int x, pt_x, pt_y;
    int p_side;
    int p_height;
    for (x = 0; x < DRAW_WIDTH; x++) {
        float scan_x = 2 * x / (float)DRAW_WIDTH - 1;
        float ray_x = dir_x + plane_x * scan_x;
        float ray_y = dir_y + plane_y * scan_x;

        // ray rasterizer
        int tile_x = (int) pos_x;
        int tile_y = (int) pos_y;

        float step_x = (ray_x == 0.0) ? 1000.f : 1 / ray_x;
        float step_y = (ray_y == 0.0) ? 1000.f : 1 / ray_y;

        if(step_x < 0) step_x = -step_x;
        if(step_y < 0) step_y = -step_y;

        int tiledir_x;
        int tiledir_y;
        float griddist_x;
        float griddist_y;

        // compute distances to reach next x/y grid line
        if(ray_x < 0){
            tiledir_x = -1;
            griddist_x = (pos_x - tile_x) * step_x;
        }else{
            tiledir_x = 1;
            griddist_x = (tile_x + 1.f - pos_x) * step_x;
        }

        if(ray_y < 0) {
            tiledir_y = -1;
            griddist_y = (pos_y - tile_y) * step_y;
        }else{
            tiledir_y = 1;
            griddist_y = (tile_y + 1.f - pos_y) * step_y;
        }

        int side;
        while(!map[tile_y][tile_x]) {
            side = griddist_x < griddist_y;

            if(side) {
                griddist_x += step_x;
                tile_x += tiledir_x;
            } else {
                griddist_y += step_y;
                tile_y += tiledir_y;
            }
        }

        float perp_dist; 
        // smart cos: https://lodev.org/cgtutor/raycasting.html 
        if(side) perp_dist = griddist_x - step_x;
        else     perp_dist = griddist_y - step_y;

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

        // corner
        if(x != 0 && (p_side != side || (side && pt_x - tile_x != 0) || (!side && pt_y - tile_y != 0)))
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
        pt_x = tile_x;
        pt_y = tile_y;
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


void draw_char(int x, int row, char c) {
    int i;
    for(i = 0; i < 8; i++)
        screen_buffer[x*8 + i + row*SCREEN_WIDTH] = font[c*8 + i];
}

void draw_text(int x, int row, char* str) {
    while(*str) draw_char(x++, row, *(str++));
}

char itc(int x) {
    return x + '0';
}

void draw_uint(int x, int row, int num) {
    int i, n;
    for(i = 4; i >= 0; i--){
        n = num % 10;
        draw_char(x + i, row, itc(n));
        
        num /= 10;
    }
}