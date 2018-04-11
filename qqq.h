//   ***************************************************************
//   * Copyright (C) 2017, Embed Inc (http://www.embedinc.com)     *
//   *                                                             *
//   * Permission to copy this file is granted as long as this     *
//   * copyright notice is included in its entirety at the         *
//   * beginning of the file, whether the file is copied in whole  *
//   * or in part and regardless of whether other information is   *
//   * added to the copy.                                          *
//   *                                                             *
//   * The contents of this file may be used in any way,           *
//   * commercial or otherwise.  This file is provided "as is",    *
//   * and Embed Inc makes no claims of suitability for a          *
//   * particular purpose nor assumes any liability resulting from *
//   * its use.                                                    *
//   ***************************************************************
//
//   C include file for the QQ2 firmware.  This include file defines the
//   capabilities of the underlying system available to C code.
//
#include "qq2_machine.h"               //define machine-dependent data types
#include "qq2_const.h"                 //define constants from asm environment

//******************************************************************************
//
//   Interface to the general system.
//
//   Machine-customized data types for use by application code:
//
//     int8u_t  -  Unsigned 8 bit integer
//     int8s_t  -  Signed 8 bit integer
//     int16u_t  -  Unsigned 16 bit integer
//     int16s_t  -  Signed 16 bit integer
//     int32u_t  -  Unsigned 32 bit integer
//     int32s_t  -  Signed 32 bit integer
//     machine_intu_t  -  Unsigned native machine word integer
//     machine_ints_t  -  Signed native machine word integer
//
//   Pre-defined constants:
//
//     fwtype_k  -  Firmware type ID
//     fwver_k  -  1-N firmware version number within type
//     fwseq_k  -  1-N firmare build sequence number within type/version
//
machine_intu_t clock_1ms (void);       //get 1 ms clock tick counter
machine_intu_t clock_10ms (void);      //get 10 ms clock tick counter
machine_intu_t clock_100ms (void);     //get 100 ms clock tick counter
int32u_t clock_seconds (void);         //get seconds since powerup or reset, 136 year range
int16u_t clock_seconds16 (void);       //faster simpler 16 bit seconds, 18.2 hour range

void __attribute__((noreturn))
  system_reset (void);                 //reset the whole system

void task_yield (void);                //let other tasks run for a while

void waitms (                          //wait fixed time while letting other tasks run
  machine_intu_t);                     //time to wait in milliseconds

//******************************************************************************
//
//   Floating point manipulation routines.
//
int32s_t                               //returned 32 bit signed fixed point number
fp32_fixs (                            //FLOAT to fixed point, rounded and saturated
  float,                               //floating point input value
  machine_ints_t);                     //number of fraction bits in result, may be negative

int32u_t                               //returned 32 bit unsigned fixed point number
fp32_fixu (                            //FLOAT to fixed point, rounded and saturated
  float,                               //floating point input value
  machine_ints_t);                     //number of fraction bits in result, may be negative

//******************************************************************************
//
//   Interface to the host.
//
//   A constant named rsp_xxx_k is defined for each response opcode, where xxx
//   is the response name.
//
machine_intu_t cmd_get8u (void);       //get command stream integer values
machine_ints_t cmd_get8s (void);
machine_intu_t cmd_get16u (void);
machine_ints_t cmd_get16s (void);
int32u_t cmd_get32u (void);
int32s_t cmd_get32s (void);

void cmd_lock_out (void);              //acquire exclusive lock on response stream

void cmd_unlock_out (void);            //release lock on response stream

void cmd_put8u (machine_intu_t);       //write integers to response stream
void cmd_put8s (machine_ints_t);
void cmd_put16u (machine_intu_t);
void cmd_put16s (machine_ints_t);
void cmd_put32u (int32u_t);
void cmd_put32s (int32s_t);

void debug1 (                          //send debug info to host
  machine_intu_t,                      //16 bit value
  const char *);                       //string that will be shown with value

void debug2 (                          //send debug info to host
  int32u_t,                            //32 bit value
  const char *);                       //string that will be shown with value

