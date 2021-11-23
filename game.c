#include "game.h"
#include <pic32mx.h>


#include "meth.h"
#include "display.h"
#include "graphics.h"
#include "io.h"
#include "sink_pathing.h"

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

float zbuffer[DRAW_WIDTH];

entity_t entities[ENTITY_COUNT] = {
    {aspr, 1.5f, 2.5f, 0.0},
    {aspr, 2.5f, 7.5f, 0.0},
    {aspr, 4.5f, 1.5f, 0.0},
    {enspr, 1.5f, 3.5f, 0.0}
};

uint32_t e_index[ENTITY_COUNT] = {0, 1, 2, 3};
entity_t* enemy = &entities[ENTITY_COUNT - 1]; 

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

            if(can_hit(enemy->x, enemy->y)){
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
        NORMALIZE(dir_x, dir_y, dirlen);
        
        // gram-schmidt to perpendicularity
        {
            const float scalar = (plane_x*dir_x + plane_y*dir_y)/(dirlen * dirlen);
            
            plane_x -= dir_x * scalar;
            plane_y -= dir_y * scalar;
        }

        NORMALIZE(plane_x, plane_y, planelen);
        
        // wall collision
        APPLY_COLLISION(pos_x, pos_y, 0.2f);
    }

    //RENDER MAP
    raycast_map();

    // PREPARE DISTANCES/DIRECTIONS
    create_sink(pos_x, pos_y);
    
    // ENTITIY RENDERING + INTERACTIONS
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
                follow_sink(e, 0.0025f);

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

        draw_fire(fire_offset);
        PORTE = 0xFF;
    }


    if(is_hit) {
        is_hit--;

        show_noise();
    }

    draw_gun(0);

    display_update();
    ++frame;
}
