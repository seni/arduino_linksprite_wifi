#include <linksprite_wifi.h>


void setup ()
{

  Serial.begin  ( 115200 );
  Serial1.begin ( 115200 );

  while ( reset_linksprite () == ERROR );

}


void show_parameters ()
{

  #define NUM_NETWORK_PARAMETERS 11
  #define NUM_SYSTEM_PARAMETERS  10

  parameter_id_t system_parameter_ids [ NUM_SYSTEM_PARAMETERS ] =
    { SerialType, SerialBaudRate, BGMode, MaximumSpeed,
      TransmissionPower, DeviceMACAddress, AutomaticRetries,
      ConnectionMode, TransparentTransmissionMode, TCPMonitorMode };
  parameter_t system_parameters [ NUM_SYSTEM_PARAMETERS ];

  parameter_id_t network_parameter_ids [ NUM_NETWORK_PARAMETERS ] =
    { BSSID, Channel, SSID, EncryptionMethod, LLDataFormat, ServicePort,
      DeviceIPAddress, ServerIPAddress, ServerMACAddress, SubnetMask,
      GateWayIPAddress };
  parameter_t network_parameters [ NUM_NETWORK_PARAMETERS ];


  DEBUG_MESSAGE.println ( "Network parameters:" );
  for ( uint8_t group = DefaultGroup; group <= Group7; group++ ) {

    get_parameters_linksprite ( ( group_id_t ) group,
				network_parameter_ids,
				network_parameters,
				NUM_NETWORK_PARAMETERS );

    if ( group == DefaultGroup )
      DEBUG_MESSAGE.println ( "Default Group:" );
    else {
      DEBUG_MESSAGE.print   ( "Group " );
      DEBUG_MESSAGE.print   ( group - 0xb0 );
      DEBUG_MESSAGE.println ( ": " );
    }

    for ( uint8_t i = 0; i < NUM_NETWORK_PARAMETERS ; i++ ) {

      switch ( network_parameters [ i ].id ) {
    	case BSSID:
    	  DEBUG_MESSAGE.print   ( "BSSID: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ ) {
	    uint8_t value = network_parameters [ i ].info [ j ];
	    if ( value < 0x10 )
	      DEBUG_MESSAGE.print ( "0" );
    	    DEBUG_MESSAGE.print ( value, HEX );
    	    if ( j < network_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( ":" );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case Channel:
	  DEBUG_MESSAGE.print    ( "Channel: " );
	  DEBUG_MESSAGE.println  ( network_parameters [ i ].info [ 0 ], DEC );
    	  break;

    	case SSID:
    	  DEBUG_MESSAGE.print   ( "SSID: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ )
    	    DEBUG_MESSAGE.print ( network_parameters [ i ].info [ j ] );
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case EncryptionMethod:
	  DEBUG_MESSAGE.print ( "Encryption method: " );
	  switch ( network_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "Open" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "WEP" );
	    break;
	  case 2:
	    DEBUG_MESSAGE.println ( "WPI" );
	    break;
	  case 3:
	    DEBUG_MESSAGE.println ( "CCMP" );
	    break;
	  default:
	    DEBUG_MESSAGE.print ( "Reserved value: " );
	    DEBUG_MESSAGE.println ( network_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case LLDataFormat:
	  DEBUG_MESSAGE.print ( "Protocol: " );
	  switch ( network_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "RAW" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "UDP" );
	    break;
	  case 2:
	    DEBUG_MESSAGE.println ( "TCP" );
	    break;
	  default:
	    DEBUG_MESSAGE.print ( "Unknown value: " );
	    DEBUG_MESSAGE.println ( network_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case ServicePort:
	  DEBUG_MESSAGE.print ( "Service port: " );
	  DEBUG_MESSAGE.println ( network_parameters [ i ].info [ 0 ] * 0x100 +
				  network_parameters [ i ].info [ 1 ], DEC );
    	  break;

    	case DeviceIPAddress:
    	  DEBUG_MESSAGE.print ( "Device IP address: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ ) {
	    uint8_t value = network_parameters [ i ].info [ j ];
    	    DEBUG_MESSAGE.print ( value, DEC );
    	    if ( j < network_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( "." );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case ServerIPAddress:
    	  DEBUG_MESSAGE.print ( "Server IP address: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ ) {
	    uint8_t value = network_parameters [ i ].info [ j ];
    	    DEBUG_MESSAGE.print ( value, DEC );
    	    if ( j < network_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( "." );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case ServerMACAddress:
    	  DEBUG_MESSAGE.print   ( "Server MAC address: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ ) {
	    uint8_t value = network_parameters [ i ].info [ j ];
	    if ( value < 0x10 )
	      DEBUG_MESSAGE.print ( "0" );
    	    DEBUG_MESSAGE.print ( value, HEX );
    	    if ( j < network_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( ":" );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case SubnetMask:
	  DEBUG_MESSAGE.print ( "Subnet mask: " );
	  switch ( network_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "not set" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "255.255.255.0" );
	    break;
	  case 2:
	    DEBUG_MESSAGE.println ( "255.255.0.0" );
	    break;
	  case 3:
	    DEBUG_MESSAGE.println ( "255.0.0.0" );
	    break;
	  default :
	    DEBUG_MESSAGE.print   ( "Reserved value: " );
	    DEBUG_MESSAGE.println ( network_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case GateWayIPAddress:
    	  DEBUG_MESSAGE.print ( "Gateway IP address: " );
    	  for ( uint8_t j = 0; j < network_parameters [ i ].length; j++ ) {
	    uint8_t value = network_parameters [ i ].info [ j ];
    	    DEBUG_MESSAGE.print ( value, DEC );
    	    if ( j < network_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( "." );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	default:
    	  DEBUG_MESSAGE.println ( "show_parameters: should not happen" );
    	  return;

    	};

    }

    DEBUG_MESSAGE.println ( "" );

  }


  DEBUG_MESSAGE.println ( "System parameters:" );
  get_parameters_linksprite ( DefaultGroup, /* group is not relevant for
					       getting system parameters */
			      system_parameter_ids,
			      system_parameters,
			      NUM_SYSTEM_PARAMETERS );

  for ( uint8_t i = 0; i < NUM_SYSTEM_PARAMETERS ; i++ ) {

    switch ( system_parameters [ i ].id ) {

    	case SerialType:
    	  DEBUG_MESSAGE.print ( "Serial type: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    DEBUG_MESSAGE.print ( system_parameters [ i ].info [ j ], HEX );
	    if ( j < system_parameters [ i ].length - 1 )
	      DEBUG_MESSAGE.print ( ", " );
	  }
	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case SerialBaudRate:
	  DEBUG_MESSAGE.print ( "Serial baud rate: " );
	  switch ( system_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "19200" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "38400" );
	    break;
	  case 2:
	    DEBUG_MESSAGE.println ( "57600" );
	    break;
	  case 3:
	    DEBUG_MESSAGE.println ( "115200" );
	    break;
	  default:
	    DEBUG_MESSAGE.print   ( "Unknown value: " );
	    DEBUG_MESSAGE.println ( system_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case BGMode:
	  DEBUG_MESSAGE.print   ( "B/G Mode: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    DEBUG_MESSAGE.print ( system_parameters [ i ].info [ j ], HEX );
	    if ( j < system_parameters [ i ].length - 1 )
	      DEBUG_MESSAGE.print ( ", " );
	  }
	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case MaximumSpeed:
	  DEBUG_MESSAGE.print   ( "Maximum speed: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    DEBUG_MESSAGE.print ( system_parameters [ i ].info [ j ], HEX );
	    if ( j < system_parameters [ i ].length - 1 )
	      DEBUG_MESSAGE.print ( ", " );
	  }
	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case TransmissionPower:
	  DEBUG_MESSAGE.print   ( "Transmission power: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    DEBUG_MESSAGE.print ( system_parameters [ i ].info [ j ], HEX );
	    if ( j < system_parameters [ i ].length - 1 )
	      DEBUG_MESSAGE.print ( ", " );
	  }
	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case DeviceMACAddress:
    	  DEBUG_MESSAGE.print   ( "Device MAC address: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    uint8_t value = system_parameters [ i ].info [ j ];
	    if ( value < 0x10 )
	      DEBUG_MESSAGE.print ( "0" );
    	    DEBUG_MESSAGE.print ( value, HEX );
    	    if ( j < system_parameters [ i ].length - 1 )
    	      DEBUG_MESSAGE.print ( ":" );
    	  }
    	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case AutomaticRetries:
	  DEBUG_MESSAGE.print   ( "Automatic retries: " );
    	  for ( uint8_t j = 0; j < system_parameters [ i ].length; j++ ) {
	    DEBUG_MESSAGE.print ( system_parameters [ i ].info [ j ], HEX );
	    if ( j < system_parameters [ i ].length - 1 )
	      DEBUG_MESSAGE.print ( ", " );
	  }
	  DEBUG_MESSAGE.println ( "" );
    	  break;

    	case ConnectionMode:
	  DEBUG_MESSAGE.print ( "Connection mode: " );
	  switch ( system_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "manual" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "automatic" );
	    break;
	  default:
	    DEBUG_MESSAGE.print   ( "Unknown value: " );
	    DEBUG_MESSAGE.println ( system_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case TransparentTransmissionMode:
	  DEBUG_MESSAGE.print ( "Transparent transmission mode: " );
	  switch ( system_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "disabled" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "enabled" );
	    break;
	  default:
	    DEBUG_MESSAGE.print   ( "Unknown value: " );
	    DEBUG_MESSAGE.println ( system_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    	case TCPMonitorMode:
	  DEBUG_MESSAGE.print ( "TCP monitor mode: " );
	  switch ( system_parameters [ i ].info [ 0 ] ) {
	  case 0:
	    DEBUG_MESSAGE.println ( "disabled" );
	    break;
	  case 1:
	    DEBUG_MESSAGE.println ( "enabled" );
	    break;
	  default:
	    DEBUG_MESSAGE.print   ( "Unknown value: " );
	    DEBUG_MESSAGE.println ( system_parameters [ i ].info [ 0 ], DEC );
	  }
    	  break;

    }

  }



}


void loop ()
{

  #define SSID_STR "linksprite1"
  #define NUM_SET_PARAMETERS 12

  parameter_t set_parameter [ NUM_SET_PARAMETERS ] =
    { /* { BSSID,                       6, { 0x00, 0x09, 0x5b, 0x3d, 0xdb, 0xe4 } }, */
      { BSSID,                       6, { 0x00, 0x1e, 0x52, 0x81, 0x6c, 0x93 } },
      { SSID, sizeof ( SSID_STR ) - 1,  { SSID_STR } },
      { Channel,                     1, { 9 } },
      { EncryptionMethod,            1, { 0x00 } },       /* no encryption */
      { ServicePort,                 2, { 0x1f, 0x40 } }, /* port 8000 */
      { DeviceIPAddress,             4, { 10, 0, 0, 2 } },
      { ServerIPAddress,             4, { 10, 0, 0, 1 } },
      { SubnetMask,                  1, { 3 } },          /* 255.0.0.0 */
      { ConnectionMode,              1, { 0x00 } },       /* 0x00 - manual,
							     0x01 - automatic */
      { TransparentTransmissionMode, 1, { 0x00 } },       /* off */
      { GateWayIPAddress,            4, { 10, 0, 0, 1 } },
      { LLDataFormat,                1, { 0x01 } }        /* UDP */
    };

  DEBUG_MESSAGE.println ( "Old parameters: " );
  show_parameters ();

  DEBUG_MESSAGE.println ( "\n\nSetting parameters:" );
  set_parameters_linksprite ( Group1, set_parameter, NUM_SET_PARAMETERS );
  reset_linksprite ();

  DEBUG_MESSAGE.println ( "\n\nNew parameters: " );
  show_parameters ();

  for (;;);

}
