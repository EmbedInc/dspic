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
//   capabilities of the underlying system that are available to C code.
//
#include "qq2_machine.h"               //define machine-dependent data types
#include "qq2_debug.h"                 //define debug switches
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
//     machine_intptr_t  -  Unsigned integer that can hold a pointer
//
//   Pre-defined constants:
//
//     fwtype_k  -  Firmware type ID
//     fwver_k  -  1-N firmware version number within type
//     fwseq_k  -  1-N firmare build sequence number within type/version
//

void __attribute__((noreturn))
  system_reset (void);                 //reset the whole system

//******************************************************************************
//
//   Clock and general timing.
//
machine_intu_t clock_1ms (void);       //get 1 ms clock tick counter, 65.5 second range
machine_intu_t clock_10ms (void);      //get 10 ms clock tick counter, 10.9 minute range
machine_intu_t clock_100ms (void);     //get 100 ms clock tick counter, 1.8 hour range
int32u_t clock_seconds (void);         //seconds since powerup or reset, 136 year range
machine_intu_t clock_seconds16 (void); //faster simpler 16 bit seconds, 18.2 hour range

void waitms (                          //wait fixed time while letting other tasks run
  machine_intu_t);                     //time to wait in milliseconds

//******************************************************************************
//
//   Cooperative multi-tasking system.
//
typedef
  void (*task_function) (              //template for top routine of a task
    void *,                            //arbitrary arguments passed from TASK_NEW
    machine_intu_t);

//   Functions.
//
machine_intu_t                         //true or false
task_exist (                           //check whether a task exists
  machine_intu_t);                     //task ID

void __attribute__((noreturn))
task_exit (void);                      //end this task

machine_intu_t                         //task ID
task_id (void);                        //get ID of the current task

void task_kill (                       //end a specific task
  machine_intu_t);                     //ID of the task to end, ignored if invalid

machine_intu_t
task_n_curr (void);                    //get number of tasks that currently exist

machine_intu_t
task_n_max (void);                     //get maximum number of tasks supported

machine_intu_t                         //ID of the new task
task_new (                             //start a new task
  task_function,                       //function to run, task ends on return
  void *,                              //pointer to start of stack for new task
  machine_intu_t,                      //size of stack for new task, bytes
  void *,                              //arbitrary arguments passed to task routine
  machine_intu_t);

machine_intu_t                         //task ID, undefined for out of range slot number
task_nid (                             //get ID of task in a particular slot
  machine_intu_t);                     //0 to TASK_N_CURR-1 task slot number

void task_prune (                      //kill all but the first N tasks
  machine_intu_t);                     //number of tasks to leave, 0 resets processor

void task_yield (void);                //let other tasks run for a while

//******************************************************************************
//
//   Dynamic memory allocation and deallocation.
//
void *                                 //pointer to start of the new memory
dymem_alloc_perm (                     //permanently alloc mem from heap, no dealloc
  int16u_t);                           //size of new memory to allocate, bytes

void *                                 //pointer to start of the new memory
dymem_alloc_temp (                     //allocate new mem from heap, can dealloc
  int16u_t);                           //size of new memory to allocate, bytes

void dymem_dealloc (                   //deallocate temporarily allocated memory
  void *);                             //pointer to anywhere in region to dealloc

//******************************************************************************
//
//   Subsystem configuration.
//
machine_intu_t                         //TRUE/FALSE
config_failed (void);                  //find whether overall configuration failed

void config_lock (void);               //get exclusive access to CONFIG routines

void config_stat (                     //send response for event or status of a subsystem
  machine_intu_t,                      //0-254 subsystem ID
  machine_intu_t);                     //0-255 status code, 128-255 indicate failure

//  The CONFIG lock must be held when calling any CONFIG_xxx routine listed
//  below here.
//
machine_intu_t                         //number of config bytes
config_setup (                         //set up for reading config of a subsystem
  machine_intu_t);                     //subsystem ID

machine_intu_t config_i8u (void);      //get next config bytes in various formats
machine_ints_t config_i8s (void);
machine_intu_t config_i16u (void);
machine_ints_t config_i16s (void);
int32u_t config_i24u (void);
int32s_t config_i24s (void);
int32u_t config_i32u (void);
int32s_t config_i32s (void);
float config_fp32 (void);

void config_skip (                     //skip over config bytes
  machine_intu_t);                     //number of bytes to skip

machine_intu_t                         //0-N byte index into config bytes
config_index (void);                   //get index of next config byte

void config_index_set (                //set index of next config byte to read
  machine_intu_t);                     //new index

int32u_t config_adr (void);            //non-volatile mem adr of next config byte

machine_intu_t                         //N config bytes at current index to end
config_left (void);                    //get number of config bytes left to read

