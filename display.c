#include "display.h"
#include <stdlib.h>
#include <stdint.h>
#include <pic32mx.h>

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

#define SET_COLUMN_ADDRESS                   ((uint8_t)0x21)
#define SET_PAGE_ADDRESS                     ((uint8_t)0x22)
#define SET_PAGE_START_ADDR(n)               ((uint8_t)(0xB0 + (n)))
#define SET_START_LINE(n)                    ((uint8_t)(0x40 + (n)))
#define SET_LOWER_COL_START(n)               ((uint8_t) (n))
#define SET_HIGHER_COL_START(n)              ((uint8_t) (0x10 + (n)))
#define SET_MEMORY_MODE                      ((uint8_t) 0x20)
#define RIGHT_HORIZONTAL_SCROLL              ((uint8_t) 0x26)
#define LEFT_HORIZONTAL_SCROLL               ((uint8_t) 0x27)
#define VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL ((uint8_t) 0x29)
#define VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  ((uint8_t) 0x2A)
#define DEACTIVATE_SCROLL                    ((uint8_t) 0x2E)
#define ACTIVATE_SCROLL                      ((uint8_t) 0x2F)
#define SET_CONTRAST                         ((uint8_t) 0x81)
#define SET_CHARGE_PUMP                      ((uint8_t) 0x8D)
#define SET_SEGMENT_REMAP                    ((uint8_t) 0xA0)
#define SET_SEGMENT_REMAP_REVERSE            ((uint8_t) 0xA1)
#define SET_VERTICAL_SCROLL_AREA             ((uint8_t) 0xA3)
#define DISPLAY_MODE_NORMAL                  ((uint8_t) 0xA4)
#define DISPLAY_MODE_ALL_ON                  ((uint8_t) 0xA5)
#define DISPLAY_MODE_ALL_OFF                 ((uint8_t) 0xA6)
#define DISPLAY_MODE_INVERT                  ((uint8_t) 0xA7)
#define SET_MULTIPLEX_RATIO                  ((uint8_t) 0xA8)
#define DISPLAY_OFF                          ((uint8_t) 0xAE)
#define DISPLAY_ON                           ((uint8_t) 0xAF)
#define SET_COM_SCAN_INC                     ((uint8_t) 0xC0)
#define SET_COM_SCAN_DEC                     ((uint8_t) 0xC8)
#define SET_DISPLAY_OFFSET                   ((uint8_t) 0xD3)
#define SET_DISPLAY_CLOCK_DIV                ((uint8_t) 0xD5)
#define SET_PRECHARGE_PERIOD                 ((uint8_t) 0xD9)
#define SET_COM_PINS                         ((uint8_t) 0xDA)
#define SET_VCOMH_DESELECT                   ((uint8_t) 0xDB)
#define NOOP                                 ((uint8_t) 0xE3)

uint8_t screen_buffer[SCREEN_BUFFER_SIZE];
uint8_t spi_send_recv(uint8_t data);

void quicksleep(int cyc) {
	while(--cyc > 0);
}

void display_init(){
    DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	//display_command(DISPLAY_OFF);
    spi_send_recv(DISPLAY_OFF);

	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(SET_CHARGE_PUMP);
    spi_send_recv(0x14);

    spi_send_recv(SET_PRECHARGE_PERIOD);
    spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
    spi_send_recv(SET_SEGMENT_REMAP_REVERSE);

    spi_send_recv(SET_COM_SCAN_DEC);

    spi_send_recv(SET_MEMORY_MODE);
    spi_send_recv(0);
    
    spi_send_recv(SET_COM_PINS);
    spi_send_recv(0x20);

    spi_send_recv(DISPLAY_ON);

    display_clear();
    display_update();
}

void display_clear() {
    int i;
    
    for(i = 0; i < SCREEN_BUFFER_SIZE; i++)
        screen_buffer[i] = 0;
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_update() {
    DISPLAY_CHANGE_TO_COMMAND_MODE;
    spi_send_recv(SET_COLUMN_ADDRESS);
    spi_send_recv(0);
    spi_send_recv(SCREEN_WIDTH - 1);
    
    spi_send_recv(SET_PAGE_ADDRESS);
    spi_send_recv(0);
    spi_send_recv(SCREEN_HEIGHT / 8 - 1);

    DISPLAY_CHANGE_TO_DATA_MODE;
    
    int i;
    for (i = 0; i < SCREEN_BUFFER_SIZE; i++)
        spi_send_recv(screen_buffer[i]);
}

void display_px(uint8_t x, uint8_t y, uint8_t on) {
    uint8_t* index = screen_buffer + (x + (y >> 3) * SCREEN_WIDTH);
    const uint8_t bit = (1 << (y & 7));

    if (on) *index |= bit;
    else *index &= ~bit;
}

void display_setpx(uint8_t x, uint8_t y){
    uint8_t* index = screen_buffer + (x + (y >> 3) * SCREEN_WIDTH);
    const uint8_t bit = (1 << (y & 7));

    *index |= bit;
}

void display_clrpx(uint8_t x, uint8_t y) {
    uint8_t* index = screen_buffer + (x + (y >> 3) * SCREEN_WIDTH);
    const uint8_t bit = (1 << (y & 7));

    *index &= ~bit;
}