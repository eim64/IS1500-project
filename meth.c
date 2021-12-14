#include "meth.h"
#include "game.h"
#include "display.h"

float taylor_sin(float val){
    return val - val * val * val / 120.f;
}

float taylor_cos(float val) {
    return 1.f - val * val / 2.0f + val*val*val*val/24.0;
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


// radially moves target away from source if too close
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

// checks if target can get hit
bool can_hit(const float t_x, const float t_y){
    const float rel_x = t_x - pos_x;
    const float rel_y = t_y - pos_y;

    const float inv_det = 1.f / (plane_x * dir_y - dir_x * plane_y);
    const float plane_coord = inv_det * (dir_y * rel_x - dir_x * rel_y);
    const float dir_coord = inv_det * (-plane_y * rel_x + plane_x * rel_y);
    
    if (dir_coord < 0.f)
        return false;

    int32_t screen_x = (int32_t)(32.f * (plane_coord / dir_coord + 1));
    const float perp_dist = dir_coord * dirlen;
    
    // compute screen projected size of target and check zbuf if blocked
    int32_t size = (int32_t)(SCREEN_HEIGHT / perp_dist);
    return (screen_x + size >= DRAW_WIDTH / 2) && (screen_x - size <= DRAW_WIDTH / 2) && (zbuffer[DRAW_WIDTH / 2] > perp_dist);
}

