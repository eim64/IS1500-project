#include <stddef.h>
#include <pic32mx.h>
#include <stdint.h>
#include <stdbool.h>
#include "display.h"

#define DEADZONE 32


float pos_x = 2.5f;
float pos_y = 1.5f;

float dir_x = 0.66f;
float dir_y = 0.f;

float plane_x = 0.f;
float plane_y = 0.5f;

const float dirlen = 0.66f;
const float planelen = 0.5f;

uint32_t ammo = 4;
uint32_t p_switch;
uint32_t gun_fire;
uint32_t fire_offset = 0;

uint32_t is_hit = 0;

int ptile_x;
int ptile_y;

int frame = 0;


#define DRAW_WIDTH 64

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
uint8_t map[MAP_WIDTH][MAP_HEIGHT] = {
  {1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,1,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,1,1,1,0,0,1,1},
  {1,0,0,1,1,1,0,0,0,1},
  {1,0,0,1,0,0,0,0,0,1},
  {1,0,0,1,1,0,0,0,0,1},
  {1,1,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1}
};

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

uint8_t g_dist[MAP_WIDTH][MAP_HEIGHT];

void rec_dst(int x, int y, uint8_t dst) {
    if (map[y][x])
        return;

    if (g_dist[y][x] <= dst)
        return;

    g_dist[y][x] = dst;
    const uint8_t nd = (dst + 4) & 0b11111100;
    rec_dst(x + 1, y + 0, nd | 0);
    rec_dst(x - 1, y + 0, nd | 1);
    rec_dst(x + 0, y + 1, nd | 2);
    rec_dst(x + 0, y - 1, nd | 3);
}



typedef struct {
    uint32_t* spr;
    float x;
    float y;
    float c_dist;
} entity_t;

#define ENTITY_COUNT 4
entity_t entities[ENTITY_COUNT] = {
    {aspr, 1.5f, 2.5f, 0.0},
    {aspr, 2.5f, 7.5f, 0.0},
    {aspr, 4.5f, 1.5f, 0.0},
    {enspr, 1.5f, 3.5f, 0.0}
};


entity_t* enemy = &entities[ENTITY_COUNT - 1];
uint32_t e_index[ENTITY_COUNT] = {0, 1, 2, 3};

// Bubble sort for now
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

#define AMMO_QUEUE_LEN 7
const float ax_q[AMMO_QUEUE_LEN] = {1.5f, 2.5f, 4.5f, 8.5f, 8.5f, 5.5f, 2.5f};
const float ay_q[AMMO_QUEUE_LEN] = {2.5f, 7.5f, 1.5f, 1.5f, 8.5f, 5.5f, 8.5f};
int32_t ammo_q_index = 3;

void pick_ammo(int32_t index) {
    ammo += 2;
    entity_t* p = entities + index;
    p->x = ax_q[ammo_q_index];
    p->y = ay_q[ammo_q_index];

    ammo_q_index = (ammo_q_index + 1) % AMMO_QUEUE_LEN;
}


float zbuffer[DRAW_WIDTH];
bool can_hit(const float t_x, const float t_y, const float col_size){
    const float rel_x = t_x - pos_x;
    const float rel_y = t_y - pos_y;

    const float inv_det = 1.f / (plane_x * dir_y - dir_x * plane_y);
    const float plane_coord = inv_det * (dir_y * rel_x - dir_x * rel_y);
    const float dir_coord = inv_det * (-plane_y * rel_x + plane_x * rel_y);
    
    if (dir_coord < 0.f)
        return false;

    int32_t screen_x = (int32_t)(32.f * (plane_coord / dir_coord + 1));
    const float perp_dist = dir_coord * dirlen;
    
    int32_t size = (int32_t)(SCREEN_HEIGHT / perp_dist);
    return (screen_x + size >= DRAW_WIDTH / 2) && (screen_x - size <= DRAW_WIDTH / 2) && (zbuffer[DRAW_WIDTH / 2] > perp_dist);
}