void config_unlock (void);             //release exclusive lock on CONFIG routines

//******************************************************************************
//
//   Math routines.
//
typedef struct {                       //48 bit unsigned integer
  union {
    int16u_t w[3];                     //array of 16 bit words
    struct {
      int16u_t w0;                     //explicit W0-Wn names for each word
      int16u_t w1;
      union {
        int16u_t w2;
        int16u_t high;                 //special alias for the high word
        };
      };
    };
  } int48u_t;

typedef struct {                       //48 bit signed integer
  union {
    int16u_t w[3];                     //array of 16 bit words
    struct {
      int16u_t w0;                     //explicit W0-Wn names for each word
      int16u_t w1;
      union {
        int16s_t w2;
        int16s_t high;                 //special alias for the high word
        };
      };
    };
  } int48s_t;

typedef struct {                       //64 bit unsigned integer
  union {
    int16u_t w[4];                     //array of 16 bit words
    struct {
      int16u_t w0;                     //explicit W0-Wn names for each word
      int16u_t w1;
      int16u_t w2;
      union {
        int16u_t w3;
        int16u_t high;                 //special alias for the high word
        };
      };
    };
  } int64u_t;

typedef struct {                       //64 bit signed integer
  union {
    int16u_t w[4];                     //array of 16 bit words
    struct {
      int16u_t w0;                     //explicit W0-Wn names for each word
      int16u_t w1;
      int16u_t w2;
      union {
        int16s_t w3;
        int16s_t high;                 //special alias for the high word
        };
      };
    };
  } int64s_t;

int32s_t                               //returned 32 bit signed fixed point number
fp32_fixs (                            //FLOAT to fixed point, rounded and saturated
  float,                               //floating point input value
  machine_ints_t);                     //number of fraction bits in result, may be negative

int32u_t                               //returned 32 bit unsigned fixed point number
fp32_fixu (                            //FLOAT to fixed point, rounded and saturated
  float,                               //floating point input value
  machine_ints_t);                     //number of fraction bits in result, may be negative

float                                  //resulting floating point value
fp32_flt32s (                          //32 bit signed fixed point to floating point
  int32s_t,                            //signed fixed point value
  machine_ints_t);                     //number of fraction bits, may be negative

float                                  //resulting floating point value
fp32_flt32u (                          //32 bit unsigned fixed point to floating point
  int32u_t,                            //unsigned fixed point value
  machine_ints_t);                     //number of fraction bits, may be negative

float                                  //resulting floating point value
fp32_flt48s (                          //48 bit signed fixed point to floating point
  int48s_t,                            //48 bit signed fixed point value
  machine_ints_t);                     //number of fraction bits, may be negative

float                                  //resulting floating point value
fp32_flt48u (                          //48 bit unsigned fixed point to floating point
  int48u_t,                            //48 bit unsigned fixed point value
  machine_ints_t);                     //number of fraction bits, may be negative

int32u_t                               //resulting 3.29 unsigned fixed point quotient
fx3f29_divu (                          //3.29 unsigned fixed point divide
  int32u_t,                            //3.29 unsigned fixed point numerator
  int32u_t);                           //3.29 unsigned fixed point denominator

int32u_t                               //resulting 3.29 unsigned fixed point product
fx3f29_mulu (                          //3.29 unsigned fixed point multiply
  int32u_t,                            //3.29 unsigned fixed point term
  int32u_t);                           //3.29 unsigned fixed point term

int32u_t                               //result
int16_muluu (                          //16 x 16 --> 32 bit multiply, all unsigned
  int16u_t, int16u_t);                 //the two operands

int32s_t                               //result
int16_mulss (                          //16 x 16 --> 32 bit multiply, all signed
  int16s_t, int16s_t);                 //the two operands

void int32_muluu (                     //32 x 32 --> 64 bit multiply, all unsigned
  int32u_t, int32u_t,                  //the two operands
  int64u_t *);                         //returned result

void int32_mulss (                     //32 x 32 --> 64 bit multiply, all signed
  int32s_t, int32s_t,                  //the two operands
  int64s_t *);                         //returned result

void int32u_acc48u (                   //add 32 bit integer into 48 bit accumulator, unsigned
  int32u_t,                            //value to add
  int48u_t *);                         //accumulator to add it into

void int32s_acc48s (                   //add 32 bit integer into 48 bit accumulator, signed
  int32s_t,                            //value to add
  int48s_t *);                         //accumulator to add it into

