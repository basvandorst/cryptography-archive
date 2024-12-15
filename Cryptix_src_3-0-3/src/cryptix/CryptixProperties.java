// $Id: CryptixProperties.java,v 1.5 1998/01/11 03:29:59 hopwood Exp $
//
// $Log: CryptixProperties.java,v $
// Revision 1.5  1998/01/11 03:29:59  hopwood
// + Added NATIVE_ALLOWED constant.
// + Cosmetics.
//
// Revision 1.4  1998/01/10 04:10:27  hopwood
// + Fixed silly error in 1.3.
//
// Revision 1.3  1998/01/10 04:07:14  hopwood
// + Committed changes below.
//
// Revision 1.2.2  1998/01/10  hopwood
// + Changed version to 3.0.3.
// + Also look for a Local.properties file, which adds to and overrides
//   entries in Cryptix.properties.
//
// Revision 1.2.1  1998/01/01  hopwood
// + Updated list of contributors.
//
// Revision 1.2  1997/12/19 05:49:34  hopwood
// + Committed changes below.
//
// Revision 1.1.1  1997/12/19  hopwood
// + Changed version to 3.0.2, no longer a snapshot.
//
// Revision 1.1  1997/11/20 21:01:06  hopwood
// + Moved CryptixException and CryptixProperties here from the cryptix.core
//   package.
//
// Revision 0.1.2.0  1997/11/14  David Hopwood
// + Merged cryptix.core.Cryptix and cryptix.core.CryptixProperties, into
//   this new class, called cryptix.CryptixProperties.
// + Added getReleaseDate method.
// + Calculate release date from the CVS date tag.
//
// Revision 0.1.1.1  1997/09/26  David Hopwood
// + Updated list of contributors.
//
// Revision 0.1.1.0  1997/08/29  David Hopwood
// + Moved debugging methods into a separate Debug class.
// + Moved security provider implementation to
//   cryptix.provider.Cryptix.
//
// Revision 0.1.0.9  1997/08/27  David Hopwood
// + Added getDebugLevel(String algorithm, String label).
//
// Revision 0.1.0.8  1997/08/21  David Hopwood
// + Added getDebugOutput method, for redirecting debugging
//   messages (currently this always returns a PrintWriter for
//   System.err).
// + Added SNAPSHOT_DATE to the version string, so that different
//   snapshots of the same release can be distinguished.
//
// Revision 0.1.0.7  1997/08/02  David Hopwood
// + Added getInternalDebugLevel method, for debugging non-algorithm
//   classes.
// + Added getHtmlInfo method.
// + Added list of contributors to HTML_INFO.
// + Added code to print the version, installation status and library
//   path in main.
// + Renamed wantNative to isNativeWanted.
//
// Revision 0.1.0.6  1997/07/26  David Hopwood
// + Moved the implementation of loading properties, and finding the
//   library path to the new class PropertySupport.
// + Removed unsupported methods inherited from java.util.Hashtable,
//   since not overriding them does no harm to security.
//
// Revision 0.1.0.5  1997/07/25  David Hopwood
// + Fix for error messages output by ld on Unix, when a native
//   library cannot be found:
//   - change the directory that the properties file is loaded
//   from to cryptix-lib.
//   - make the path of that directory accessible using a public
//   method.
//
// Revision 0.1.0.4  1997/07/23  David Hopwood
// + Changed Disable.Native to Native.Disable.
//
// Revision 0.1.0.3  1997/07/21  David Hopwood
// + Changed property names used by wantNative and getDebugLevel
//   (see comments).
// + Removed traceAlways method (tracing is now done by IJCE, not by
//   Cryptix).
// + Other cosmetic changes.
//
// Revision 0.1.0.2  1997/07/12  David Hopwood
// + Changed back case of serialVersionUID, since it must have exactly
//   that name. Also changed 'properties' to lower case, since it refers
//   to a mutable object (even though the reference is constant).
//
// Revision 0.1.0.1  1997/07/10  R. Naffah
// + Fixed wantNative() method. Used to return boolean inverse of
//   correct value.
// + Corrected spelling of native disabling properties to follow
//   that of properties file.
// + Enclosed netscape.security methods within try/catch.
// + Re-wrote constants in uppercase.
// + In-line all cryptix.security.Cryptix methods into this class.
// + Changed the name to just Cryptix (from CryptixProvider).
//
// Revision 0.1.0.0  1997/??/??  David Hopwood
// + Original version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix;

