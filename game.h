#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

extern float pos_x;
extern float pos_y;

extern float dir_x;
extern float dir_y;

extern float plane_x;
extern float plane_y;

extern const float dirlen;
extern const float planelen;

extern int frame;


#define DRAW_WIDTH 64

#define MAP_WIDTH 10
#define MAP_HEIGHT 10
extern uint8_t map[MAP_WIDTH][MAP_HEIGHT];
extern float zbuffer[DRAW_WIDTH];


#define ENTITY_COUNT 4
typedef struct {
    uint32_t* spr;
    float x;
    float y;
    float c_dist;
} entity_t;

extern entity_t entities[ENTITY_COUNT];
extern uint32_t e_index[ENTITY_COUNT];