//******************************************************************************
//
//   Interface to Modbus-capable UART.
//
//   The routine names defined here are generic.  The set of routines may have
//   a string added to their names to distinguish multiple UARTs in the system.
//   If so, the templates here must be edited before being copied into the .h
//   file for a project.  The optional name is inserted after UART_, with
//   another "_" following.  For example, if the unique name is XYZZ, then
//   routine UART_LOCK would really be named UART_XYZZ_LOCK.
//
#define uart_ev_err_k 0x8000           //hard error: overrun, framing, etc
#define uart_ev_perr_k 0x4000          //parity error, data in low byte
#define uart_ev_pack_k 0x2000          //start of packet break, no character

machine_intu_t                         //data in low byte + UART_EV_xxx_K flags
uart_get (void);                       //get next event

machine_intu_t                         //TRUE or FALSE
uart_get_ready (void);                 //find if event is immediately available

void uart_lock (void);                 //acquire exclusive lock for sending

void uart_put (                        //send character
  machine_intu_t);                     //char in low byte, upper byte ignored

void uart_unlock (void);               //release sending lock

void uart_wait_send (void);            //guarantee packet break before next PUT

//******************************************************************************
//
//   Interface for sending and receiving complete Modbus packets.  These
//   routines are layered on the Mobus-capable UART, defined above.
//
//   The routine names defined here are generic.  The set of routines may have
//   a string added to their names to distinguish multiple Modbus I/O modules in
//   the system.  If so, the templates here must be edited before being copied
//   into the .h file for a project.  The optional name is inserted after
//   MODBUS_, with another "_" following.  For example, if the unique name is
//   XYZZ, then routine MODBUS_PACK_GET would really be named
//   MODBUS_XYZZ_PACK_GET.
//
typedef struct {
  int32u_t recv_total;                 //total received packets
  int32u_t recv_broadcast;             //broadcast packets received (slave only)
  int32u_t recv_mine;                  //packets received for this address (slave only)
  int32u_t err_crc;                    //packets rejected due to CRC error
  int32u_t err_parity;                 //number of bytes received with parity error
  int32u_t err_short;                  //number of packets with too few bytes
  int32u_t err_long;                   //number of packets with too many bytes
  int32u_t err_other;                  //framing errors, overrun errors, etc
  int32u_t send_total;                 //total number of packets sent
  } modbus_stats_t;

//********
//
//   These routines would typically be called during module configuration.
//
//   The master/slave status, start character, and slave address can be changed
//   during normal operation.
//
//   The Modbus receiving task must only be started if the module is enabled.
//   This is what defines whether the module is "on" or not.  The task can be
//   stopped and restarted during normal operation, but there MUST never be
//   more than one receiving task running per Modbus module.  Calling
//   MODBUS_START when the task is already running can cause all manner of
//   destruction.
//
void modbus_adr_set (                  //make slave, set slave address
  machine_intu_t);                     //1-247 adr, 0 = none (recv broadcast only)

void modbus_master (void);             //make this device the Modbus master

void modbus_startchar (                //special char preceeds all Modubs packets
  machine_intu_t);                     //special character in low byte

void modbus_startchar_off (void);      //no special start char, default

void modbus_start (void);              //start Modbus reception task

//********
//
//   Routines intended for use during normal operation.
//
//   Modbus packets are received and verified by the Modbus reception task.
//   Fully validated packets are receive by the application by calling
//   MODBUS_PACK_GET.  Packets are double-buffered internally, so another
//   packet can be received while the application is processing the previous.
//   When done with a packet, the application MUST call MODBUS_PACK_RELEASE.
//
machine_intu_t                         //number of data bytes, 0 on timeout
modbus_pack_get (                      //get next received Modbus packet
  machine_intu_t,                      //max milliseconds to wait
  const int8u_t * *,                   //packet start, NIL on timeout
  machine_intu_t *);                   //Modbus serial address

void modbus_pack_release (void);       //release last received Modbus packet

void modbus_pack_put (                 //send Modbus packet, multi-task safe
  machine_intu_t,                      //address, only used if master
  int8u_t *,                           //data bytes address, no adr or checksum
  machine_intu_t);                     //1-253 number of data bytes

void modbus_stats_get (                //get current snapshot of statistics counters
  modbus_stats_t *,                    //adr of structure to receive snapshot
  machine_intu_t);                     //reset stats to 0 after copy on TRUE

void modbus_stats_clear (void);        //reset all statistics counters to 0

//******************************************************************************
//
//   Host command and response stream access.
//
//   A constant named rsp_xxx_k is defined for each response opcode, where xxx
//   is the response name.
//
machine_intu_t cmd_get8u (void);       //get command stream integer values
machine_ints_t cmd_get8s (void);
machine_intu_t cmd_get16u (void);
machine_ints_t cmd_get16s (void);
int32u_t cmd_get24u (void);
int32s_t cmd_get24s (void);
int32u_t cmd_get32u (void);
int32s_t cmd_get32s (void);

