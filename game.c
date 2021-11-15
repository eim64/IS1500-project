#include <stddef.h>
#include <pic32mx.h>
#include <stdint.h>
#include <stdbool.h>
#include "display.h"

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

float zbuffer[DRAW_WIDTH];

bool can_hit(const float t_x, const float t_y, const float col_size){
    const float rel_x = t_x - pos_x;
    const float rel_y = t_y - pos_y;

    const float hyp_s = rel_x * rel_x - rel_y * rel_y;
    if (hyp_s < zbuffer[SCREEN_WIDTH / 2] * zbuffer[SCREEN_WIDTH / 2]) {
        const float cross = rel_x * t_y - rel_y * t_x;
        const float dist_s = (cross * cross) / hyp_s;

        const float spread_s = col_size * col_size;
        return dist_s < spread_s;
    }

    return false;
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
        }

        p_switch = switch_state;

        if(button_state & 0x1) rotation_speed =  0.01f;
        if(button_state & 0x4) rotation_speed -= 0.01f;

        if(button_state & 0x2) {
            pos_x += dir_x * 0.01f;
            pos_y += dir_y * 0.01f;
        }

        if(button_state & 0x8) {
            pos_x -= dir_x * 0.01f;
            pos_y -= dir_y * 0.01f;
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
        int tile_x = (int)pos_x;
        int tile_y = (int)pos_y;

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

    ammo &= 0xF;
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

    screen_buffer[SCREEN_WIDTH * 3 + 30] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 31] = 0xFF;

    screen_buffer[SCREEN_WIDTH * 3 + 33] = 0xFF;
    screen_buffer[SCREEN_WIDTH * 3 + 34] = 0xFF;
    
    screen_buffer[SCREEN_WIDTH * 3 + 36] = 0xFF;

    display_setpx(31, 23);
    display_setpx(32, 23);

    display_setpx(34, 23);
    display_setpx(35, 23);

    display_update();
}
