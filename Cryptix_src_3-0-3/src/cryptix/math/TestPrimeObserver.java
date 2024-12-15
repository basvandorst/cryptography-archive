// This file is currently unlocked (change this line if you lock the file)
//
// $Log: TestPrimeObserver.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.2.5.1  1997/03/15  Jill Baker
//   Moved this file here from old namespace.
//
// Revision 0.2.5.0  1997/02/24  Original Author not stated
//   Original version
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.math;

/**
 * This is a callback interface for monitoring the progress of primality
 * testing using the TestPrime class.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author unattributed
 * @since  Cryptix 2.2 or earlier
 */
public interface TestPrimeObserver
{
    public static final int HAS_SMALL_FACTORS = 1;
    public static final int NO_SMALL_FACTORS = 2;
    public static final int NOT_PRIME = 3;
    public static final int IS_PRIME = 4;
    
    void handleEvent(int evnt);
}