//******************************************************************************
//
//   Byte buffer interface.  These routines facilitate reading and writing bytes
//   and words to/from a buffer.  These kinds of buffers are used as low level
//   facilities by various sub-systems.
//
//   The code is in the standard XC16 assembler module.
//
machine_intu_t                         //0-255 byte value
buf_get8u (                            //get next byte value from buffer
  int8u_t * *,                         //pointer to buffer byte, updated
  machine_intu_t *);                   //bytes left to read from buffer, updated

machine_intu_t                         //unsigned word value
buf_get16u (                           //get next word from buf, high-low order
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *);                   //bytes left to read from buffer, updated

machine_ints_t                         //signed word value
buf_get16s (                           //get next word from buf, high-low order
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *);                   //bytes left to read from buffer, updated

void buf_put8u (                       //write byte into buffer
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *,                    //number of bytes in buffer, updated
  machine_intu_t);                     //byte value to write in low 8 bits

void buf_put8s (                       //write byte into buffer
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *,                    //number of bytes in buffer, updated
  machine_ints_t);                     //byte value to write in low 8 bits

void buf_put16u (                      //write word into byte buffer, high-low order
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *,                    //number of bytes in buffer, updated
  machine_intu_t);                     //word value to write

void buf_put16s (                      //write word into byte buffer, high-low order
  int8u_t * *,                         //pointer to next buffer byte, updated
  machine_intu_t *,                    //number of bytes in buffer, updated
  machine_ints_t);                     //word value to write

//******************************************************************************
//
//   Interface to the non-volatile memory.
//
//   The non-volatile memory is presented as a sequence of 8-bit bytes.
//   Addresses are 32 bits for compatibility with various possible
//   implementations.
//
//   Data that is written to the non-volatile memory may be cached transparently
//   to the application.  Reads will always return the latest written data,
//   whether that has been physically written to the non-volatile memory or not.
//   Cached changed data will be automatically written to the physical memory
//   in the background within a minimum time after the last write.  The
//   application can call NVOL_FLUSH to force all changes to be written to the
//   non-volatile memory immediately.  NVOL_FLUSH does not return until any such
//   outstanding writes have completed.
//
//   This implementation:
//
//     Automatic flush time:            xx second
//     Write time:                      xx milliseconds maximum
//
int32u_t nvol_size (void);             //get size of non-volatile memory, bytes

machine_intu_t                         //0-255 byte value
nvol_read (                            //read byte from non-volatile memory
  int32u_t *);                         //address, incremented by 1

void nvol_write (                      //write byte to non-volatile memory
  int32u_t *,                          //address, incremented by 1
  machine_intu_t);                     //byte value in low 8 bits

void nvol_flush (void);                //force cached changes to HW now

//******************************************************************************
//
//   Interface to the display and buttons.
//
//   Event IDs are:
//
//     event_none_k  -  No event, returned by EVENT_WAIT on timeout
//     event_left_press_k  -  Left button pressed
//     event_left_release_k  -  Left button released
//     event_right_press_k  -  Right button pressed
//     event_right_release_k  -  Right button released
//     event_up_press_k  -  Up button pressed
//     event_up_release_k  -  Up button released
//     event_down_press_k  -  Down button pressed
//     event_down_release_k  -  Down button released
//     event_mid_press_k  -  Middle button pressed
//     event_mid_release_k  -  Middle button released
//     event_onoff_press_k  -  on/off button pressed
//     event_onoff_release_k  -  on/off button released
//     event_leftright_k  -  Left and right buttons held for a while
//
//   EVENT_WAIT timeout:
//
//     evwaitms_k  -  Units of EVENT_WAIT timeout value, in milliseconds
//     evwaithz_k  -  EVENT_WAIT timeout counts per second, rounded to integer
//
//   Special characters are:
//
//     char_bell_k  -  Emits short tone, no position change
//     char_cr_k  -  Carriage return, moves to first char on line
//     char_lf_k  -  Line feed, moves one line down unless at bottom
//     char_back_k  -  Moves one char left unless at left edge
//     char_left_k  -  Left-pointing marker
//     char_right_k  -  Right-pointing marker
//     char_up_k  -  Up-pointing marker
//     char_down_k  -  Down-pointing marker
//     char_block_k  -  Filled block
//
#include "qq2_disp.h"                  //constants exported from ASM environment

