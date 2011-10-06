/* Linksprite UART-Wifi module, documented in "LinkSprite-UART-WiFi.pdf" */

#include <WProgram.h>

#include "linksprite_wifi.h"


/* Masking definitions */
#define HIGH_NIBBLE(x)   ( x & 0xf0 )
#define LOW_NIBBLE(x)    ( x & 0x0f )

/* UART synchronized field, start symbol */
#define SYN              0xAA

/* UART control field, type of data */
/* Frame has CRC and needs and ACK from the receiver */
#define A                0x80

/* Control data frame */
#define CONTROL          0x00

/* Common data frame */
/* #define DATA             ( 0x01 << 4 ) */

/* Acknowledgement frame */
#define ACK              ( 0x02 << 4 )

/* Timeouts and Delays */
#define INPUT_TIMEOUT    2048
#define SYN_TIMEOUT      32
#define INPUT_DELAY      1 /* millisecond */

/* Length of non-data frame fields */
#define HEADER_LENGTH    5
#define CRC_LENGTH       1
#define PADDING_LENGTH   6
#define FOOTER_LENGTH    PADDING_LENGTH

/* Command types */
#define COMMAND_SCAN_NETWORK            0x00
#define COMMAND_CONNECT_TO_NETWORK      0x01
#define COMMAND_DISCONNECT_FROM_NETWORK 0x02
#define COMMAND_SET_PARAMETERS          0x03
#define COMMAND_GET_PARAMETERS          0x04
#define COMMAND_RESET                   0x05

/* Message types */
#define MESSAGE_SCAN_NETWORK_RESULT     0x40
#define MESSAGE_CONNECT_TO_NETWORK      0x41
#define MESSAGE_DISCONNECT_FROM_NETWORK 0x42
#define MESSAGE_SET_PARAMETERS          0x43
#define MESSAGE_GET_PARAMETERS          0x44
#define MESSAGE_FINISHED_INIT           0x45

/* Crc-8 lookup table */
uint8_t crc8_table [] =
{ 0x00,0x91,0xe3,0x72,0x07,0x96,0xe4,0x75,0x0e,0x9f,0xed,0x7c,0x09,0x98,0xea,0x7b,
  0x1c,0x8d,0xff,0x6e,0x1b,0x8a,0xf8,0x69,0x12,0x83,0xf1,0x60,0x15,0x84,0xf6,0x67,
  0x38,0xa9,0xdb,0x4a,0x3f,0xae,0xdc,0x4d,0x36,0xa7,0xd5,0x44,0x31,0xa0,0xd2,0x43,
  0x24,0xb5,0xc7,0x56,0x23,0xb2,0xc0,0x51,0x2a,0xbb,0xc9,0x58,0x2d,0xbc,0xce,0x5f,
  0x70,0xe1,0x93,0x02,0x77,0xe6,0x94,0x05,0x7e,0xef,0x9d,0x0c,0x79,0xe8,0x9a,0x0b,
  0x6c,0xfd,0x8f,0x1e,0x6b,0xfa,0x88,0x19,0x62,0xf3,0x81,0x10,0x65,0xf4,0x86,0x17,
  0x48,0xd9,0xab,0x3a,0x4f,0xde,0xac,0x3d,0x46,0xd7,0xa5,0x34,0x41,0xd0,0xa2,0x33,
  0x54,0xc5,0xb7,0x26,0x53,0xc2,0xb0,0x21,0x5a,0xcb,0xb9,0x28,0x5d,0xcc,0xbe,0x2f,
  0xe0,0x71,0x03,0x92,0xe7,0x76,0x04,0x95,0xee,0x7f,0x0d,0x9c,0xe9,0x78,0x0a,0x9b,
  0xfc,0x6d,0x1f,0x8e,0xfb,0x6a,0x18,0x89,0xf2,0x63,0x11,0x80,0xf5,0x64,0x16,0x87,
  0xd8,0x49,0x3b,0xaa,0xdf,0x4e,0x3c,0xad,0xd6,0x47,0x35,0xa4,0xd1,0x40,0x32,0xa3,
  0xc4,0x55,0x27,0xb6,0xc3,0x52,0x20,0xb1,0xca,0x5b,0x29,0xb8,0xcd,0x5c,0x2e,0xbf,
  0x90,0x01,0x73,0xe2,0x97,0x06,0x74,0xe5,0x9e,0x0f,0x7d,0xec,0x99,0x08,0x7a,0xeb,
  0x8c,0x1d,0x6f,0xfe,0x8b,0x1a,0x68,0xf9,0x82,0x13,0x61,0xf0,0x85,0x14,0x66,0xf7,
  0xa8,0x39,0x4b,0xda,0xaf,0x3e,0x4c,0xdd,0xa6,0x37,0x45,0xd4,0xa1,0x30,0x42,0xd3,
  0xb4,0x25,0x57,0xc6,0xb3,0x22,0x50,0xc1,0xba,0x2b,0x59,0xc8,0xbd,0x2c,0x5e,0xcf
};


