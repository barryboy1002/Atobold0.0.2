#ifndef VGA_TEXT_H
#define VGA_TEXT_H

#include <stddef.h>
#include <stdint.h>

/*For now we can't have more than 15 colors*/
typedef enum  {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE,
  VGA_COLOR_GREEN,
  VGA_COLOR_CYAN,
  VGA_COLOR_RED,
  VGA_COLOR_MAGENTA,
  VGA_COLOR_BROWN,
  VGA_COLOR_LIGHT_GREY,
  VGA_COLOR_DARK_GREY,
  VGA_COLOR_LIGHT_BLUE,
  VGA_COLOR_LIGHT_GREEN,
  VGA_COLOR_LIGHT_CYAN,
  VGA_COLOR_LIGHT_RED,
  VGA_COLOR_LIGHT_MAGENTA,
  VGA_COLOR_LIGHT_BROWN,
  VGA_COLOR_WHITE
}vga_color;

typedef struct
{
  size_t row;
  size_t column;

  size_t width;
  size_t height;
  
  uint8_t color;

  volatile uint16_t * buffer;

} vga_text;

/* initialize the terminal */
void vga_text_init (vga_text * terminal);

/* clear the screen*/
void vga_text_clear(vga_text * terminal);

/* scroll the screen up by one line (called when the cursor moves past
   the last row) */
void vga_text_scroll(vga_text * terminal);

/* cursor ops */
void vga_text_set_cursor(
    vga_text * terminal,
    size_t row,
    size_t column
);

/* Color ops */ 
void vga_text_set_color(
    vga_text * terminal,
    vga_color f,
    vga_color b
);

/* char out */
void vga_text_putchar(
    vga_text * terminal,
    char c
);

/*string out */ 
void vga_text_write(
    vga_text * terminal,
    const char * string
);

/*write a string then a newline*/
void vga_text_writeline(
    vga_text * terminal,
    const char* string
);
/* write numbers onto the screen */
void vga_text_writedec(
    vga_text * terminal,
    uint32_t value
);

void vga_text_writehex(
    vga_text * terminal,
    uint32_t value 
);

/* low level writing*/
void vga_text_put_entry_at(
    vga_text * terminal,
    char character,
    uint8_t fcolor,
    uint8_t bcolor,
    size_t row,
    size_t column
);








#endif