extern machine_intu_t disp_fwtype;     //display firmware type ID, 0 = unknown
extern machine_intu_t disp_fwver;      //display firmware version number, 1-N
extern machine_intu_t disp_fwseq;      //display firmware sequence number, 1-N

machine_intu_t                         //EVENT_xxx_K event ID
event_wait (                           //wait for next event or timeout
  machine_intu_t);                     //max time to wait, units of EVWAITMS_K ms

machine_intu_t                         //EVENT_xxx_K event ID
event_check_lr (                       //check for special left/right buttons hold event
  machine_intu_t);                     //last event received

void events_clear (void);              //clear all pending events

void disp_backlight (                  //display backlight on/off
  machine_intu_t);                     //0 = OFF, else ON

machine_intu_t                         //display contrast setting, 0-255
disp_cont (void);                      //get display contrast setting

machine_intu_t                         //resulting display contrast setting, 0-255
disp_cont_set (                        //set display contrast
  machine_intu_t);                     //desired contrast, 0-255, may be clipped to useful range

machine_intu_t                         //new display contrast setting, 0-255
disp_contup (void);                    //increment display contrast setting

machine_intu_t                         //new display contrast setting, 0-255
disp_contdown (void);                  //decrement display contrast setting

void disp_cont_off (void);             //lowest possible contrast, doesn't change setting

void disp_cont_on (void);              //restore contrast to current setting, undoes DISP_CONT_OFF

void disp_save (void);                 //save all persistent settings in non-volatile memory

void disp_clear (void);                //clear display, reset to top left corner

void disp_clearto (                    //clear towards right
  machine_intu_t);                     //0-N column, curr pos left one right

void disp_pos (                        //set character position
  machine_intu_t,                      //0-N column number, left to right
  machine_intu_t);                     //0-N line number, top to bottom

void disp_char (                       //write character, advance char pos 1 right
  machine_intu_t);                     //character code in low 8 bits

void disp_string (                     //write string to display
  const char *);                       //null-terminated string to write

void disp_crlf (void);                 //go to start of next line down

void disp_digit (                      //write 0-9,A-Z digit
  machine_intu_t);                     //0-35 digit value

void disp_int16u (                     //write unsigned decimal integer
  machine_intu_t,                      //integer value to write
  machine_intu_t);                     //1-16 min field width, leading blank padded

void disp_int16s (                     //write signed decimal integer
  machine_ints_t,                      //integer value to write
  machine_intu_t);                     //1-16 min field width, leading blank padded

void disp_int16h (                     //write unsigned hexadecimal integer
  machine_intu_t,                      //integer value to write
  machine_intu_t);                     //1-16 min field width, leading zero padded

void disp_int32h (                     //write 32 bit unsigned hexadecimal integer
  int32u_t,                            //integer value to write
  machine_intu_t);                     //1-16 min field width, leading zero padded

void disp_int16o (                     //write unsigned octal integer
  machine_intu_t,                      //integer value to write
  machine_intu_t);                     //1-16 min field width, leading zero padded

void disp_int16b (                     //write unsigned binary integer
  machine_intu_t,                      //integer value to write
  machine_intu_t);                     //1-16 min field width, leading zero padded

#define int32fx_signed_k 0x01          //the input value is signed, not unsigned
#define int32fx_point_k 0x02           //write decimal point even if no digits to right
#define int32fx_nzero_k 0x04           //don't write zero if only digit left of point
#define int32fx_leadz_k 0x08           //fill field with leading zeros, not blanks

void disp_int32fx (                    //write fixed point integer value in decimal
  int32u_t,                            //32 bit fixed point value
  machine_intu_t,                      //minimum field width, up to 31
  machine_intu_t,                      //number of fraction digits right of decimal point
  machine_intu_t);                     //set of flags, use INT32FX_xxx_K

