// This file is currently unlocked (change this line if you lock the file)
//
// $Log: BlowfishKeyGenerator.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:56  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.2  1997/08/02  David Hopwood
// + Changed minimum key length from 64 to 40 bits.
//
// Revision 0.1.0.1  1997/07/31  David Hopwood
// + Changed to allow variable-length keys.
//
// Revision 0.1.0.0  1997/07/31  David Hopwood
// + Start of history (Cryptix 2.2.0a).
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.provider.key;

/**
 * A variable-length key generator for Blowfish.
 * <p>
 * The minimum, default, and maximum key lengths are 40, 128, and 448
 * bits respectively.
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  David Hopwood
 * @since   Cryptix 2.2.0a, 2.2.2
 */
public class BlowfishKeyGenerator extends RawKeyGenerator {
    public BlowfishKeyGenerator() {
        super("Blowfish", 40 / 8, 128 / 8, 448 / 8);
    }
}
