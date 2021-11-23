#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

extern uint32_t aspr[16];
extern uint32_t enspr[16];
void sort_entities();
void draw_sprite(uint32_t *sprite, float x, float y);
void raycast_map();
void show_noise();
void draw_gun(int offset);
void draw_fire(int offset);