/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 */

package cryptix.pgp;

import java.util.Properties;

/**
 * This class enables the encoding and decoding of the "- " protection
 * which will protect a String against some forms of corruption during
 * email transport. Note that this class expects all line breaks to
 * be represented (both before and after) as CRLFs.
 *
 * @author Jill Baker
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */

public abstract class DashProtected
{
    /**
     * Prefix all vulnerable lines with "- ".
     * @param s text to be encoded
     * @return dash-proteced text
     */
    public static String encode( String s )
    {
        int startPos = 0;
        int len = s.length();
        StringBuffer buffer = new StringBuffer( 2 * len );

        while (startPos < len)
        {
            int nextPos = s.indexOf( "\r\n", startPos ) + 2;
            if (nextPos == 1) nextPos = len;

            if ((s.startsWith( "-",     startPos ))
             || (s.startsWith( "From ", startPos ))
             || (s.startsWith( ".\r\n", startPos )))
            {
                buffer.append( "- " );
            }
            buffer.append( s.substring( startPos, nextPos ));
            startPos = nextPos;
        }
        return buffer.toString();
    }

    /**
     * Remove all "- "s from the start of lines.
     * @param s dash-proteced text to be decoded
     * @return text with dash-protection removed
     */
    public static String decode( String s )
    {
        int startPos = (s.startsWith( "- " )) ? 2 : 0;
        int len = s.length();
        StringBuffer buffer = new StringBuffer( len );

        while (startPos < len)
        {
            int nextPos = s.indexOf( "\r\n- ", startPos );
            if (nextPos == -1)
            {
                buffer.append( s.substring( startPos ));
                startPos = len;
            }
            else
            {
                buffer.append( s.substring( startPos, nextPos + 2 ));
                startPos = nextPos + 4;
            }
        }
        return buffer.toString();
    }

}
