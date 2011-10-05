

import java.io.*;
import java.net.*;


public class udpServer {


    public static void main ( String args [] )
        throws Exception
    {

        DatagramSocket wifiSocket = new DatagramSocket ( 8000 );

        int  maxUdpBufferSize = 64 * 1024; /* UDP has a maximum packet size of about 65k */
        byte udpDataBuffer [] = new byte [ maxUdpBufferSize ];

	int count = 0;
        while ( true ) {

            DatagramPacket inputPacket = new DatagramPacket ( udpDataBuffer, maxUdpBufferSize );

            wifiSocket.receive ( inputPacket );

            String data = new String ( inputPacket.getData (), 0, inputPacket.getLength () );

            System.out.println ( "" + count + ", got: " + data );
            System.out.println ( "length: " + inputPacket.getLength () );
            System.out.println ( "address: " + inputPacket.getAddress () );
            System.out.println ( "port: " + inputPacket.getPort () );

	    String returnData = "" + count++ + ": received " + data + ";\n";
            DatagramPacket outputPacket =
		new DatagramPacket ( returnData.getBytes (),
				     returnData.length (),
				     inputPacket.getAddress (),
				     inputPacket.getPort () );

	    wifiSocket.send ( outputPacket );

        }

	/* wifiSocket.close (); */

    }

}