uint8_t update_crc8 ( uint8_t data, uint8_t crc8 )
{

  return crc8_table [ data ^ crc8 ];

}


uint8_t get_crc8 ( uint8_t *data, uint16_t length )
{

  uint16_t crc8;

  crc8 = 0;

  while( length-- )
    crc8 = update_crc8 ( *data++, crc8 );

  return crc8;

}


bool check_crc8 ( uint8_t *data, uint16_t length )
{

  return get_crc8 ( data, length ) == 0;

}


static uint8_t sequence_number = 0;

void increase_sequence_number ()
{

  sequence_number = ( sequence_number + 1 ) % 16;

}


int8_t wait_for_input ()
{

  uint16_t input_timeout = INPUT_TIMEOUT;

  while ( !LinkSprite.available () ) {

    if ( !input_timeout-- ) {
      if ( PRINT_DEBUG )
        DEBUG_MESSAGE.println ( "wait_for_input: input timeout." );
      return ERROR;
    }

    delay ( INPUT_DELAY );

  }

  return SUCCESS;

}


uint8_t receive_frame_header ( frame_header_t *frame_header )
{

  uint8_t syn_timeout = SYN_TIMEOUT;
  uint8_t frame_header_input [ 4 ];

  do {

    if ( wait_for_input () == ERROR )
      return ERROR;

    if ( !syn_timeout-- ) {
      if ( PRINT_DEBUG )
        DEBUG_MESSAGE.println ( "receive_frame_header: SYN timeout." );
      return ERROR;
    }

  } while ( LinkSprite.read () != SYN );


  /* Frame type */
  if ( wait_for_input () == ERROR )
    return ERROR;

  frame_header_input [ 0 ] = LinkSprite.read ();
  frame_header->type = frame_header_input [ 0 ];

  /* Check sequence number */
  if ( LOW_NIBBLE ( frame_header->type ) != sequence_number ) {
    if ( PRINT_DEBUG ) {
      DEBUG_MESSAGE.print   ( "receive_frame_header: wrong sequence number, " );
      DEBUG_MESSAGE.print   ( "expected: " );
      DEBUG_MESSAGE.print   ( sequence_number, DEC );
      DEBUG_MESSAGE.print   ( ", received: " );
      DEBUG_MESSAGE.print   ( LOW_NIBBLE ( frame_header->type ), DEC );
      DEBUG_MESSAGE.println ( "." );
    }
    return ERROR;
  }

  /* High length byte */
  if ( wait_for_input () == ERROR )
    return ERROR;

  frame_header_input [ 1 ] = LinkSprite.read ();
  frame_header->length = frame_header_input [ 1 ] << 8;

  /* Low length byte */
  if ( wait_for_input () == ERROR )
    return ERROR;

  frame_header_input [ 2 ] = LinkSprite.read ();
  frame_header->length|= frame_header_input [ 2 ];

  /* Frame header crc-8 and check */
  if ( wait_for_input () == ERROR )
    return ERROR;

  frame_header_input [ 3 ] = LinkSprite.read ();
  frame_header->crc8 = frame_header_input [ 3 ];

  if ( !check_crc8 ( frame_header_input, 4 ) ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "receive_frame_header: frame header crc-8 check failed." );
    return ERROR;
  }

  return SUCCESS;

}


uint8_t receive_padding_bytes ()
{

  uint8_t input;

  /* Read in padding bytes */
  for ( uint8_t padding_count = 0;
        padding_count < PADDING_LENGTH;
        padding_count++ ) {

    /* *** next input *** */

    if ( wait_for_input () == ERROR )
      return ERROR;

    input = LinkSprite.read ();

    /* Input must be zero */
    if ( input ) {
      if ( PRINT_DEBUG ) {
        DEBUG_MESSAGE.print   ( "receive_ack_frame: padding byte " );
        DEBUG_MESSAGE.print   ( padding_count, DEC );
        DEBUG_MESSAGE.println ( " is not zero." );
      }
      return ERROR;
    }

  }

  return SUCCESS;

}


