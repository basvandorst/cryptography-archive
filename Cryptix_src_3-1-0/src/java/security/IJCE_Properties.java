// This file is currently unlocked (change this line if you lock the file)
//
// $Log: IJCE_Properties.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:57  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.0  1997/07/26  David Hopwood
// + Original version, created by separating out the code for reading
//   and initializing properties from cryptix.provider.Cryptix (as was).
//   The reason for doing that is so that CryptixProperties and
//   java.security.IJCE_Properties are as similar as possible, to ease
//   maintenance.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package java.security;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;

import netscape.security.PrivilegeManager;

/**
 * This is a support class providing facilities needed to load and manage
 * properties. It is used by other classes in <samp>java.security.*</samp>;
 * see those classes for further documentation.
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  David Hopwood
 * @author  Jill Baker
 * @author  Raif S. Naffah
 */
class IJCE_Properties
{

// Constants and vars.
//...........................................................................

    /**
     * The common name for this class library. This is used for error messages,
     * because most of the code for this class is duplicated between Cryptix
     * and IJCE.
     */
    static final String PRODUCT_NAME = "IJCE";

    /**
     * The name of the directory in which the properties file and (if
     * applicable) native libraries are found.
     */
    static final String LIB_DIRNAME = "ijce-lib";

    /**
     * The filename of the properties file.
     */
    static final String PROPERTIES_FILENAME = "IJCE.properties";

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
            String msg = "Warning: Unable to load " + PROPERTIES_FILENAME;
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
    static String getLibraryPath() throws IOException {
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
                File pf = new File(path, PROPERTIES_FILENAME);
                if (pf.isFile()) {
                    try {
                        BufferedInputStream in =
                            new BufferedInputStream(new FileInputStream(pf));
                        properties.load(in);
                        in.close();
                        lib_path = path;
                        return;
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
            try { PrivilegeManager.revertPrivilege("UniversalFileRead"); }
            catch (NoClassDefFoundError e) {}
        }
        System.err.println(
            "Warning: failed to load the " + PROPERTIES_FILENAME + " file.\n" +
            "Make sure that the CLASSPATH entry for " + PRODUCT_NAME + " is an absolute path.");
    }

    /**
     * Saves the properties to the OutputStream <i>os</i>, in the format
     * used by <code>java.util.Properties.save</code>. The string <i>comment</i>
     * is written as a comment in the first line of the output.
     */
    static void save(OutputStream os, String comment) {
        properties.save(os, comment);
    }

    /**
     * Gets the value of a property.
     */
    static String getProperty(String key) {
        return properties.getProperty(key);
    }

    /**
     * Gets the value of a property, or returns <i>defaultValue</i> if the
     * property was not set.
     */
    static String getProperty(String key, String defaultValue) {
        return properties.getProperty(key, defaultValue);
    }

    /**
     * Returns an enumeration of all the property names.
     */
    static Enumeration propertyNames() {
        return properties.propertyNames();
    }

    /**
     * Lists the properties to the PrintStream <i>out</i>.
     */
    static void list(PrintStream out) {
        properties.list(out);
    }

    /**
     * Lists the properties to the PrintWriter <i>out</i>.
     */
    static void list(PrintWriter out) {
        properties.list(out);
    }
}
