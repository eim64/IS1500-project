#include <pic32mx.h>  
#include "display.h"
#include "game.h"
#include "killscreen.h"

void user_isr() { }

void input_init() {
  // Pushbuttons init
  // BTN1, PORT F, Bit 1
  // BTN2-3, PORT D, Bit 5-7
  TRISFSET = (1 << 1);
  TRISDSET = (7 << 5);

  // Switches init
  // Port D, Bit 8-11
  TRISDSET = (0xF << 8);

  // Potentiometer
  TRISBSET = 0x4; // Port E, bit 3, Potentiometer
  AD1PCFG = 0xFFFB; // ADC config reg, bit 2 is 0
  AD1CHS = (1 << 17); // Channel select

  AD1CON1SET = (0x1 << 10); // value 32 bit integer
  AD1CON1SET = (0x7 << 5); // end samp start conv(Auto)
  AD1CON1SET = (0x1 << 15); // ADC ON
  AD1CON3SET = (0x1 << 15); // Use the ADC internal clock
}

void output_init() {
  // User LED init, PORT E, bit 0-7
  TRISECLR = 0xFF; // set as output
  PORTE = 0x0; // clear value

  // OLED init
  PORTF = 0xFFFF;
  PORTG = (1 << 9); // Port G, Bit 8, OLED serial 
  TRISFCLR = 0x70; // OLED VDD enable, OLED data/command select and OLED VBAT enable
  TRISGCLR = 0x200; // RG9, OLED Reset

  // SPI init
  SPI2CON = 0;
  SPI2BRG = 4; // Baud rate
  // Receive Overflow Flag, 0 is No overflow has occurred
  SPI2STATCLR = 0x40;
  // Clock Polarity Select, "Idle state for clock is a high level; active state is a low level"
  SPI2CONSET = 0x40; 
  // Master mode enable 
  SPI2CONSET = 0x20; 
  // Enable SPI preipheral
  SPI2CONSET = 0x8000;


  // Initalize timer 2
  //          80 MHz        42ms -> 24 FPS
  // period = 80 000 000 * (42*10^-3) = 3 360 000
  // 1:256 prescale:  3 360 000 / 256 = 13 125 = 0x3345
  // 0x8070 -> 7, set bits6-4 TCKPS<2:0> to 111 to prescale with 256
  // 0x8070 -> 8, set bit 15 is 1 to start the timer 
  TMR2 = 0; 
  T2CON = 0x8070;
  PR2 = 0x3345;
}

game_state p_state;

int main()
{
  output_init();

  input_init();

  display_init();

  restart_game();
  
  while(1)
  {
    switch (current_state) {
      case gaming:  game_logic();  break;
      case deadass: death_logic(); break;
    }

    if(current_state != p_state) {
      switch (current_state)
      {
        case gaming:  restart_game();   break;
        case deadass: update_message(); break;
      }
    }


    if(IFS(0) & 0x100) {
      display_update();
      // Reset the interupt status flag
      IFSCLR(0) = 0x100;
    }

    p_state = current_state;
  }
}