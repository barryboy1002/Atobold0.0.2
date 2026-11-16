#include "kernel/drivers/timer.h"
#include "kernel/drivers/vga_text.h"
#include "kernel/utils/sleep.h"


extern vga_text terminal;

void sleep(uint32_t seconds){
  uint32_t start_time = uptime_get_seconds();
  uint32_t xend_time = start_time + seconds;
  vga_text_writeline(&terminal, "sleeping started");
  while(uptime_get_seconds() < xend_time){
    //spin     
  }
  vga_text_writeline(&terminal, "sleeping stopped");
}