import java.io.PrintWriter;
import java.io.PrintStream;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.OutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;
import java.security.Security;
import java.security.Provider;

import netscape.security.PrivilegeManager;

/**
 * This class defines the version number of the Cryptix library, and also
 * provides facilities needed to load and manage properties.
 * <ul>
 *   <li> The major version is intended to signal significant
 *        changes in compatibility or style.
 *   <li> The minor version is intended to signal small changes
 *        in compatibility and new, compatible additions.
 *   <li> The intermediate version is incremented for bug-fix and alpha
 *        releases.
 * </ul>
 * <p>
 * Version numbers should be written as a triple of integers, not as a decimal.
 * If the last number is omitted it is assumed to be 0, so for example, version
 * 1.10 is after version 1.2.
 * <p>
 * The property values are read from files called
 * <a href="../src/cryptix-lib/Cryptix.properties"><samp>Cryptix.properties</samp></a>
 * and <samp>Local.properties</samp>, which are stored in the <i>Cryptix library
 * directory</i>.
 * The <samp>Local.properties</samp> file (if it exists) adds to and overrides
 * entries in <samp>Cryptix.properties</samp>, and is intended to be used for
 * local customisations by a user or adminstrator.
 * <p>
 * The library directory is found by searching the current classpath. For
 * each classpath entry, a "cryptix-lib" subdirectory is looked for. If the
 * entry is a .zip or .jar file, "cryptix-lib" will be looked for in the
 * same directory as that file.
 * <p>
 * Property values loaded from <samp>Cryptix.properties</samp> or
 * <samp>Local.properties</samp> are always read-only. They can be accessed by
 * untrusted code, so should not be secret.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.5 $</b>
 * @author  David Hopwood
 * @author  Jill Baker
 * @author  Raif S. Naffah
 * @since   Cryptix 2.2.2
 */
public class CryptixProperties
{
    private CryptixProperties() {} // static methods only


// Constants and variables
//...........................................................................

    private static final int MAJOR_VERSION = 3;
    private static final int MINOR_VERSION = 0;
    private static final int INTER_VERSION = 3;

    /**
     * This field should be true if this is a snapshot release.
     */
    private static final boolean IS_SNAPSHOT = false;

    /**
     * The release date, taken as the date this file was last committed to
     * CVS.
     */
    private static final String CVS_DATE = "$Date: 1998/01/11 03:29:59 $";

    /**
     * If this constant is false, native linking is disabled regardless of
     * any settings in the properties files.
     */
    public static final boolean NATIVE_ALLOWED = true;

    /**
     * An HTML description of the provider.
     */
    static final String HTML_INFO =

"<h1>" + getVersionString() + "</h1>\n" +
"<p>\n" +
"<b>Copyright</b> &copy; 1995-1997\n" +
"<a href=\"http://www.systemics.com/\">Systemics Ltd</a> on behalf of the\n" +
"<a href=\"http://www.systemics.com/docs/cryptix/\">Cryptix Development Team</a>.\n" +
"<br>All rights reserved.\n" +
"<p>\n" +
"This library includes, or is derived from software developed by\n" +
"(and owned by):\n" +
"<blockquote>\n" +
"  Jill&nbsp;Baker, Paulo&nbsp;Barreto, George&nbsp;Barwood,\n" +
"  Antoon&nbsp;Bosselaers, Ian&nbsp;Brown, Lawrence&nbsp;Brown,\n" +
"  Joan&nbsp;Daemen, Richard&nbsp;De&nbsp;Moliner,\n" +
"  John&nbsp;F.&nbspDumas, Peter&nbsp;Gutmann,\n" +
"  Mark&nbsp;A.&nbsp;Herschberg, Uwe&nbsp;Hollerbach,\n" +
"  David&nbsp;Hopwood, Gary&nbsp;Howland, Geoffrey&nbsp;Keating,\n" +
"  Sascha&nbsp;Kettler, A.M.&nbsp;Kuchling, Matthew&nbsp;Kwan,\n" +
"  Jerry&nbsp;McBride, Andrew&nsbp;E.&nsbp;Mileski, Raif&nbsp;Naffah,\n" +
"  NIST, Bryan&nbsp;Olson, Vincent&nbsp;Rijmen,\n" +
"  RSA&nbsp;Data&nbsp;Security&nbsp;Inc., Bruce&nbsp;Schneier,\n" +
"  Systemics&nbsp;Ltd., Mike&nbsp;Wynn, Eric&nbsp;Young,\n" +
"  Yuliang&nbsp;Zheng.\n" +
"</blockquote>\n" +
"<p>\n" +
"See the <a href=\"http://www.systemics.com/docs/cryptix/FAQ.html\">Cryptix FAQ</a>\n" +
"for further details.\n";


// Version information methods
//...........................................................................

