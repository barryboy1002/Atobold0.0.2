#include "kernel/drivers/timer.h"
#include "kernel/drivers/vga_text.h"
#include "kernel/utils/sleep.h"


extern vga_text terminal;

void sleep(uint32_t seconds){
  uint32_t start_time = (timer_get_ticks())/100;
  uint32_t xend_time = start_time + seconds*100;
  vga_text_writeline(&terminal, "sleeping started");
  while((timer_get_ticks()) < xend_time){
      
  }
  vga_text_writeline(&terminal, "sleeping stopped");
}

