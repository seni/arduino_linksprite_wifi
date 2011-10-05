/* Linksprite UART-Wifi module, documented in "LinkSprite-UART-WiFi.pdf" */

#include <stdint.h>


/* Error message serial port */
#define PRINT_DEBUG      true
#define DEBUG_MESSAGE    Serial

/* Arduino serial port the LinkSprite module is connected to */
#define LinkSprite       Serial1

/* Send all frames with ack */
#define WITH_ACK         true


/* Common data frame */
#define DATA             ( 0x01 << 4 )

#define BSSID_LENGTH 6

/* SSIDs have a maximum length of 32 plus a tailing \0 */
#define SSID_LENGTH  33

/* The parameter information field has a maximum size of 64 */
/* for module versions 1.4 and below */
/* #define MAX_PARAMETER_INFO_LENGTH 64 */

/* The parameter information field has a maximum size of 54 */
#define MAX_PARAMETER_INFO_LENGTH 54


typedef struct _frame_header_t {

  uint8_t  type;
  uint16_t length;
  uint8_t  crc8;

} frame_header_t;


typedef struct _network_scan_result_t {

  /* index or strength value, depends on        */
  /* version of the module, version >= 1.53     */
  /* and <= 1.51 for an index of the newtork AP */
  uint8_t index_strength;
  uint8_t channel; /* channel 1 - 14 */
  uint8_t bssid [ BSSID_LENGTH ];
  uint8_t length; /* length of the ssid */
  uint8_t ssid  [ SSID_LENGTH ];

} network_scan_result_t;


typedef enum _parameter_id_t {

  BSSID                       = 0x01,
  Channel                     = 0x02,
  SSID                        = 0x03,
  EncryptionMethod            = 0x04,
  SecretKey                   = 0x05,
  LLDataFormat                = 0x06,
  ServicePort                 = 0x07,
  DeviceIPAddress             = 0x08,
  ServerIPAddress             = 0x09,
  ServerMACAddress            = 0x0a,
  SubnetMask                  = 0x0b,
  GateWayIPAddress            = 0x0c,
  /* 0x0d - 0xa0: Reserved Range */
  SerialType                  = 0xa1,
  SerialBaudRate              = 0xa2,
  BGMode                      = 0xa3,
  MaximumSpeed                = 0xa4,
  TransmissionPower           = 0xa5,
  DeviceMACAddress            = 0xa6,
  ConnectionMode              = 0xa7,
  AutomaticRetries            = 0xa8,
  TransparentTransmissionMode = 0xa9,
  TCPMonitorMode              = 0xaa
  /* 0xab - 0xff: Reserved Range */

} parameter_id_t;


typedef enum _group_id_t {

  DefaultGroup = 0xb0,
  Group1       = 0xb1,
  Group2       = 0xb2,
  Group3       = 0xb3,
  Group4       = 0xb4,
  Group5       = 0xb5,
  Group6       = 0xb6,
  Group7       = 0xb7

} group_id_t;


typedef struct _parameter_t {

  parameter_id_t id;
  uint8_t        length;
  uint8_t        info [ MAX_PARAMETER_INFO_LENGTH ];

} parameter_t;


/* Return values */
#define ERROR            0
#define SUCCESS          1


uint8_t send_frame ( uint8_t   control_type,
		     uint8_t  *data,
		     uint16_t  data_length,
		     bool      with_ack );

void increase_sequence_number ();

uint8_t reset_linksprite ();

uint8_t scan_network_linksprite ( uint16_t               portmask,
				  network_scan_result_t *results,
				  uint8_t               *num_result_entries );

uint8_t get_parameters_linksprite ( group_id_t      parameter_group,
				    parameter_id_t  parameter_id [],
				    parameter_t    *parameter,
				    uint8_t         num_parameter_ids );

uint8_t set_parameters_linksprite ( group_id_t  parameter_group,
				    parameter_t parameter [],
				    uint8_t     num_parameters );

uint8_t connect_to_network_linksprite ( group_id_t group_id );
