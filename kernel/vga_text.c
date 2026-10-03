#include "vga_text.h"

void vga_text_init(vga_text * terminal)
{
  terminal->row = 0;
  terminal->column = 0;
  terminal->width = 80;
  terminal->height = 25;
  terminal->buffer = (volatile uint16_t *)0xB8000;
  
  vga_text_set_color(terminal, VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  vga_text_clear(terminal);
}

void vga_text_clear(vga_text * terminal)
{
  uint8_t color = terminal->color;
  uint16_t blank = ((uint16_t)color << 8) | ' ';

  for (; terminal->row >= 0; terminal->row--) {
    for (; terminal->column >= 0 ; terminal->column--) {
      size_t index = terminal->row * terminal->width + terminal->column;
      terminal->buffer[index] = blank;
    }
  }

  /*terminal->row = 0;
  terminal->column = 0;*/
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

void vga_text_putchar(vga_text * terminal, char c)
{
  if (c == '\n') {
    terminal->column = 0;
    terminal->row++;
  } else if (c == '\r') {
    terminal->column = 0;
  } else {
    uint8_t color = terminal->color;
    const size_t index = terminal->row * terminal->width + terminal->column;
    uint16_t entry = ((uint16_t)color << 8) | (uint16_t)c;
    terminal->buffer[index] = entry;
    terminal->column++;
  }

  if (terminal->column >= terminal->width) {
    terminal->column = 0;
    terminal->row++;
  }

  if (terminal->row >= terminal->height) {
    terminal->row = 0;
  }
}

void vga_text_write(vga_text * terminal, const char * string)
{
  for (size_t i = 0; string[i] != '\0'; i++) {
    vga_text_putchar(terminal, string[i]);
  }
}

void vga_text_writeline(vga_text * terminal, const char * string)
{
  vga_text_write(terminal, string);
  vga_text_putchar(terminal, '\n');
}

void vga_text_writedec(vga_text * terminal, uint32_t value)
{
  char buffer[10];
  size_t digits = 0;

  if (value == 0) {
    vga_text_putchar(terminal, '0');
    return;
  }

  while (value > 0) {
    buffer[digits++] = '0' + (value % 10);
    value /= 10;
  }

  while (digits > 0) {
    vga_text_putchar(terminal, buffer[--digits]);
  }
}

void vga_text_writehex(vga_text * terminal, uint32_t value)
{
  char buffer[8];
  size_t digits = 0;

  if (value == 0) {
    vga_text_putchar(terminal, '0');
    return;
  }

  while (value > 0) {
    uint32_t digit = value % 16;
    if (digit < 10) {
      buffer[digits++] = '0' + digit;
    } else {
      buffer[digits++] = 'A' + (digit - 10);
    }
    value /= 16;
  }

  while (digits > 0) {
    vga_text_putchar(terminal, buffer[--digits]);
  }
}

void vga_text_put_entry_at(
  vga_text * terminal,
  char character,
  uint8_t fcolor,
  uint8_t bcolor,
  size_t row,
  size_t column
) {
  uint8_t color = ((uint8_t)bcolor << 4) | (uint8_t)fcolor;
  size_t index = row * terminal->width + column;
  uint16_t entry = ((uint16_t)color << 8) | (uint16_t)character;
  terminal->buffer[index] = entry;
}
