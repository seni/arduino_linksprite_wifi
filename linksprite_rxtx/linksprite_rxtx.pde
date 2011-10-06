#include <linksprite_wifi.h>


void setup ()
{

  Serial.begin  ( 115200 );
  Serial1.begin ( 115200 );

  while ( reset_linksprite () == ERROR );

}


void loop ()
{

  while ( connect_to_network_linksprite ( Group1 ) == ERROR );

  for (;;) {

    uint16_t length = 32;
    uint8_t  data [ length ];

    send_frame ( DATA, ( uint8_t * ) "1234567890", 10, true );

    switch ( receive_frame_linksprite ( data, &length ) ) {

    case ERROR:
      break;

    case DISCONNECT:
      while ( connect_to_network_linksprite ( Group1 ) == ERROR );
      break;

    default:
      Serial.write ( data, length );
      Serial.println ( "" );

    }

  }

}
