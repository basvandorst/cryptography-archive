// $Id: MIMEFormatException.java,v 1.1.1.1 1997/12/22 03:28:42 hopwood Exp $
//
// $Log: MIMEFormatException.java,v $
// Revision 1.1.1.1  1997/12/22 03:28:42  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0  1997/12/20  hopwood
// + Original version.
//
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */

package cryptix.util.mime;

/**
 * This exception is thrown when MIME input data does not meet the expected
 * constraints (e.g. contains invalid characters, has the wrong length, etc.)
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1.1.1 $</b>
 * @author  David Hopwood
 * @since   Cryptix 3.0.2
 */
public class MIMEFormatException
extends java.io.IOException
{
    /** @param reason  the reason why the exception was thrown. */
    public MIMEFormatException(String reason) { super(reason); }       
}
