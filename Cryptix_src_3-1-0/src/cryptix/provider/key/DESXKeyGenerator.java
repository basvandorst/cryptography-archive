// $Id: DESXKeyGenerator.java,v 1.1 1998/05/27 14:06:08 kettler Exp $
//
// $Log: DESXKeyGenerator.java,v $
// Revision 1.1  1998/05/27 14:06:08  kettler
// + Added DESX another variant of using DES with 56+64 bit keys
//
//
// $Endlog$
/*
 * Copyright (c) 1998 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.provider.key;

import java.security.WeakKeyException;

/**
 * A key generator for DESX with one DES key and one XOR key.
 * <p>
 * <b>Copyright</b> &copy; 1998
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1 $</b>
 * @author  Sascha Kettler
 * @author  David Hopwood
 * @author  Raif Naffah
 * @since   Cryptix 3.0.4
 */
public class DESXKeyGenerator
extends DESKeyGenerator
{
    public DESXKeyGenerator() {
        super("DESX", 16);
    }

    /**
     * Returns true iff the first 8 bytes of <i>key</i> represent a weak
     * DES key.
     */
    protected boolean isWeak(byte[] key) {
        if (isWeak(key, 0))
            return true;

        return false;
    }
}
