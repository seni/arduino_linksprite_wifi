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

    send_frame ( DATA, ( uint8_t * ) "1234567890", 10, false );
    increase_sequence_number ();

    while ( LinkSprite.available () )
      Serial.println ( Serial1.read () );

  }

}
