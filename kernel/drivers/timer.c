/* Using the PIT timer*/


/*PIT ports*/
#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL1_DATA 0x41
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND 0x43

/*PIT input Clock*/
#define BASE_FREQUENCY  1193182

/*Channel selection*/
#define PIT_CHANNEL0  0x00
#define PIT_CHANNEL1  0x40
#define PIT_CHANNEL2  0x80


/*Access modes*/ 
#define LATCH_MODE 0x00 
#define LOBYTE_MODE 0x10
#define HIBYTE_MODE 0x20
#define LOHIBYTE_MODE 0x30

/*Operating Modes*/ 
#define MODE0 0x00
#define MODE1 0x02
#define MODE2 PIT_CHANNEL0_DATAx04
#define MODE3 0x06
#define MODE4 0x08
#define MODE5 0x0A

/*Counting Modes*/ 
#define PIT_BINARY  0x00
#define PIT_BCD 0x01



#include "kernel/interrupts.h"
#include "kernel/drivers/timer.h"
#include "kernel/drivers/vga_text.h"

volatile uint32_t ticks = 0 ;

extern vga_text terminal;


void timer_init(uint32_t frequency){
  uint16_t divisor = BASE_FREQUENCY / frequency;

  /*tell PIT how we send the divisor value and the mode */
  outb(PIT_COMMAND, LOHIBYTE_MODE | MODE3 | PIT_CHANNEL0 | PIT_BINARY);
  io_wait();
  
  /*write low and high bytes respectively*/
  outb(PIT_CHANNEL0_DATA, divisor & 0xFF);
  io_wait();
  outb(PIT_CHANNEL0_DATA, divisor >> 8);
  io_wait();
  
}

void timer_handler(void){
  ticks++;
  if((ticks % 100) == 0){
    vga_text_writeline(&terminal, "1 second");
  }
}


uint32_t timer_get_ticks(){
  return ticks;
}