void cmd_lock_out (void);              //acquire exclusive lock on response stream

void cmd_unlock_out (void);            //release lock on response stream

void cmd_put8u (machine_intu_t);       //write integers to response stream
void cmd_put8s (machine_ints_t);
void cmd_put16u (machine_intu_t);
void cmd_put16s (machine_ints_t);
void cmd_put24u (int32u_t);
void cmd_put24s (int32s_t);
void cmd_put32u (int32u_t);
void cmd_put32s (int32s_t);

//   DEBUGn responses with IDs 0-127 are always sent.  DEBUGn response with IDs
//   128-255 are only sent when these are globally enabled with the DEBUG
//   command.  These are initialized to disabled at startup.
//
void debug0 (                          //send DEBUG0 response to host
  machine_intu_t);                     //ID

void debug1s (                         //send DEBUG1 response to host
  machine_intu_t,                      //ID
  machine_ints_t);                     //8 bit signed data

void debug1u (                         //send DEBUG1 response to host
  machine_intu_t,                      //ID
  machine_intu_t);                     //8 bit unsigned data

void debug2s (                         //send DEBUG2 response to host
  machine_intu_t,                      //ID
  int16s_t);                           //16 bit signed data

void debug2u (                         //send DEBUG2 response to host
  machine_intu_t,                      //ID
  int16u_t);                           //16 bit unsigned data

void debug3s (                         //send DEBUG3 response to host
  machine_intu_t,                      //ID
  int32s_t);                           //24 bit signed data

void debug3u (                         //send DEBUG3 response to host
  machine_intu_t,                      //ID
  int32u_t);                           //24 bit unsigned data

void debug4s (                         //send DEBUG4 response to host
  machine_intu_t,                      //ID
  int32s_t);                           //32 bit signed data

void debug4u (                         //send DEBUG4 response to host
  machine_intu_t,                      //ID
  int32u_t);                           //32 bit unsigned data

//******************************************************************************
//
//   Byte buffer interface.  These routines facilitate reading and writing bytes
//   and words to/from a buffer.  These kinds of buffers are used as low level
//   facilities by various sub-systems.
//
machine_intu_t                         //0-255 byte value
buf_get8u (                            //get next byte value from buffer
  int8u_t * *,                         //pointer to buffer byte, updated
  machine_intu_t *);                   //bytes left to read from buffer, updated

machine_ints_t                         //-128 to +127 byte value
buf_get8s (                            //get next byte value from buffer
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
//
//   Cached changed data may be automatically written to the physical memory
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

//   Interface to higher level non-volatile memory management.  These functions
//   are layered on the low level functions, above.
//
void nvmem_cksum_set (void);           //set checksum to current app content

int32u_t nvmem_last (void);            //get adr of last application-usable byte

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
//     event_app_k  -  application-caused event
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

void event_set (                       //explicitly set a event condition
  machine_intu_t);                     //EVENT_xxx_K event ID

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

void disp_column (                     //set column without altering line
  machine_intu_t);                     //0-N column number

void disp_line (                       //set line without altering column
  machine_intu_t);                     //0-N line number

void disp_scroll (void);               //scroll, if needed, to make curr line visible

void disp_char (                       //write character, advance char pos 1 right
  machine_intu_t);                     //character code in low 8 bits

machine_intu_t                         //returned 0-255 character code
disp_getchar (                         //get displayed character, blank past display
  machine_intu_t,                      //0-N column number
  machine_intu_t);                     //0-N line number

void disp_string (                     //write string to display
  const char *);                       //null-terminated string to write

void disp_crlf (void);                 //go to start of next line down

void disp_newline (void);              //to start of next line, scroll to make visible

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
//   CAN bus sending.
//
#define can_frtype_stddat_k 0          //standard data frame
#define can_frtype_extdat_k 1          //extended data frame
#define can_frtype_stdreq_k 2          //standard remote request frame
#define can_frtype_extreq_k 3          //extended remote request frame

void can_send_dat (                    //add one data byte to CAN frame being built
  machine_intu_t);                     //0-255 byte value in low bits

void can_send_dat16 (                  //add two data bytes to CAN frame being built
  machine_intu_t);                     //bytes, written most to least significant order

void can_send_dat24 (                  //add three data bytes to CAN frame being built
  int32u_t);                           //bytes, written most to least significant order

void can_send_dat32 (                  //add four data bytes to CAN frame being built
  int32u_t);                           //bytes, written most to least significant order

void can_send (void);                  //send CAN frame, release CAN sending lock

void can_send_init (                   //init for sending CAN frame, acquire sending lock
  machine_intu_t);                     //type of frame, use CAN_FRTYPE_xxx_K

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