//******************************************************************************
//
//   Interface to the menu system.
//
//   These functions are layered on the low level system display interface.
//   They provide a way to use the display and buttons to implement menus.
//

//   Special values returned by MENU_SELECT.
//
#define menu_sel_leftright_k 0         //special left/right button event
#define menu_sel_cancel_k -1           //user deliberately cancelled the menu
#define menu_sel_off_k -2              //user wants system or interface off
#define menu_sel_timeout_k -3          //user failed to respond within timeout

//   Template for app-supplied routine to write a single menu entry.
//
//   This routine is called to write each entry of a scrollable or dynamic menu
//   or list.  On entry, the display writing position will be at the first
//   writeable character position of the menu entry.  After return, the
//   remainder of the line will be cleared.
//
//   Entries of a selectable menu can be up to 18 characters long.  Of the 20
//   characters on a line, the first is reserved for the selection indicator,
//   and the last to indicate scrollability.
//
//   Entries of a list (not selectable) can be up to 19 characters long.  Unlike
//   selectable menu entries, the first character of the line is not reserved
//   for the selection indicator.
//
//   The return value is used to indicate to the menu system when the end of the
//   menu is reached.  For a valid 1-N entry number, the routine must write the
//   entry text and return TRUE.  For a invalid entry number, the routine must
//   return FALSE and not write to the display.
//
typedef machine_intu_t                 //TRUE written, FALSE no such entry
(*menu_entry_write) (                  //typedef name
  machine_intu_t);                     //1-N number of entry to write

//   Entry points.
//
void menu_sys_init (void);             //one-time initialization of the menu system

void menu_sys_on (                     //indicate user wants the device on or off
  machine_intu_t);                     //0 for off, anything else for on

void menu_new_static (                 //new fixed menu, 3 long or 6 short entries
  const char *);                       //title

void menu_entry_set_n (                //set specific entry of fixed menu
  machine_intu_t,                      //1-6 entry number
  const char *);                       //entry name, 19 char long, 9 char short

machine_intu_t                         //max number of chars allowed to write
menu_entry_start_n (                   //set up for writing static menu entry
  machine_intu_t);                     //1-6 entry number

void menu_entry_add (                  //add next entry to static menu
  const char *);                       //entry name, 19 char long, 9 char short

void menu_new_static_ents (            //new fixed menu, entries defined here
  const char *,                        //title
  const char *);                       //entries, NULL after each, extra NULL ends

void menu_new_scroll (                 //new scrollable menu
  const char *,                        //title
  machine_intu_t,                      //auto refresh, timeout ticks, 0 = no auto refresh
  menu_entry_write);                   //routine to call to write each entry

void menu_new_scroll_list (            //new scrollable list, not selectable
  const char *,                        //title
  machine_intu_t,                      //auto refresh, timeout ticks, 0 = no auto refresh
  menu_entry_write);                   //routine to call to write each entry

void menu_leftright_allow (void);      //allow special left/right button event

machine_ints_t                         //1-N entry number or MENU_SEL_xxx_K special IDs
menu_select (                          //let user select menu entry
  machine_intu_t,                      //1-N entry to start at
  machine_intu_t);                     //inactivity timeout, units of EVWAITMS_K ms

machine_intu_t                         //MENU_SEL_xxx_K terminate reason, never selection
menu_list_run (                        //show the list, handle user actions
  machine_intu_t,                      //1-N entry to start at
  machine_intu_t);                     //inactivity timeout, units of EVWAITMS_K ms

machine_intu_t                         //0 menu abort, 1 cancelled, 2 new confirmed
menu_val_fx16u (                       //user edit a 16 bit unsigned fixed point value
  const char *,                        //title to display during editing
  int16u_t *,                          //pointer to the value to modify
  int16u_t,                            //min allowed value
  int16u_t,                            //max allowed value
  machine_intu_t);                     //number of digits right of decimal point

machine_intu_t                         //0 menu abort, 1 cancelled, 2 new confirmed
menu_val_fx32u (                       //user edit a 32 bit unsigned fixed point value
  const char *,                        //title to display during editing
  int32u_t *,                          //pointer to the value to modify
  int32u_t,                            //min allowed value
  int32u_t,                            //max allowed value
  machine_intu_t);                     //number of digits right of decimal point

