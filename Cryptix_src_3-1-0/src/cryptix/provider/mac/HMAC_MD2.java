// $Id: HMAC_MD2.java,v 1.1 1998/01/12 04:17:51 hopwood Exp $
//
// $Log: HMAC_MD2.java,v $
// Revision 1.1  1998/01/12 04:17:51  hopwood
// + HMAC rewrite.
//
// Revision 0.1.0  1998/01/04  hopwood
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1995-97 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.provider.mac;

/**
 * A class to implement the HMAC message authentication code, as described in
 * RFC 2104, with the MD2 digest algorithm.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1 $</b>
 * @author  David Hopwood
 * @since   Cryptix 3.0.3
 */
public class HMAC_MD2
extends HMAC
{

// Constructor
//...........................................................................

    public HMAC_MD2() { super("MD2", 64); }
}