    /**
     * Returns the major version of this release of Cryptix.
     */
    public static int getMajorVersion() { return MAJOR_VERSION; }

    /**
     * Returns the minor version of this release of Cryptix.
     */
    public static int getMinorVersion() { return MINOR_VERSION; }

    /**
     * Returns the intermediate version of this release of Cryptix.
     */
    public static int getIntermediateVersion() { return INTER_VERSION; }

    /**
     * Returns true iff this version of Cryptix is at least the given
     * version.
     */
    public static boolean
    isVersionAtLeast(int major, int minor, int intermediate) {
        if (MAJOR_VERSION > major) return true;
        if (MAJOR_VERSION < major) return false;
        if (MINOR_VERSION > minor) return true;
        if (MINOR_VERSION < minor) return false;
        return INTER_VERSION >= intermediate;
    }

    /**
     * Returns the release date of this version of Cryptix, as a string in
     * the form "yyyy/mm/dd".
     */
    public static String getReleaseDate() {
        try {
            return CVS_DATE.substring(7, 17);
        } catch (StringIndexOutOfBoundsException e) {
            return "unknown";
        }
    }

    /**
     * Returns a string describing this version of Cryptix.
     */
    public static String getVersionString() {
        StringBuffer version = new StringBuffer("Cryptix-Java ")
            .append(MAJOR_VERSION).append(".").append(MINOR_VERSION);
        if (INTER_VERSION != 0)
            version.append(".").append(INTER_VERSION);
        if (IS_SNAPSHOT)
            version.append(" (").append(getReleaseDate()).append(" snapshot)");

        return version.toString();
    }

    /**
     * Returns English-language HTML credits for Cryptix, in a form that
     * could be included in an application's About box, for example.
     * <p>
     * The returned string does not have &lt;HTML&gt; or &lt;BODY&gt; tags,
     * so that it can easily be included in a larger page.
     */
    public static String getHtmlInfo() {
        return HTML_INFO;
    }


// Main
//...........................................................................