uint8_t receive_ack_frame ()
{

  frame_header_t frame_header;

  /* Get frame header */
  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  /* Check for ACK control field */
  if ( HIGH_NIBBLE ( frame_header.type ) != ACK ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println ( "receive_ack_frame: no ACK after SYN." );
    return ERROR;
  }

  /* Check that length == zero */
  if ( frame_header.length ) {
    if ( PRINT_DEBUG ) {
      DEBUG_MESSAGE.print ( "receive_ack_frame: length not zero:" );
      DEBUG_MESSAGE.println ( frame_header.length, DEC );
    }
    return ERROR;
  }

  if ( !receive_padding_bytes () )
    return ERROR;

  return SUCCESS;

}


uint8_t send_frame ( uint8_t   control_type,
                     uint8_t  *data,
                     uint16_t  data_length,
                     bool      with_ack )
{

  uint8_t  frame_header [ HEADER_LENGTH ];
  /* Initialize all footer elements to 0 (i.e. padding bytes are 0) */
  uint8_t  frame_footer [ FOOTER_LENGTH ] = { 0x00 };
  uint16_t all_data_length = data_length;
  uint8_t  control_field = 0;

  if ( with_ack )
    control_field |= A;
  control_field |= control_type;
  control_field |= sequence_number;

  if ( with_ack )
    all_data_length+= CRC_LENGTH;

  frame_header [ 0 ] = SYN;
  frame_header [ 1 ] = control_field;
  frame_header [ 2 ] = highByte ( all_data_length ); /* High byte of length */
  frame_header [ 3 ] = lowByte  ( all_data_length ); /* Low byte of length */
  /* CHK: crc-8 of the frame header (control field and data length) */
  frame_header [ 4 ] = get_crc8 ( &frame_header [ 1 ], 3 );

  /* Write frame header */
  LinkSprite.write ( frame_header, HEADER_LENGTH );

  /* Write frame data */
  LinkSprite.write ( data, data_length );

  if ( with_ack ) {
    /* Write crc-8 */
    uint8_t crc_8 = get_crc8 ( data, data_length );
    LinkSprite.write ( &crc_8, 1 );
  }

  /* Write frame footer, i.e. zero padding */
  LinkSprite.write ( frame_footer, FOOTER_LENGTH );

  /* Check ACK frame */
  if ( with_ack && ( receive_ack_frame () == ERROR ) )
    return ERROR;

  return SUCCESS;

}


uint8_t reset_linksprite ()
{

  #define RESET_CONTROL_LENGTH 1
  /* Reset command */
  uint8_t reset_control [] = { COMMAND_RESET };
  frame_header_t frame_header;
  uint8_t input;

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "resetting module" );

  sequence_number = 0;
  LinkSprite.flush ();

  if ( send_frame ( CONTROL,
                    reset_control,
                    RESET_CONTROL_LENGTH,
                    WITH_ACK ) == ERROR )
    return ERROR;

  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  if ( frame_header.length < 3 ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "reset_linksprite: length of reset information smaller than 3" );
    return ERROR;
  }

  /* Finish of initialization flag */
  if ( wait_for_input () == ERROR )
    return ERROR;

  input = LinkSprite.read ();

  if ( input != MESSAGE_FINISHED_INIT ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "reset_linksprite: message flag received is not 0x45" );
    return ERROR;
  }

  /* Reset type */
  if ( wait_for_input () == ERROR )
    return ERROR;

  input = LinkSprite.read ();

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print ( "reset type: " );
    if ( input == 0x00 )
      DEBUG_MESSAGE.println ( "hardware" );
    else if ( input == 0x01 )
      DEBUG_MESSAGE.println ( "software" );
    else
      DEBUG_MESSAGE.println ( "other" );
  }

  /* Version */
  if ( wait_for_input () == ERROR )
    return ERROR;

  input = LinkSprite.read ();

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print ( "version: 0x" );
    DEBUG_MESSAGE.println ( input, HEX );
  }

  /* The initialization answer from the module does not append a valid crc,
     so we do not need to check for that */
  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.print ( "reset information: " );

  for ( uint16_t input_count = 3;
        input_count < frame_header.length;
        input_count++ ) {

    if ( wait_for_input () == ERROR )
    return ERROR;

    input = LinkSprite.read ();

    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.write ( input );

  }

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "" );

  if ( !receive_padding_bytes () )
    return ERROR;

  increase_sequence_number ();

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print   ( "sequence_number: " );
    DEBUG_MESSAGE.println ( sequence_number, DEC );
  }

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "resetting successful" );

  return SUCCESS;

}


