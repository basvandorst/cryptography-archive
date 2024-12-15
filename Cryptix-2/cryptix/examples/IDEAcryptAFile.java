/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */     

package cryptix.examples;

import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import java.util.Random;

import cryptix.security.IDEA;
import cryptix.security.MessageDigest;
import cryptix.security.MD5;
import cryptix.security.CipherFeedback;

/**
 * This is a demo of how to use the IDEA block cipher for encrypting data.
 * A file is opened and then en/de-crypted to stdout.
 * To encrypt :
 * <pre>
 *   java IDEAcryptAFile plaintext-filename passphrase > ciphertext-filename
 * </pre>
 * and to decrypt :
 * <pre>
 *   java IDEAcryptAFile -d ciphertext-filename passphrase > plaintext-filename
 * </pre>
 *
 * <P>
 * <b>References</b>
 *
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 *
 * <p>
 * @author Systemics Ltd
 * @see BlockCipher
 * @see IDEA
 * @see CipherFeedback
 */
public class IDEAcryptAFile
{
//      private static final int BUF_LENGTH = 1024;

        private static final int BUF_LENGTH = 8;

        public static void
        usage()
        {
                System.err.println( "usage java IDEAcryptAFile [-d] filename passphrase" );
                System.err.println( "Use -d to decrypt, encrypt otherwise" );
        }

        public static void
        main( String argv[] )
        {
                String filename = null, passphrase = null;
                boolean encrypt = true;

                if ( argv.length == 2 )
                {
                        filename = argv[0];
                        passphrase = argv[1];
                }
                else if ( argv.length == 3 && "-d".equals( argv[0] ) )
                {
                        filename = argv[1];
                        passphrase = argv[2];
                        encrypt = false;
                }
                

                if ( filename != null )
                        try
                        {
                                doIdea( filename, passphrase, encrypt );
                        }
                        catch( IOException ioe )
                        {
                                System.err.println(
                                        "There has been an IO exception: " +
                                        "the file has not been " +
                                        ( encrypt?"encrypted":"decrypted" )
                                );
                                ioe.printStackTrace();
                        }
                else
                        usage();
        }
        
        /**
         * Open a file and en/de-crypts it, printing the result to stdout.
         * @param filename     The name of the file to en/de-crypt
         * @param passphrase   The string to hash to get the Idea key
         * @param encrypt      True for encrypt, otherwise decrypt
         */
        private static void
        doIdea( String filename, String passphrase, boolean encrypt )
        throws IOException
        {
                FileInputStream file_in = new FileInputStream( filename );

                //
                // Create a basic block cipher with the passphrase set.
                //
                IDEA idea_cipher = new IDEA( MessageDigest.hash( passphrase, new MD5() ) );

                //
                // Now wrap that block cipher into a cipher feedback class.
                // It needs an initial value which we get from some random
                // data.
                //
                byte iv_buf[] = new byte[idea_cipher.blockLength()];
                if ( encrypt )
                {
                        Random rnd = new Random();
                        for ( int i = 0, j = iv_buf.length; j >0 ; i++, j-- )
                                iv_buf[i] = (byte)rnd.nextInt();
                        System.out.write( iv_buf );
                }
                else
                        new DataInputStream( file_in ).readFully( iv_buf );

                CipherFeedback cipher = new CipherFeedback( idea_cipher, iv_buf );

                //
                // Process the data.
                //
                byte buf[] = new byte[BUF_LENGTH];
                int position = 0;
                while( true )
                {
                        int data = file_in.read();
                        if ( data == -1 ) // eof
                                break;
                        buf[position++] = (byte)data;
                        if ( position == BUF_LENGTH )
                        {
                                position = 0;
                                if ( encrypt )
                                        cipher.encrypt( buf );
                                else
                                        cipher.decrypt( buf );
                                System.out.write( buf );
                        }
                }
                if ( position > 0 )     // flush out remains of buffer
                {
                        if ( encrypt )
                                cipher.encrypt( buf, 0, buf, 0, position );
                        else
                                cipher.decrypt( buf, 0, buf, 0, position );
                        System.out.write( buf, 0, position );
                }
                System.out.flush();
                file_in.close();
        }
}

