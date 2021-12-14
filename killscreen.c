#include "killscreen.h"
#include <pic32mx.h>

#include "graphics.h"
#include "display.h"
#include "game.h"
#include "io.h"


#define NUM_MESSAGES 4
char deathmessages[NUM_MESSAGES][16 + 1] = 
{
    "NOOB DOWN!!!",
    "CRINGE",
    "BRUH MOMENT",
    "CRY IS FREE)"
};

char* current_msg;
int current_length;

uint32_t p_btns;

void update_message() {
    current_msg = deathmessages[frame % NUM_MESSAGES];
    current_length = 0;

    while(current_msg[current_length++]);
    p_btns = 1;
}

void death_logic() {
    display_clear();

    draw_text(8 - current_length / 2, 0, current_msg);
    draw_text(8 - 5, 2, "KILLCOUNT:");
    draw_uint(8 - 2, 3, kill_count);

    // return to gaming
    uint32_t btns = getbtns(); 
    if(btns && !p_btns)
        current_state = gaming;

    // led effect
    PORTE = (1 << (((frame++) / 18) % 8));
    
    p_btns = btns;
}