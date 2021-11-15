#include <stdint.h>
#include <stdarg.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)
extern uint8_t screen_buffer[SCREEN_BUFFER_SIZE];

void display_init();
void display_update();
void display_clear();
void display_px(uint8_t x, uint8_t y, uint8_t on);
void display_setpx(uint8_t x, uint8_t y);
void display_clrpx(uint8_t x, uint8_t y);