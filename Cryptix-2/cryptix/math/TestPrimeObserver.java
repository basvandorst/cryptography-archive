/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.math;

/**
 * This is the callback for monitoring progress of prime number generation
 *
 * <p>Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */
public interface TestPrimeObserver
{
	public static final int HAS_SMALL_FACTORS = 1;
	public static final int NO_SMALL_FACTORS = 2;
	public static final int NOT_PRIME = 3;
	public static final int IS_PRIME = 4;
	
	public abstract void handleEvent( int evnt );
}