machine_intu_t                         //0 menu abort, 1 cancelled, 2 new confirmed
menu_val_float (                       //user edit a floating point value
  const char *,                        //title to display during editing
  float *,                             //pointer to the value to modify
  float,                               //min allowed value
  float,                               //max allowed value
  machine_intu_t);                     //number of digits to show right of decimal point

//******************************************************************************
//
//   Interface to the Jowa CAN bus (JCAN).
//
void jcan_typea_start (                //init for sending type A JCAN data frame
  machine_intu_t,                      //N, 0 - 3 (2 bits)
  int32u_t);                           //ID, 0 - 1,048,575 (20 bits)

void jcan_typeb_start (                //init for sending type B JCAN data frame
  machine_intu_t,                      //N, 0 - 63 (6 bits)
  machine_intu_t);                     //ID, 0 - 65535 (16 bits)

void jcan_typec_start (                //init for sending type C JCAN data frame
  machine_intu_t,                      //N, 0 - 1023 (10 bits)
  machine_intu_t);                     //ID, 0 - 4095 (12 bits)

void jcan_typed_start (                //init for sending type D JCAN data frame
  machine_intu_t,                      //N, 0 - 4095 (12 bits)
  machine_intu_t);                     //ID, 0 - 1023 (10 bits)

void can_send_dat (                    //add one data byte to CAN frame being built
  machine_intu_t);                     //0-255 byte value in low bits

void can_send_dat16 (                  //add two data bytes to CAN frame being built
  machine_intu_t);                     //bytes, written most to least significant order

void can_send_dat24 (                  //add three data bytes to CAN frame being built
  int32u_t);                           //bytes, written most to least significant order

void can_send_dat32 (                  //add four data bytes to CAN frame being built
  int32u_t);                           //bytes, written most to least significant order

void can_send (void);                  //send CAN frame, release CAN sending lock

void jcan_typea_req (                  //send a type A JCAN remote request frame
  machine_intu_t,                      //N, 0 - 3 (2 bits)
  int32u_t);                           //ID, 0 - 1,048,575 (20 bits)

void jcan_typeb_req (                  //send a type B JCAN remote request frame
  machine_intu_t,                      //N, 0 - 63 (6 bits)
  machine_intu_t);                     //ID, 0 - 65535 (16 bits)

void jcan_typec_req (                  //send a type C JCAN remote request frame
  machine_intu_t,                      //N, 0 - 1023 (10 bits)
  machine_intu_t);                     //ID, 0 - 4095 (12 bits)

void jcan_typed_req (                  //send a type D JCAN remote request frame
  machine_intu_t,                      //N, 0 - 4095 (12 bits)
  machine_intu_t);                     //ID, 0 - 1023 (10 bits)

//******************************************************************************
//
//   Interfaces specific to this system, not general library modules.
//
#define inactive_k (5 * 60 * evwaithz_k) //standard inactivity timeout, EVENT_WAIT units


//
//   Menus.  These all have a common naming scheme and interface.  The menus
//   are all named UMEN_xxx, where XXX gives some idea of the menu that the
//   routine implements.  The return value of a menu routine function is:
//
//     TRUE
//
//       The menu was terminated normally.  No exception condition applies.  The
//       user either made a selection, or cancelled the menu in the intended
//       way.
//
//       The purpose of this return value is to indicate that menu navigation
//       should continue normally.
//
//     FALSE
//
//       The user specifically indicated that the unit or the user interface
//       should be off, or there was no user activity within the timeout.
//
//       The purpose of this return value is to indicate that menu navigation
//       as a whole should be aborted.  A menu routine would normally return
//       with FALSE immediately when one of its subordinate menu routines
//       returns with FALSE.  This causes a menu system abort to propagate up
//       the nested levels of menus quickly, causing the top level menu routine
//       to return FALSE.
//
//       The caller of the top level menu usually handles system on/off issues,
//       turning off the backlight (if any), etc.
//
machine_intu_t umen_qqq (void);        //QQQ menu
