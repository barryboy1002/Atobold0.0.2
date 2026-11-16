#include "kernel/drivers/keyboard.h"
#include "kernel/interrupts.h"
#include "kernel/drivers/vga_text.h"
#include "lib/stdint.h"

#define PS2_DATA 0x60
#define PS2_COMMAND  0x64
#define PS2_STATUS  0x64

#define DISABLE_DEVICES  0xAD
#define SET_SYSTEM 0x20

//special bytes
#define SELF_TEST 0xAA

#define KEYMAP_SIZE 128

typedef enum{
  STATE_NORMAL,
  STATE_RELEASED,
  STATE_EXTENDED,
  STATE_EXTENDED_RELEASE
} keyboard_state;

static const char keymap[KEYMAP_SIZE] = {
            /* Numbers row */
        [0x0E] = '`',
        [0x16] = '1',
        [0x1E] = '2',
        [0x26] = '3',
        [0x25] = '4',
        [0x2E] = '5',
        [0x36] = '6',
        [0x3D] = '7',
        [0x3E] = '8',
        [0x46] = '9',
        [0x45] = '0',
        [0x4E] = '-',
        [0x55] = '=',
        [0x66] = '\b',

        /* Top letter row (QWERTY) */
        [0x0D] = '\t',
        [0x15] = 'q',
        [0x1D] = 'w',
        [0x24] = 'e',
        [0x2D] = 'r',
        [0x2C] = 't',
        [0x35] = 'y',
        [0x3C] = 'u',
        [0x43] = 'i',
        [0x44] = 'o',
        [0x4D] = 'p',
        [0x54] = '[',
        [0x5B] = ']',
        [0x5D] = '\\',

        /* Middle letter row (ASDF) */
        [0x1C] = 'a',
        [0x1B] = 's',
        [0x23] = 'd',
        [0x2B] = 'f',
        [0x34] = 'g',
        [0x33] = 'h',
        [0x3B] = 'j',
        [0x42] = 'k',
        [0x4B] = 'l',
        [0x4C] = ';',
        [0x52] = '\'',
        [0x5A] = '\n',

        /* Bottom letter row (ZXCV) */
        [0x1A] = 'z',
        [0x22] = 'x',
        [0x21] = 'c',
        [0x2A] = 'v',
        [0x32] = 'b',
        [0x31] = 'n',
        [0x3A] = 'm',
        [0x41] = ',',
        [0x49] = '.',
        [0x4A] = '/',

        /* Spacebar */
        [0x29] = ' ',
};

extern vga_text terminal;

static keyboard_state key_sm = STATE_NORMAL;

static void ps2_wait_write(void){
    //poll status register until the input buffer is 0(empty).
    while(inb(PS2_STATUS) & (1 << 1));
}

static void ps2_wait_read(void){
  //poll status register until the  output buffer is 1(full)
  while(!(inb(PS2_STATUS) & (1 << 0)));
}

static void ps2_write_data(uint8_t data){
  ps2_wait_write();
  outb(PS2_DATA, data);
}

static uint8_t  ps2_read_data(){
  ps2_wait_read();
  return inb(PS2_DATA);
}

static void ps2_write_comm(uint8_t command){
  ps2_wait_write();
  outb(PS2_COMMAND, command);
}

static bool sc_checker(uint8_t sc_code, const char * err_string){
  if(ps2_read_data() != sc_code){
    vga_text_writeline(&terminal, err_string);
    return FALSE;
  }
  return TRUE;
}

void ps2_controller_init () {
  //disable devices
  ps2_write_comm(DISABLE_DEVICES);

  //flushing the output devices
  inb(PS2_DATA);
  //setting control config byte
  ps2_write_comm(0x20);
  uint8_t config  = ps2_read_data();

  config &= ~((1<<0) | (1 << 1) | (1<< 6));

  ps2_write_comm(0x60);
  ps2_write_data(config);

  //controller self test
  ps2_write_comm(SELF_TEST);
  if (sc_checker(0x55, "failed self controller test") == FALSE) return;
  
  //interface test 
  ps2_write_comm(0xAB);
  if(sc_checker(0x00 , "failed internal test") == FALSE) return;
  
  //enable first ps/2 port
  ps2_write_comm(0xAE);

}

void ps2_keyboard_init (){
  //static uint8_t retry = 3;
  ps2_write_data(0xFF);
  if(sc_checker(0xFA, "failed to reset keyboard") == FALSE) return;
  if(sc_checker(0xAA, "failed assurance test") == FALSE) return; 
  
  
  ps2_write_data(0xF0);
  if (sc_checker(0xFA, "failed to set scancode") == FALSE) return;

  ps2_write_data(0x02);
  if (sc_checker(0xFA, "failed to set scancode") == FALSE) return;

  ps2_write_data(0xF4);
  if (sc_checker(0xFA, "failed to enable scanning") == FALSE)  return;

  ps2_write_comm(0x20);
  uint8_t config = ps2_read_data();

  config |= (1<<0);
  ps2_write_comm(0x60);
  ps2_write_data(config);
}

void keyboard_handler(void){
  uint8_t scancode = ps2_read_data();
  if(scancode > KEYMAP_SIZE+1 && scancode != 0xF0 && scancode != 0xE0) vga_text_writeline(&terminal, "Not accounted for");
  switch(key_sm){
    case(STATE_NORMAL):
      if(scancode == 0xF0){
        key_sm = STATE_RELEASED;
        break;
      }
      else if(scancode  ==  0xE0){
        key_sm = STATE_EXTENDED;
        break;
      }
      char pressed = keymap[scancode];
      if(pressed != 0){
        vga_text_putchar(&terminal, pressed);
      }
      break;
    case(STATE_RELEASED):
      key_sm = STATE_NORMAL;
      break;
    case(STATE_EXTENDED):
      if(scancode == 0xF0){
        key_sm = STATE_EXTENDED_RELEASE;
        break;
      }
      else{
        key_sm = STATE_NORMAL;
        break;
      }
      break;
    case(STATE_EXTENDED_RELEASE):
      key_sm = STATE_NORMAL;
      break;
    default:
      break;
  }
  
}

/* TODO :
 * set a state machine for  making sure we don't have double bytes
 * */





