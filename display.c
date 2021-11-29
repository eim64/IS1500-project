#include "display.h"
#include <stdlib.h>
#include <stdint.h>
#include <pic32mx.h>

#define SET_COLUMN_ADDRESS                   ((uint8_t)0x21)
#define SET_PAGE_ADDRESS                     ((uint8_t)0x22)
#define SET_MEMORY_MODE                      ((uint8_t) 0x20)
#define SET_CHARGE_PUMP                      ((uint8_t) 0x8D)
#define SET_SEGMENT_REMAP_REVERSE            ((uint8_t) 0xA1)
#define DISPLAY_OFF                          ((uint8_t) 0xAE)
#define DISPLAY_ON                           ((uint8_t) 0xAF)
#define SET_COM_SCAN_INC                     ((uint8_t) 0xC0)
#define SET_COM_SCAN_DEC                     ((uint8_t) 0xC8)
#define SET_PRECHARGE_PERIOD                 ((uint8_t) 0xD9)
#define SET_COM_PINS                         ((uint8_t) 0xDA)

uint8_t screen_buffer[SCREEN_BUFFER_SIZE];

uint8_t SPI_send(uint8_t data) {
	// Wait for transmit buffer to be empty
    while(!(SPI2STAT & 0x08));
	
    // Write the data which is just a byte
    SPI2BUF = data;

    // Receive Buffer Status, wait for recieve to be completed
    while(!(SPI2STAT & 1));

    return SPI2BUF;
}

void delay(int time) {
	while(time >= 0) time--;
}

void display_init(){
    // Port F, bit 4, 1 if Data mode, 0 if command mode
    PORTFCLR = 0x10; 
	delay(10);
	
    // Port F, bit 6, 0 to turn on the VDD to send the power to the logic of the display.
    PORTFCLR = 0x40; 
	delay(1000000);
	
    // Send display off command.
    SPI_send(DISPLAY_OFF); 
    
    // Port G, bit 9, OLED Reset
	PORTGCLR = 0x200;
	delay(10);

    //Port G, bit 9, stop OLED Reset
	PORTGSET = 0x200; 
	delay(10);
	
    // Send set charge pump command
	SPI_send(SET_CHARGE_PUMP); 
    SPI_send(0x14);

    // Send set precharge period command
    SPI_send(SET_PRECHARGE_PERIOD); 
    SPI_send(0xF1);
	
	// Port F, bit 5, 0 to turn on the VBAT to send power to the display itself.
    PORTFCLR = 0x20; 
	delay(10000000);
	
    // Invert display by remapping columns and rows so that display starts in the upper left corner.
    SPI_send(SET_SEGMENT_REMAP_REVERSE);
    SPI_send(SET_COM_SCAN_DEC);

    // Memory addressing mode + Sequential COM configuration
    SPI_send(SET_MEMORY_MODE); // send the set memory addressing mode
    SPI_send(0);
    SPI_send(SET_COM_PINS); // send the set COM configuration command
    SPI_send(0x20);

    // Send display on command
    SPI_send(DISPLAY_ON);

    // Update
    display_clear();
    display_update();
}

void display_clear() {
    // Clear all pixels
    int i = 0;
    while(i < SCREEN_BUFFER_SIZE) 
      screen_buffer[i++] = 0;
}



void display_update() {
    PORTFCLR = 0x10; // Port F, bit 4, 1 if Data mode, 0 if command mode
    SPI_send(SET_COLUMN_ADDRESS);
    SPI_send(0);
    SPI_send(SCREEN_WIDTH - 1);
    
    SPI_send(SET_PAGE_ADDRESS);
    SPI_send(0);
    SPI_send(SCREEN_HEIGHT / 8 - 1);

    PORTFSET = 0x10; // Port F, bit 4, 1 if Data mode, 0 if command mode
    
    int i = 0;
    while(i < SCREEN_BUFFER_SIZE)
        SPI_send(screen_buffer[i++]);
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