    /**
     * Prints the Cryptix version string, whether Cryptix is installed correctly,
     * and the location of the library directory.
     */
    public static void main(String[] args) {
        System.out.println(getVersionString());
        System.out.println();
        if (Security.getProvider("Cryptix") == null) {
            System.out.println("Cryptix is not installed as a provider in the java.security file.");
            System.out.println("Enter \"java cryptix.provider.Install\" to correct this.");
        } else {
            System.out.println("Cryptix is correctly installed in the java.security file.");
        }
        try {
            String libPath = CryptixProperties.getLibraryPath();
            System.out.println("The library directory is");
            System.out.println("  " + libPath);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

//
// The rest of this class should mirror the corresponding code in
// java.security.IJCE_Properties.
//...........................................................................


// Constants and vars.
//...........................................................................

    /**
     * The common name for this class library. This is used for error messages,
     * because most of the code for this class is duplicated between Cryptix
     * and IJCE.
     */
    static final String PRODUCT_NAME = "Cryptix";

    /**
     * The name of the directory in which the properties files and (if
     * applicable) native libraries are found.
     */
    static final String LIB_DIRNAME = "cryptix-lib";

    /**
     * The names of the properties files.
     */
    static final String[] PROPERTIES_FILES = { "Cryptix.properties", "Local.properties" };

    /**
     * A global, private Properties object.
     */
    private static final Properties properties = new Properties();

    /**
     * The full actual path (ending with LIB_DIRNAME and a file separator)
     * of the library directory. lib_path is null if an error occurred during
     * initialization, before the path was determined.
     */
    private static String lib_path; // defaults to null


// Static code
//...........................................................................

    static {
        try {
            setProperties();
        } catch (Exception e) {
            String msg = "Warning: Unable to load the " + PRODUCT_NAME + " properties file";
            if (lib_path != null)
                msg += " from directory " + lib_path;
            System.err.println(msg);
            e.printStackTrace();
        }
    }


// Own methods
//...........................................................................

    /**
     * Returns the path of the library directory. The name of this directory
     * is given by the LIB_DIRNAME constant.
     * <p>
     * The returned path is always absolute, and ends with a file separator
     * character (e.g. "/" on Unix).
     *
     * @exception IOException if an error occurred during intialization,
     *            preventing the path from being determined.
     */
    public static String getLibraryPath() throws IOException {
        // lib_path cannot change after class initialization.
        if (lib_path == null) throw new IOException(PRODUCT_NAME + " library directory (" +
            LIB_DIRNAME + ") could not be found");
        return lib_path;
    }

    /**
     * Loads the properties file.
     */
    private static void setProperties() {
        try { PrivilegeManager.enablePrivilege("UniversalPropertyRead"); }
        catch (NoClassDefFoundError e) {}
       
        String classpath = System.getProperty("java.class.path");
        String ps = System.getProperty("path.separator");
        String fs = System.getProperty("file.separator");
       
        try { PrivilegeManager.revertPrivilege("UniversalPropertyRead"); }
        catch (NoClassDefFoundError e) {}

        StringTokenizer st = new StringTokenizer(classpath, ps);
        boolean loaded = false;
        while (st.hasMoreTokens()) {
            String entry = st.nextToken();
            File f;
            if (entry.endsWith(".zip") || entry.endsWith(".jar")) {
                int i = entry.lastIndexOf(fs);
                if (i == -1)
                    continue; // ignore .zip/.jar files specified by relative paths

                f = new File(entry.substring(0, i), LIB_DIRNAME);
            } else {
                f = new File(entry, LIB_DIRNAME);
            }

            try { PrivilegeManager.enablePrivilege("UniversalFileRead"); }
            catch (NoClassDefFoundError e) {}

            if (f.isDirectory()) {
                String path = f.getAbsolutePath();
                if (!path.endsWith(fs)) {
                    path += fs;
                }
                for (int i = 0; i < PROPERTIES_FILES.length; i++) {
                    File pf = new File(path, PROPERTIES_FILES[i]);
                    if (pf.isFile()) {
                        try {
                            BufferedInputStream in =
                                new BufferedInputStream(new FileInputStream(pf));
                            properties.load(in);
                            in.close();
                            lib_path = path;
                            loaded = true;
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                }
                if (loaded) return;
            }
            try { PrivilegeManager.revertPrivilege("UniversalFileRead"); }
            catch (NoClassDefFoundError e) {}
        }
        System.err.println(
            "Warning: failed to load the " + PRODUCT_NAME + " properties file.\n" +
            "Make sure that the CLASSPATH entry for " + PRODUCT_NAME + " is an absolute path.");
    }

    /**
     * Saves the properties to the OutputStream <i>os</i>, in the format
     * used by <code>java.util.Properties.save</code>. The string <i>comment</i>
     * is written as a comment in the first line of the output.
     */
    public static void save(OutputStream os, String comment) {
        properties.save(os, comment);
    }

    /**
     * Gets the value of a property.
     */
    public static String getProperty(String key) {
        return properties.getProperty(key);
    }

    /**
     * Gets the value of a property, or returns <i>defaultValue</i> if the
     * property was not set.
     */
    public static String getProperty(String key, String defaultValue) {
        return properties.getProperty(key, defaultValue);
    }

    /**
     * Returns an enumeration of all the property names.
     */
    public static Enumeration propertyNames() {
        return properties.propertyNames();
    }

    /**
     * Lists the properties to the PrintStream <i>out</i>.
     */
    public static void list(PrintStream out) {
        properties.list(out);
    }

    /**
     * Lists the properties to the PrintWriter <i>out</i>.
     */
    public static void list(PrintWriter out) {
        properties.list(out);
    }
}
