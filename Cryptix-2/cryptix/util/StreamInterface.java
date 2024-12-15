/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This library and applications are FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the conditions within the COPYRIGHT file are adhered to.
 *
 */

package cryptix.util;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

/**
 * This interface allows objects to be read from
 * or written onto DataInput and DataOutput objects.
 * There are also binary array methods to allow
 * other types of storing or when the length is required.<p>
 */
public interface StreamInterface
{
	/**
	 * This loads the object from a saved buffer.
	 */
	void load( byte buffer[] ) throws IOException;

	/**
	 * This saves the object into a buffer.
	 */
	byte[] save() throws IOException;
	
	/**
	 * This reads the object from the data input stream
	 */
	void read(DataInput in) throws IOException;

	/**
	 * This writes the object to the data output stream
	 */
	void write(DataOutput out) throws IOException;
}
