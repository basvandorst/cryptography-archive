// $Id: EmailAddress.java,v 1.3 1998/03/11 17:13:23 ianb Exp $
//
// $Log: EmailAddress.java,v $
// Revision 1.3  1998/03/11 17:13:23  ianb
// + stores address in lower case as addresses are case-insensitive
// + displays invalid address when an exception is thrown
//
// Revision 1.2  1997/11/20 22:27:04  hopwood
// + cryptix.util.* name changes.
//
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.1  1997/06/21  David Hopwood
// + Added more documentation.
// + Fixed minor bug where the wrong exception would have been thrown
//   for some invalid addresses.
// + Changed test code to use self_test convention.
//
// Revision 0.1.0.0  1997/06/21  Ian Brown
// + Start of history (Cryptix 2.2).
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.util;

import java.io.PrintWriter;
import java.io.IOException;
import java.util.Vector;

/**
 * Represents an Internet e-mail address - username and host can be retrieved
 * separately.
 * <p>
 * This may seem rather banal.
 * It's useful for three reasons, however:
 * <ul>
 *   <li> To fetch a user's public key from a distributed keyserver system, you
 *        need to know the hostname - this class saves duplication of effort.
 *   <li> It allows KeyRing.getKey() to be overloaded rather than having a
 *        separate getKeyByAddress().
 *   <li> 'Friendly' e-mail addresses of the form
 *        "<samp>Friendly Name &lt;dfh343@host.domain&gt;</samp>"
 *        are automatically dealt with.
 * </ul>
 * <p>
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.3 $</b>
 * @author  Ian Brown
 * @since   Cryptix 2.2
 */
public final class EmailAddress
{
    private String address;
    private int atPos;
    private Vector hostpath; // defaults to null

    /**
     * Constructs an EmailAddress from the given friendly address.
     *
     * @exception IOException if the address is not valid.
     */
    public EmailAddress(String friendlyAddress)
    throws IOException
    {
        // Actual address may  be between < and >
        int startAddress = friendlyAddress.indexOf("<") + 1;
        int endAddress = friendlyAddress.lastIndexOf(">");
        // indexOf returns -1 if char can't be found
        if (startAddress != 0 && endAddress != -1 &&
             endAddress > startAddress)
            address = friendlyAddress.substring(startAddress, endAddress);
        else
            address = friendlyAddress;

       // E-mail addresses are case-insensitive, so do all storage and comparison
       // on lower-case characters
        address = friendlyAddress.toLowerCase();
        if ((atPos = address.indexOf("@")) == -1 )
            throw new IOException("Invalid Internet e-mail address: " + address);
    }

    public String toString()
    {
        return address;
    }

    /**
     * Gets the username for this address (i.e. the portion before the "@"
     * symbol).
     */
    public String user()
    {
        return address.substring(0, atPos);
    }

    /**
     * Gets the host name/domain for this address (i.e. the portion after
     * the "@" symbol).
     */
    public String host()
    {
        return address.substring(atPos + 1);
    }

    /**
     * Gets a Vector containing each dot-separated component of the host name.
     * This is useful for clients of distributed keyservers who need to query
     * each possible keyserver in turn.
     */
    public Vector hostPath()
    {
        if (hostpath == null)
        {
            // Do initialisation here rather than in constructor as most
            // instantiations of this class won't need these details.
            hostpath = new Vector();
            int lastDot = atPos;
            int thisDot;
            while ((thisDot = address.indexOf(".", lastDot + 1)) != -1)
            {
                hostpath.addElement(address.substring(lastDot + 1, thisDot));
                lastDot = thisDot;
            }
            hostpath.addElement(address.substring(lastDot + 1)); // Get last domain
        }
        return hostpath;
    }

    public boolean equals(Object obj)
    {
        if (obj instanceof EmailAddress)
        {
            EmailAddress check = (EmailAddress) obj;
            return (check.toString().equals(toString()));
        }
        else return false;
    }

    public int hashCode()
    {
        return address.hashCode();
    }


// Test methods
//...........................................................................

    public static void main(String[] args) 
    {
        try { self_test(new PrintWriter(System.out, true)); }
        catch (Exception e) { e.printStackTrace(); }
    }

    public static void self_test(PrintWriter out) throws IOException
    {
        EmailAddress myAddress = new EmailAddress("Ian Brown <Ian.Brown@newcastle.ac.uk>");
        out.println("User: " + myAddress.user());
        Vector hosts = myAddress.hostPath();
        for (int i = 0; i < hosts.size(); i++)
        {
            out.println("Next domain: " + hosts.elementAt(i));
        }
    }
}
