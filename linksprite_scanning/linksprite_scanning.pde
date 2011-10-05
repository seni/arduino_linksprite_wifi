#include <linksprite_wifi.h>


void setup ()
{

  Serial.begin  ( 115200 );
  Serial1.begin ( 115200 );

  while ( reset_linksprite () == ERROR );

}


/* Channel masks */
#define CHANNEL1  0x0100
#define CHANNEL2  0x0200
#define CHANNEL3  0x0400
#define CHANNEL4  0x0800
#define CHANNEL5  0x1000
#define CHANNEL6  0x2000
#define CHANNEL7  0x4000
#define CHANNEL8  0x8000
#define CHANNEL9  0x0001
#define CHANNEL10 0x0002
#define CHANNEL11 0x0004
#define CHANNEL12 0x0008
#define CHANNEL13 0x0010
#define CHANNEL14 0x0020
#define ALL_CHANNELS ( CHANNEL1  | CHANNEL2  | CHANNEL3  | CHANNEL4  | \
		       CHANNEL5  | CHANNEL6  | CHANNEL7  | CHANNEL8  | \
		       CHANNEL9  | CHANNEL10 | CHANNEL11 | CHANNEL12 | \
		       CHANNEL13 | CHANNEL14 )


void loop ()
{

  uint8_t nresults = 16;
  network_scan_result_t scan_results [ nresults ];

  scan_network_linksprite ( ALL_CHANNELS, scan_results, &nresults );

  DEBUG_MESSAGE.print   ( "Number of stations: " );
  DEBUG_MESSAGE.println ( nresults, DEC );

  for ( uint8_t i = 0; i < nresults ; i++ ) {

    DEBUG_MESSAGE.print ( "net " );
    DEBUG_MESSAGE.print ( i, DEC );
    DEBUG_MESSAGE.print ( ", index/strength: " );
    DEBUG_MESSAGE.print ( scan_results [ i ].index_strength, DEC );

    DEBUG_MESSAGE.print ( ", channel " );
    DEBUG_MESSAGE.print ( scan_results [ i ].channel, DEC );

    DEBUG_MESSAGE.print ( ", bssid: " );
    for ( uint8_t j = 0; j < BSSID_LENGTH; j++ )
      DEBUG_MESSAGE.print ( scan_results [ i ].bssid [ j ], HEX );

    DEBUG_MESSAGE.print ( ", ssid: " );
    for ( uint8_t j = 0; j < scan_results [ i ].length; j++ )
      DEBUG_MESSAGE.write ( scan_results [ i ].ssid [ j ] );

    DEBUG_MESSAGE.println ( "" );

  }

  DEBUG_MESSAGE.println ( "" );

}