// draws a 16x16 sprite
void draw_sprite(uint32_t *sprite, float x, float y) {
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

float taylor_sin(float val){
    return val - val * val * val / 120.f;
}

float taylor_cos(float val) {
    return 1.f - val * val / 2.0f + val*val*val*val/24.0;
}

uint32_t getbtns(){
    return ((PORTD >> 4) & 0xE) | ((PORTF & 0x2) >> 1);
}

uint32_t getswitches(){
    return (PORTD >> 8) & 0xF;
}

// Quake 3 lets gooooooooooo
float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

void push(float* target_x, float* target_y, float source_x, float source_y, float distance) {
    const float dist_x = source_x - *target_x;
    const float dist_y = source_y - *target_y;

    const float mag = dist_x * dist_x + dist_y*dist_y;
    if (mag < distance * distance)
    {
        const float scalar = distance * Q_rsqrt(mag);
        *target_x = source_x - dist_x * scalar;
        *target_y = source_y - dist_y * scalar;
    }
}

unsigned int read_potentiometer(){
    // Start sample
    AD1CON1SET = 0x2;
    // Wait for DONE
    while(!(AD1CON1 & 0x1));
    // Return the value 
    return ADC1BUF0;
}


void game_logic() {
    display_clear();

    // controller logic
    {
        float rotation_speed = 0.00f;
        
        uint32_t button_state = getbtns();
        uint32_t switch_state = getswitches();

        if(ammo && !gun_fire && switch_state && !p_switch){
            gun_fire = 10;
            ammo--;
            
            if(!fire_offset) fire_offset = 4;
            else             fire_offset = 0;

            if(can_hit(enemy->x, enemy->y, 1.f)){
                enemy->spr = aspr;
            }
        }

        p_switch = switch_state;

    
        unsigned int potent_value = read_potentiometer(); // 0 - 1024
        
        if (potent_value < 512 + DEADZONE && potent_value > 512 - DEADZONE) 
            potent_value = 512;
        
        rotation_speed = (((float)potent_value) - 512.f) / (512.f *  100.f);

        const float mvscal = 0.01f;

        if(button_state & 0x1) {
            pos_x -= dir_y * mvscal;
            pos_y += dir_x * mvscal;
        }

        if(button_state & 0x8) {
            pos_x += dir_y * mvscal;
            pos_y -= dir_x * mvscal;
        }

        if(button_state & 0x2) {
            pos_x += dir_x * mvscal;
            pos_y += dir_y * mvscal;
        }

        if(button_state & 0x4) {
            pos_x -= dir_x * mvscal;
            pos_y -= dir_y * mvscal;
        }

        const float r_cos = taylor_cos(rotation_speed);
        const float r_sin = taylor_sin(rotation_speed);

        // apply rotation matrix
        dir_x = r_cos * dir_x - r_sin * dir_y;
        dir_y = r_sin * dir_x + r_cos * dir_y;
        
        plane_x = r_cos * plane_x - r_sin * plane_y;
        plane_y = r_sin * plane_x + r_cos * plane_y;
        
        // correct lengths from improper transform
        {
            const float dir_mag = dir_x*dir_x + dir_y*dir_y;
            const float scalar = Q_rsqrt(dir_mag) * dirlen;

            dir_x *= scalar;
            dir_y *= scalar;
        }

        
        // gram-schmidt to perpendicularity
        {
            const float scalar = (plane_x*dir_x + plane_y*dir_y)/(dirlen * dirlen);
            
            plane_x -= dir_x * scalar;
            plane_y -= dir_y * scalar;
        }

        {
            const float plane_mag = plane_x*plane_x + plane_y*plane_y;
            const float scalar = Q_rsqrt(plane_mag) * planelen;

            plane_x *= scalar;
            plane_y *= scalar;
        }
        

        // wall collision
        const int tile_x = (int) pos_x;
        const int tile_y = (int) pos_y;

        const float player_radius = 0.2f;

        if (map[tile_y + 1][tile_x] && (tile_y + 1.0 - pos_y) < player_radius)
            pos_y = tile_y + 1.0 - player_radius;

        if (map[tile_y - 1][tile_x] && (pos_y - tile_y) < player_radius)
            pos_y = tile_y + player_radius;

        if (map[tile_y][tile_x + 1] && (tile_x + 1.0 - pos_x) < player_radius)
            pos_x = tile_x + 1.0 - player_radius;
        
        if (map[tile_y][tile_x - 1] && (pos_x - tile_x) < player_radius)
            pos_x = tile_x + player_radius;

        if (map[tile_y + 1][tile_x + 1]) 
            push(&pos_x, &pos_y, tile_x + 1.0f, tile_y + 1.0f, player_radius);

        if (map[tile_y + 1][tile_x - 1]) 
            push(&pos_x, &pos_y, tile_x + 0.0f, tile_y + 1.0f, player_radius);

        if (map[tile_y - 1][tile_x + 1]) 
            push(&pos_x, &pos_y, tile_x + 1.0f, tile_y + 0.0f, player_radius);

        if (map[tile_y - 1][tile_x - 1]) 
            push(&pos_x, &pos_y, tile_x + 0.0f, tile_y + 0.0f, player_radius);
    }

    // draw 2D rep
    {
        display_setpx((pos_x / MAP_WIDTH) * 32 + 64,(pos_y / MAP_HEIGHT) * 32);
    }


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

    // PREPARE DISTANCES
    {
        const int tile_x = (int) pos_x;
        const int tile_y = (int) pos_y;
        if (tile_x != ptile_x || tile_y != ptile_y){
            uint8_t a, b;
            for(a = 0; a < MAP_WIDTH; a++)
                for(b = 0; b < MAP_HEIGHT; b++)
                    g_dist[b][a] = 255;

            rec_dst(tile_x, tile_y, 0);
        }

        ptile_x = tile_x;
        ptile_y = tile_y;
    }

    
    // SPRITE RENDERING + AMMO PICKUP
    {
        uint32_t i;
        for(i = 0; i < ENTITY_COUNT; i++) {
            entity_t* e = entities + i;
            const float dist_x = e->x - pos_x;
            const float dist_y = e->y - pos_y;
            const float dist_s = dist_x * dist_x + dist_y * dist_y;

            // AMMO COLLISION
            if(e->spr == aspr && dist_s < 0.4f) {
                pick_ammo(i);
                --i;

                continue;
            }

            // ENEMY AI
            if(e->spr == enspr) {
                const int tile_x = (int) e->x;
                const int tile_y = (int) e->y;

                float target_x = ((float)tile_x) + 0.5f;
                float target_y = ((float)tile_y) + 0.5f;

                const float mvspeed = 0.005f;
                if(g_dist[tile_y][tile_x] >> 2)
                switch (g_dist[tile_y][tile_x] & 0x3)
                {
                    case 0: target_x -= 1.f; break;
                    case 1: target_x += 1.f; break;
                    case 2: target_y -= 1.f; break;
                    case 3: target_y += 1.f; break;
                }

                target_x -= e->x;
                target_y -= e->y;

                const float scalar = Q_rsqrt(target_x * target_x + target_y*target_y) * mvspeed;
                target_x *= scalar;
                target_y *= scalar;

                e->x += target_x;
                e->y += target_y;

                // PLAYER GET HIT
                if(!is_hit && dist_s < (0.6 * 0.6)) {
                    is_hit = 20;
                }
            }

            e->c_dist = dist_s;
        }

        sort_entities();

        for(i = 0; i < ENTITY_COUNT; i++){
            entity_t* e = entities + e_index[i];
            
            draw_sprite(e->spr, e->x, e->y);
        }
            

    }

    if(ammo > 8) ammo = 8;
    PORTE = ((1 << ammo) - 1);

    if(gun_fire){
        gun_fire--;

        display_setpx(30 + fire_offset, 21);
        display_setpx(29 + fire_offset, 20);
        display_setpx(29 + fire_offset, 20);

        display_setpx(31 + fire_offset, 19);
        display_setpx(31 + fire_offset, 18);

        display_setpx(33 + fire_offset, 21);
        display_setpx(33 + fire_offset, 20);
        display_setpx(34 + fire_offset, 20);

        PORTE = 0xFF;
    }


    if(is_hit) {
        is_hit -= 1;
        uint32_t* scr_buf = (uint32_t)screen_buffer;

        int i;
        for(i = 0; i < DRAW_WIDTH/4; i++){
            scr_buf[i +  0] ^= (frame * 1112516) ^ 0xABCABCAB;
            scr_buf[i + 32] ^= (frame * 1112516) ^ 0xABCABCAB;
            scr_buf[i + 64] ^= (frame * 1112516) ^ 0xABCABCAB;
            scr_buf[i + 96] ^= (frame * 1112516) ^ 0xABCABCAB;
        }
    }




    screen_buffer[SCREEN_WIDTH * 3 + 30] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 31] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 32] = 0x00;
    screen_buffer[SCREEN_WIDTH * 3 + 33] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 34] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 35] = 0x00;
    screen_buffer[SCREEN_WIDTH * 3 + 36] = 0xFF;

    display_setpx(31, 23);
    display_setpx(32, 23);

    display_setpx(34, 23);
    display_setpx(35, 23);

    display_update();
    ++frame;
}