uint8_t scan_network_linksprite ( uint16_t               portmask,
                                  network_scan_result_t *results,
                                  uint8_t               *num_result_entries )
{

  #define SCAN_NETWORK_CONTROL_LENGTH 3
  /* Scan network command */
  uint8_t scan_network_control [] = { COMMAND_SCAN_NETWORK,
                                      highByte ( portmask ),
                                      lowByte  ( portmask ) };
  network_scan_result_t *result = results;
  frame_header_t frame_header;
  uint8_t input;
  uint8_t result_entries_count = *num_result_entries;
  bool finished;

  /* States of the result input */
  enum { state_flag,
         state_index_strength,
         state_channel,
         state_bssid,
         state_length,
         state_ssid } state;

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "scanning network" );

  /* No memory for result entries to be filled */
  if ( !result_entries_count )
    return SUCCESS;

  if ( send_frame ( CONTROL,
                    scan_network_control,
                    SCAN_NETWORK_CONTROL_LENGTH,
                    WITH_ACK ) == ERROR )
    return ERROR;

  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  state = state_flag;
  for ( uint16_t i = 0, finished = false;
        i < frame_header.length; i++ ) {

    uint8_t state_bssid_count, state_ssid_count;

    if ( wait_for_input () == ERROR )
      return ERROR;

    input = LinkSprite.read ();

    if ( !finished ) /* I.e. There are more entries to be stored,
                        else skip over them */
      switch ( state ) {

      case state_flag:
        if ( input != MESSAGE_SCAN_NETWORK_RESULT ) {
          if ( PRINT_DEBUG ) {
            DEBUG_MESSAGE.print ( "scan_network_control_length: wrong flag, " );
            DEBUG_MESSAGE.println ( input, HEX );
          }
          return ERROR;
        }
        state = state_index_strength;
        break;

      case state_index_strength:
        result->index_strength = input;
        state = state_channel;
        break;

      case state_channel:
        if ( ( input < 1 ) || ( input > 14 ) ) {
          if ( PRINT_DEBUG ) {
            DEBUG_MESSAGE.print
              ( "scan_network_linksprite: channel " );
            DEBUG_MESSAGE.print ( input, DEC );
            DEBUG_MESSAGE.print ( " out of bounds" );
          }
          return ERROR;
        }
        result->channel = input;
        state = state_bssid;
        state_bssid_count = 0;
        break;

      case state_bssid:
        result->bssid [ state_bssid_count ] = input;
        if ( ++state_bssid_count == BSSID_LENGTH )
          state = state_length;
        break;

      case state_length:
        result->length = input;
        state = state_ssid;
        state_ssid_count = 0;
        break;

      case state_ssid:
        result->ssid [ state_ssid_count ] = input;
        if ( ++state_ssid_count == result->length ) {
          state = state_index_strength;
          result++;
          if ( !--result_entries_count )
            finished = true;
        }
        break;

      default:
        if ( PRINT_DEBUG )
          DEBUG_MESSAGE.println ( "scan_network_linksprite: should not happen" );
        return ERROR;

      }

  }

  increase_sequence_number ();

  /* Do not return an error on an incomplete dangling
     station, but return the number of complete stations
     and SUCCESS */
  if ( ( state != state_flag ) && ( state != state_index_strength ) )
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println ( "scan_network_linksprite: incomplete packet" );

  *num_result_entries-= result_entries_count;

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "scan_network_linksprite: scanning network successful" );

  return SUCCESS;

}



