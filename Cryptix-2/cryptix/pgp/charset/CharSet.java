package cryptix.pgp.charset;

import cryptix.pgp.*;

/**
 * This abstract class contains static methods to convert between PGP's
 * internal character set, and any charset whose name is recognised
 * by old-PGP.
 *
 * @author Jill Baker
 */
public abstract class CharSet
{
    /**
     * Convert a byte[] buffer from PGP's internal form to the specified
     * charset.
     */
    public static byte[] encode( byte[] in, String name )
    throws FormatException
    {
        String lname = name.toLowerCase();

        if (lname.equals( "ascii" ))
            return Ascii.encode( in );

        else if (lname.equals( "alt_codes" ))
            return Cp866.encode( in );

        else if (lname.equals( "cp850" ))
            return Cp850.encode( in );

        else if (lname.equals( "cp852" ))
            return Cp852.encode( in );

        else if (lname.equals( "cp860" ))
            return Cp860.encode( in );

        else if (lname.equals( "cp866" ))
            return Cp866.encode( in );

        else if (lname.equals( "keybcs" ))
            return Keybcs.encode( in );

        else if (lname.equals( "ko18" ))
            return in;

        else if (lname.equals( "latin1" ))
            return in;

        else if (lname.equals( "mac" ))
            return Mac.encode( in );

        else if (lname.equals( "next" ))
            return Next.encode( in );

        else if (lname.equals( "noconv" ))
            return in;

        else
            throw new FormatException( "Charset '" + name + "' not recognised" );
    }

    /**
     * Convert a byte[] buffer from the specified charset
     * to PGP's internal form.
     */
    public static byte[] decode( byte[] in, String name )
    throws FormatException
    {
        String lname = name.toLowerCase();

        if (lname.equals( "ascii" ))
            return Ascii.decode( in );

        else if (lname.equals( "alt_codes" ))
            return Cp866.decode( in );

        else if (lname.equals( "cp850" ))
            return Cp850.decode( in );

        else if (lname.equals( "cp852" ))
            return Cp852.decode( in );

        else if (lname.equals( "cp860" ))
            return Cp860.decode( in );

        else if (lname.equals( "cp866" ))
            return Cp866.decode( in );

        else if (lname.equals( "keybcs" ))
            return Keybcs.decode( in );

        else if (lname.equals( "ko18" ))
            return in;

        else if (lname.equals( "latin1" ))
            return in;

        else if (lname.equals( "mac" ))
            return Mac.decode( in );

        else if (lname.equals( "next" ))
            return Next.decode( in );

        else if (lname.equals( "noconv" ))
            return in;

        else
            throw new FormatException( "Charset '" + name + "' not recognised" );
    }
}



