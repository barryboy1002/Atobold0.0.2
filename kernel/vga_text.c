#include "vga_text.h"

void vga_text_init(vga_text * terminal)
{
  terminal->row = 0;
  terminal->column = 0;

  vga_text_set_color(terminal, VGA_COLOR_WHITE, VGA_COLOR_LIGHT_GREY);
  
  terminal->width = 80;
  terminal->height = 50;

  terminal->buffer = (uint16_t *)0XB8000;
  vga_text_clear(terminal);
}

void vga_text_clear(vga_text * terminal)
{
  uint8_t color = terminal->color;
  uint16_t blank = ((uint16_t)color << 8) | '';

  for(size_t row = 0; row < terminal->height; row++){
    for(size_t col =0 ; col  < terminal->width; col++){
      size_t index = row * terminal->width + col;
      terminal->buffer[index] = blank;
    }
  }

  terminal->row = 0;
  terminal->column = 0;
}

void vga_text_set_color(vga_text * terminal, vga_color f, vga_color b)
{
  terminal->color = ((uint8_t)b << 4) | (uint8_t)f;
}

void vga_text_set_cursor(vga_text * terminal, size_t row, size_t column)
{
  terminal->row = row;
  terminal->column = column;
}