uint8_t get_parameters_linksprite ( group_id_t      parameter_group,
                                    parameter_id_t  parameter_id [],
                                    parameter_t    *parameter,
                                    uint8_t         num_parameter_ids )
{

  #define GET_PARAMETERS_CONTROL_LENGTH ( 2 + num_parameter_ids )
  /* Scan network command */
  uint8_t get_parameters_control [ GET_PARAMETERS_CONTROL_LENGTH ];
  frame_header_t frame_header;
  uint8_t input;
  uint8_t parameter_entries_count = num_parameter_ids;
  uint8_t parameter_info_count;
  bool finished;

  /* States of the result input */
  enum { state_flag,
         state_group,
         state_id,
         state_length,
         state_info } state;

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print   ( "getting parameters for group " );
    DEBUG_MESSAGE.println ( parameter_group, HEX );
  }

  /* No memory for parameter entries to be filled */
  if ( !parameter_entries_count )
    return SUCCESS;

  parameter_group;
  get_parameters_control [ 0 ] = COMMAND_GET_PARAMETERS;
  get_parameters_control [ 1 ] = parameter_group;

  for ( int i = 0; i < num_parameter_ids; i++ )
    get_parameters_control [ i + 2 ] = parameter_id [ i ];

  if ( send_frame ( CONTROL,
                    get_parameters_control,
                    GET_PARAMETERS_CONTROL_LENGTH,
                    WITH_ACK ) == ERROR )
    return ERROR;

  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  state = state_flag;
  for ( uint16_t i = 0, finished = false;
        i < frame_header.length; i++ ) {

    if ( wait_for_input () == ERROR )
      return ERROR;

    input = LinkSprite.read ();

    if ( !finished ) /* I.e. There are more entries to be stored,
                        else skip over them */
      switch ( state ) {

      case state_flag:
        if ( input != MESSAGE_GET_PARAMETERS ) {
          if ( PRINT_DEBUG ) {
            DEBUG_MESSAGE.print ( "get_parameters_linksprite: wrong flag, expected 0x44, got " );
            DEBUG_MESSAGE.println ( input, HEX );
          }
          return ERROR;
        }
        state = state_group;
        break;

      case state_group:
        if ( input != parameter_group ) {
          /* Print error messages including the group offset */
          if ( PRINT_DEBUG ) {
            DEBUG_MESSAGE.print
              ( "get_parameters_linksprite: wrong returned group id, expected: " );
            DEBUG_MESSAGE.print ( parameter_group, HEX );
            DEBUG_MESSAGE.print ( ", received: " );
            DEBUG_MESSAGE.println ( input, HEX );
          }
          return ERROR;
        }
        state = state_id;
        break;

      case state_id:
        parameter->id = ( parameter_id_t ) input;
        state = state_length;
        break;

      case state_length:
        parameter->length = input;
        parameter_info_count = 0;
        state = state_info;
        break;

      case state_info:
        parameter->info [ parameter_info_count ] = input;
        if ( ++parameter_info_count == parameter->length ) {
          state = state_id;
          parameter++;
          if ( !--parameter_entries_count )
            finished = true;
        }
        break;

      default:
        if ( PRINT_DEBUG )
          DEBUG_MESSAGE.println ( "get_parameters_linksprite: should not happen" );
        return ERROR;

      }

  }

  increase_sequence_number ();

  if ( state != state_id ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println ( "get_parameters_linksprite: incomplete packet" );
    return ERROR;
  }

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "getting parameters successful" );

  return SUCCESS;

}


uint8_t set_parameters_linksprite ( group_id_t  parameter_group,
                                    parameter_t parameter [],
                                    uint8_t     num_parameters )
{

  frame_header_t frame_header;
  uint8_t input;
  uint8_t parameter_size = 0;
  uint8_t set_parameters_control_size;

  if ( PRINT_DEBUG )
    DEBUG_MESSAGE.println ( "setting parameters" );

  for ( uint8_t i = 0; i < num_parameters; i++ )
    parameter_size+= parameter [ i ].length;

  set_parameters_control_size = 2 + 2*num_parameters + parameter_size;
  {

    /* Set parameters command */
    uint8_t set_parameters_control [ set_parameters_control_size ];
    uint8_t k = 2;

    set_parameters_control [ 0 ] = COMMAND_SET_PARAMETERS;
    set_parameters_control [ 1 ] = parameter_group;

    for ( uint8_t i = 0; i < num_parameters; i++ ) {

      set_parameters_control [ k++ ] = parameter [ i ].id;
      set_parameters_control [ k++ ] = parameter [ i ].length;

      for ( uint8_t j = 0; j < parameter [ i ].length; j++ )
        set_parameters_control [ k++ ] = parameter [ i ].info [ j ];

    }

    if ( send_frame ( CONTROL,
                      set_parameters_control,
                      set_parameters_control_size,
                      WITH_ACK ) == ERROR )
      return ERROR;

  }

  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  if ( frame_header.length != 2 ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "set_parameters_linksprite: length of returned message not equal 2" );
    return ERROR;
  }

  /* Check returned message */
  if ( wait_for_input () == ERROR )
    return ERROR;

  input = LinkSprite.read ();

  if ( input != MESSAGE_SET_PARAMETERS ) {
    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "set_parameters_linksprite: message flag received is not 0x43" );
    return ERROR;
  }

  /* Check returned status */
  if ( wait_for_input () == ERROR )
    return ERROR;

  input = LinkSprite.read ();

  increase_sequence_number ();

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print ( "set_parameters_linksprite returned status: " );
    if ( input == 0x00 )
      DEBUG_MESSAGE.println ( "success" );
    else {
      DEBUG_MESSAGE.println ( "failed" );
      return ERROR;
    }
  }

  return SUCCESS;

}


