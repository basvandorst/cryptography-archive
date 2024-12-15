package cryptix.examples;

import cryptix.security.SPEED;

public final class TestSPEED
{
        public static final void
        main( String argv[] )
        {
                try
                {
                        myMain( argv );
                }
                catch( Throwable t )
                {
                        t.printStackTrace();
                }
        }

        private static void
        myMain( String argv[] )
        throws Exception
        {
          //
          //  Note that the paper uses certification data that is indexed
          //  from RIGHT to LEFT, i.e., 7, 6, 5, 4, 3, 2, 1, 0.
          //
          test(64,                        // certification 1
            "0000000000000000",
            "0000000000000000",
            "2E008019BC26856D");
          test(128,
            "00000000000000000000000000000000",
            "00000000000000000000000000000000",
            "A44FBF29EDF6CBF8D7A2DFD57163B909");
          test(128,                        // certification 2
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
            "6C13E4B9C3171571AB54D816915BC4E8");
          test(48,
            "504F4E4D4C4B4A494847464544434241",
            "1F1E1D1C1B1A191817161514131211100F0E0D0C0B0A09080706050403020100",
            "90C5981EF6A3D21BC178CACDAD6BF39B2E51CDB70A6EE875A73BF5ED883E3692");
          test(256,
            "0000000000000000000000000000000000000000000000000000000000000000",
            "0000000000000000000000000000000000000000000000000000000000000000",
            "6CD44D2B49BC6AA7E95FD1C4AF713A2C0AFA1701308D56298CDF27A02EB09BF5");
          test(256,
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
            "C8F3E864263FAF24222E38227BEBC022CF4A9A0ECE89FB81CA1B9BA3BA93D0C5");
          test(256,                        // certification 3
            "605F5E5D5C5B5A595857565554535251504F4E4D4C4B4A494847464544434241",
            "1F1E1D1C1B1A191817161514131211100F0E0D0C0B0A09080706050403020100",
            "3DE16CFA9A626847434E1574693FEC1B3FAA558A296B61D708B131CCBA311068");
        }
        
        private static void
        test( int num_rnds, String keyStr, String plainStr, String cipherStr )
        {
                byte key[] = fromRevString( keyStr );
                byte plain[] = fromRevString( plainStr );
                byte cipher[] = fromRevString( cipherStr );

                SPEED cryptor = new SPEED( key, plain.length, num_rnds );
                byte encP[] = new byte[plain.length];
                byte decC[] = new byte[plain.length];

                cryptor.encrypt( plain, encP );
                String a,b;
                System.out.println( "    key:" + toRevString( key ) );
                System.out.println( "  plain:" + toRevString( plain ) );
                System.out.println( "    enc:" + ( a = toRevString( encP ) ) );
                b = toRevString( cipher);
                if ( a.equals( b ) )
                        System.out.print( "encryption good; " );
                else
                {
                        System.out.println( "   calc:" + b );
                        System.out.println( " ********* SPEED ENC FAILED ********* " );
                }

                cryptor.decrypt( encP, decC );
                a = toRevString( decC );
                b = toRevString( plain );
                if ( a.equals( b ) )
                        System.out.println( "decryption good" );
                else
                {
                        System.out.println();
                        System.out.println( "    enc:" + toRevString( encP ) );
                        System.out.println( "    dec:" + ( a = toRevString( decC ) ) );
                        System.out.println( "   calc:" + ( b = toRevString( plain ) ) );
                        System.out.println( " ********* SPEED DEC FAILED ********* " );
                }
        }

        //
        //  Note that the paper uses certification data that is indexed
        //  from RIGHT to LEFT, i.e., 7, 6, 5, 4, 3, 2, 1, 0.
        //  To keep the data consistent with the paper, we reverse it in and out.
        //  It is not expected that normal use would do this, however.
        //
        private static byte[]
        fromRevString( String inHex )
        {
                int len=inHex.length();
                int pos =0;
                byte buffer[] = new byte [( ( len + 1 ) / 2 )];
                if ( ( len % 2 ) == 1 )         // untested against certification
                {
                        buffer[0]=(byte)asciiToHex(inHex.charAt( --len ));
                        pos=1;
                }

                for(int ptr = pos; len > 0; )
                        buffer[pos++] = (byte)( 
                                ( asciiToHex( inHex.charAt( --len ) ) ) |
                                ( asciiToHex( inHex.charAt( --len ) ) << 4)
                                        );
                return buffer;
        }

        private static final String
        toRevString( byte buffer[] )
        {
                char[] str = new char[buffer.length * 2];
                for (int i = 0; i < buffer.length; i++)
                {
                        byte b = buffer[ buffer.length - i - 1 ];
                        str[ i*2 ]     = hexToAscii( b & 0x0F );
                        str[ i*2 + 1 ] = hexToAscii( (b >>> 4) & 0x0F );
                }
                return new String (str);
        }

        private static final int
        asciiToHex(char c)
        {
                if ( ( c >= 'a' ) && ( c <= 'f' ) )
                        return ( c - 'a' + 10 );
                if ( ( c >= 'A' ) && ( c <= 'F' ) )
                        return ( c - 'A' + 10 );
                if ( ( c >= '0' ) && ( c <= '9' ) )
                        return ( c - '0' );
                throw new Error("ascii to hex failed");
        }

        private static char
        hexToAscii(int h)
        {
                if ( ( h >= 10 ) && ( h <= 15 ) )
                        return (char)( 'A' + ( h - 10 ) );
                if ( ( h >= 0 ) && ( h <= 9 ) )
                        return (char)( '0' + h );
                throw new Error("hex to ascii failed");
        }
}