uint8_t connect_to_network_linksprite ( group_id_t group_id )
{

  #define CONNECT_TO_NETWORK_CONTROL_LENGTH 2
  /* Connect network command */
  uint8_t connect_to_network_control [] = { COMMAND_CONNECT_TO_NETWORK, group_id - 0xb0 };
  frame_header_t frame_header;
  uint8_t input;
  uint8_t return_value = SUCCESS;

  /* States of the result input */
  enum { state_flag,
         state_result,
         state_other } state;

  if ( PRINT_DEBUG ) {
    DEBUG_MESSAGE.print ( "connect_to_network_linksprite, group " );
    DEBUG_MESSAGE.println ( group_id - 0xb0, DEC );
  }

  if ( send_frame ( CONTROL,
                    connect_to_network_control,
                    CONNECT_TO_NETWORK_CONTROL_LENGTH,
                    WITH_ACK ) == ERROR )
    return ERROR;

  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  state = state_flag;
  for ( uint16_t i = 0; i < frame_header.length; i++ ) {

    if ( wait_for_input () == ERROR )
      return ERROR;

    input = LinkSprite.read ();

    switch ( state ) {

    case state_flag:
      if ( input != MESSAGE_CONNECT_TO_NETWORK ) {
        if ( PRINT_DEBUG ) {
          DEBUG_MESSAGE.print ( "connect_to_network_linksprite: wrong flag, expected 0x41, got " );
          DEBUG_MESSAGE.println ( input, HEX );
        }
        return ERROR;
      }
      state = state_result;
      break;

    case state_result:
      if ( input ) {
        if ( PRINT_DEBUG ) {
          DEBUG_MESSAGE.print ( "connect_to_network_linksprite: connection failed, result " );
          DEBUG_MESSAGE.println ( input, HEX );
        }
        return_value = ERROR;
      }
      state = state_other;
      break;

    case state_other:
      state = state_other;
      break;

    default:
      if ( PRINT_DEBUG )
        DEBUG_MESSAGE.println ( "connect_to_network_linksprite: should not happen" );
      return ERROR;

    }

  }

  if ( !receive_padding_bytes () )
    return ERROR;

  increase_sequence_number ();

  if ( PRINT_DEBUG && ( return_value == SUCCESS ) )
    DEBUG_MESSAGE.println ( "connect_to_network_linksprite successful" );

  return return_value;

}


uint8_t receive_frame_linksprite ( uint8_t *data, uint16_t *length )
{

  frame_header_t frame_header;
  uint8_t input;
  uint16_t i;

  /* Get frame header */
  if ( !receive_frame_header ( &frame_header ) )
    return ERROR;

  if ( frame_header.length > *length ) {

    if ( PRINT_DEBUG )
      DEBUG_MESSAGE.println
        ( "receive_frame: frame header length greater than available data length" );

    return ERROR;

  }

  for ( i = 0; i < frame_header.length; i++ ) {

    if ( wait_for_input () == ERROR )
      return ERROR;

    input = LinkSprite.read ();

    DEBUG_MESSAGE.println ( input, HEX );

    /* The module also sends MESSAGE_CONNECT_TO_NETWORK on disconnects, */
    /* could this be a firmware bug? */
    if ( ( i == 0 ) && ( ( input == MESSAGE_CONNECT_TO_NETWORK ) ||
			 ( input == MESSAGE_DISCONNECT_FROM_NETWORK ) ) ) {

      if ( wait_for_input () == ERROR )
        return ERROR;

      input = LinkSprite.read ();

      if ( PRINT_DEBUG ) {

        DEBUG_MESSAGE.print   ( "receive_frame: disconnect with status " );

        if ( !input )
          DEBUG_MESSAGE.println ( "0x00 - normal" );
        else {
          DEBUG_MESSAGE.print   ( input, HEX );
          DEBUG_MESSAGE.println ( " - abnormal" );
        }

        increase_sequence_number ();

        return DISCONNECT;

      }

    } else {

      data [ i ] = input;

    }

  }

  *length = i;

  increase_sequence_number ();

  return SUCCESS;

}
