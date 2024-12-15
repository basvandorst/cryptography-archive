// This file is currently unlocked (change this line if you lock the file)
//
// $Log: FileRandomStream.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
/*
 * Copyright (c) 1995, 1996, 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package cryptix.pgp;

import java.io.*;
import java.net.*;
import java.util.*;
import java.security.SecureRandom;

import cryptix.security.CSRandomStream;
import cryptix.security.SHA;
import netscape.security.PrivilegeManager;

/**
 * A random stream that uses a seed file to provide an initial source of
 * entropy. When the object is finalized, the seed file is updated (it can
 * also be updated explicitly).
 * <p>
 * As well as the seed file, entropy is taken from the following
 * possibly-unpredictable sources:
 * <ul>
 *   <li> a stack trace for the current method,
 *   <li> the current time in milliseconds,
 *   <li> the times (in milliseconds) taken to do a garbage collection and a
 *        finalization,
 *   <li> 20 bytes generated from new SecureRandom(), if available (i.e.
 *        on Java 1.1 only),
 *   <li> the System properties (or if they cannot be read, some properties
 *        accessible to applets),
 *   <li> the IP address(es) of localhost,
 *   <li> listings of the directories ".", "..", "/", "/tmp", "/temp" and
 *        user.home, if available.
 *   <li> 20 bytes from /dev/random, if it exists and can be read.
 * </ul>
 * <p>
 * The GUI is a better source of entropy than some of these methods
 * (especially for applets), but is not included here since Cryptix is
 * non-GUI.
 * <p>
 * If neither SecureRandom nor /dev/random are available (i.e. Java 1.0.2
 * on most platforms other than Linux), the security of this may be a little
 * suspect. We should really implement SecureRandom (and SeedGenerator) for
 * Java 1.0.2.
 * <p>
 * SECURITY: in the previous version, this class could in theory be exploited by
 * untrusted code to clobber the seed file (which can be any filename). This could
 * be bad for certain OS or other configuration files, possibly leading to a more
 * serious exploit.
 * <p>
 * This version includes a security check that the caller is allowed to write to
 * the seed file.
 * <p>
 *
 * <b>Copyright</b> &copy; 1995-1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author original author unknown
 * @author David Hopwood
 * @since  Cryptix 2.2 or earlier
 */
//
// Must be able to add entropy from file
// Key generators should be able to add their own entropy (from GUI),
// key generators must create their own generator, and seed with 20
// bytes from this.  That way they can save the seed.
//
public class FileRandomStream extends CSRandomStream
{
    // Doesn't matter if not all of these are accessible in all browsers.
    private static final String[] appletProperties = {
        "java.class.version", "java.vendor", "java.vendor.url", "java.version",
        "os.arch", "os.name", "browser.version", "browser.vendor"
    };

    private File seedFile;

    /**
     * Constructs a FileRandomStream with no seed file.
     */
    public FileRandomStream() throws IOException
    {
        super(new byte[20]);
        initInternal(null);
    }

    /**
     * Constructs a FileRandomStream with the given seed file.
     *
     * @exception SecurityException the current security manager's checkWrite
     *          method is called with the absolute path of <i>seed_file</i>
     *          (if it is non-null).
     */
    public FileRandomStream(File seed_file) throws IOException
    {
        super(new byte[20]);

        if (seed_file == null) {
            initInternal(null);
        } else {
            // Don't allow untrusted code to clobber any file using this class.
            String abspath = seed_file.getAbsolutePath();
            SecurityManager security = System.getSecurityManager();
            if (security != null) security.checkWrite(abspath);
            initInternal(abspath);
        }
    }

    /**
     * (Re-)initializes the generator. This will attempt to gather
     * somewhat-unpredictable input from various sources, in addition to
     * the seed file.
     * <p>
     * <i>seed_file</i> may be null if no seed file is to be used.
     *
     * @exception SecurityException the current security manager's checkWrite
     *          method is called with the absolute path of <i>seed_file</i>
     *          (if it is non-null).
     */
    public synchronized void init(File seed_file) throws IOException
    {
        if (seed_file == null) {
            initInternal(null);
        } else {
            // Don't allow untrusted code to clobber any file using this class.
            String abspath = seed_file.getAbsolutePath();
            SecurityManager security = System.getSecurityManager();
            if (security != null) security.checkWrite(abspath);
            initInternal(abspath);
        }
    }

    private void initInternal(String seed_filename) throws IOException
    {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(bos);

        try {
            //
            // Stack trace for this method.
            //
            new Throwable().printStackTrace(new PrintStream(dos));

            //
            // Time a garbage collection and finalization.
            //
            dos.writeLong(System.currentTimeMillis());
            System.gc();
            dos.writeLong(System.currentTimeMillis());
            System.runFinalization();
            dos.writeLong(System.currentTimeMillis());

            try {
                // In Java 1.1, use SecureRandom to generate 20 random bytes
                // (more than 20 wouldn't help, since that is the size of
                // SecureRandom's internal state).
                // This will be slow if SecureRandom has not already been used.

                byte[] buf = new byte[20];
                new SecureRandom().nextBytes(buf);
                dos.write(buf);
            } catch (NoClassDefFoundError e) {}

            dos.writeLong(System.currentTimeMillis());
            try {
                PrivilegeManager.enablePrivilege("UniversalPropertyAccess");
                try {
                    // Get the System properties.

                    Properties props = System.getProperties();
                    props.save(dos, "");
                } catch (Exception e) {
                    // We may be an applet. Try getting the properties
                    // accessible to applets individually.

                    for (int i = 0; i < appletProperties.length; i++) {
                        try {
                            dos.writeChars(System.getProperty(appletProperties[i]));
                        } catch (Exception e2) {}
                    }
                }
            } catch (Exception e) {}

            PrivilegeManager.enablePrivilege("UniversalFileRead");
            //
            // At this point assume we are an application
            //
            File cwd = new File(System.getProperty("user.dir"));
            cwd = new File(cwd.getAbsolutePath());

            String home_dir = System.getProperty("user.home");
            File home = new File(home_dir);
            home = new File(home.getAbsolutePath());

            if (seed_filename == null)
                seedFile = new File(home, ".random");
            else
                seedFile = new File(seed_filename);

            // shouldn't we throw an exception if the seed file doesn't exist?
            if (seedFile.exists())
                readSeedFile();

            dos.writeLong(System.currentTimeMillis());

            //
            // Throw in some network info.
            //
            dos.writeChars(InetAddress.getLocalHost().toString());

            InetAddress[] addrs = InetAddress.getAllByName(
                InetAddress.getLocalHost().getHostName());
            for(int i = 0; i < addrs.length; i++)
                dos.writeChars(addrs[i].toString());

            addrs = InetAddress.getAllByName("localhost");
            for(int i = 0; i < addrs.length; i++)
                dos.writeChars(addrs[i].toString());
            dos.writeLong(System.currentTimeMillis());

            //
            // Now do a "ls -la" in ., .., /, /tmp, /temp and HOME
            //
            String[] files = cwd.list();
            for(int i = 0; i < files.length; i++)
            {
                File f = new File(cwd, files[i]);
                dos.writeLong(f.lastModified());
                dos.writeLong(f.length());
                dos.writeBoolean(f.isDirectory());
                dos.writeBoolean(f.isFile());
                dos.writeBoolean(f.canWrite());
                dos.writeBoolean(f.canRead());
                dos.writeChars(f.toString());
            }
            dos.writeLong(System.currentTimeMillis());

            File parent = new File(cwd.getParent());
            files = parent.list();
            for(int i = 0; i < files.length; i++)
            {
                File f = new File(parent, files[i]);
                dos.writeLong(f.lastModified());
                dos.writeLong(f.length());
                dos.writeBoolean(f.isDirectory());
                dos.writeBoolean(f.isFile());
                dos.writeBoolean(f.canWrite());
                dos.writeBoolean(f.canRead());
                dos.writeChars(f.toString());
            }
            dos.writeLong(System.currentTimeMillis());

            String root = File.separator;
            File file = new File(root);
            files = file.list();
            for(int i = 0; i < files.length; i++)
            {
                File f = new File("", files[i]);
                dos.writeLong(f.lastModified());
                dos.writeLong(f.length());
                dos.writeBoolean(f.isDirectory());
                dos.writeBoolean(f.isFile());
                dos.writeBoolean(f.canWrite());
                dos.writeBoolean(f.canRead());
                dos.writeChars(f.toString());
            }
            dos.writeLong(System.currentTimeMillis());

            file = new File(root+"tmp");
            if (file.exists() && file.isDirectory())
            {
                files = file.list();
                for(int i = 0; i < files.length; i++)
                {
                    File f = new File(file, files[i]);
                    dos.writeLong(f.lastModified());
                    dos.writeLong(f.length());
                    dos.writeBoolean(f.isDirectory());
                    dos.writeBoolean(f.isFile());
                    dos.writeBoolean(f.canWrite());
                    dos.writeBoolean(f.canRead());
                    dos.writeChars(f.toString());
                }
                dos.writeLong(System.currentTimeMillis());
            }
            else
                dos.writeLong(System.currentTimeMillis());

            file = new File(root+"temp");
            if (file.exists() && file.isDirectory())
            {
                files = file.list();
                for(int i = 0; i < files.length; i++)
                {
                    File f = new File(file, files[i]);
                    dos.writeLong(f.lastModified());
                    dos.writeLong(f.length());
                    dos.writeBoolean(f.isDirectory());
                    dos.writeBoolean(f.isFile());
                    dos.writeBoolean(f.canWrite());
                    dos.writeBoolean(f.canRead());
                    dos.writeChars(f.toString());
                }
                dos.writeLong(System.currentTimeMillis());
            }
            else
                dos.writeLong(System.currentTimeMillis());

            if (home.exists() && home.isDirectory())
            {
                files = home.list();
                for(int i = 0; i < files.length; i++)
                {
                    File f = new File(home, files[i]);
                    dos.writeLong(f.lastModified());
                    dos.writeLong(f.length());
                    dos.writeBoolean(f.isDirectory());
                    dos.writeBoolean(f.isFile());
                    dos.writeBoolean(f.canWrite());
                    dos.writeBoolean(f.canRead());
                    dos.writeChars(f.toString());
                }
                dos.writeLong(System.currentTimeMillis());
            }
            else
                dos.writeLong(System.currentTimeMillis());


            file = new File(root+"dev", "random");
            if (file.exists() && !file.isDirectory() && file.canRead())
            {
                FileInputStream fis = new FileInputStream(file);
                for(int i = 0; i < 20; i++)
                    dos.writeByte(fis.read());
                dos.writeLong(System.currentTimeMillis());
            }
        }
        catch (Exception e)
        {
            e.printStackTrace(new PrintStream(dos));
        }
        PrivilegeManager.revertPrivilege("UniversalFileRead");
        PrivilegeManager.revertPrivilege("UniversalPropertyAccess");

        add(bos.toByteArray());
        updateSeedFile();
    }

    protected void finalize()
    {
        try {
            updateSeedFile();
        }
        catch (IOException e) {}
        catch (SecurityException e) {}
    }

    /**
     * Add some entropy to the generator.
     */
    public void add(byte[] entropy)
    {
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(bos);

        try {
            dos.writeLong(System.currentTimeMillis());
        } catch (IOException e) {} // Should never happen

        super.add(bos.toByteArray());
        super.add(entropy);
    }

    /**
     * Add entropy from the seed file.
     */
    public synchronized void readSeedFile()
    {
        try {
            byte[] buf;
            int len = (int) seedFile.length();

            PrivilegeManager.enablePrivilege("UniversalFileRead");
            FileInputStream in = new FileInputStream(seedFile);
            new DataInputStream(in).readFully(buf = new byte[len]);
            in.close();
            PrivilegeManager.revertPrivilege("UniversalFileRead");

            add(buf);
        } catch (IOException ioe) {}
    }

    /**
     * Update the seed file.
     */
    public synchronized void updateSeedFile() throws IOException
    {
        byte[] buf = new byte[24];
        this.read(buf);

        PrivilegeManager.enablePrivilege("UniversalFileWrite");
        FileOutputStream out = new FileOutputStream(seedFile);
        out.write(buf);
        out.close();
        PrivilegeManager.revertPrivilege("UniversalFileWrite");
    }

//
// Self-test disabled since it is insecure.
//
/*
    public static void main(String[] args) throws IOException
    {
        FileRandomStream ris;
        if (args.length > 0)
            ris = new FileRandomStream(new File(args[0]));
        else
            ris = new FileRandomStream();

        DataInputStream dis = new DataInputStream(ris);
        for (int i = 0; i < 100; i++)
        {
            System.out.println(dis.readByte());
        }
    }
*/
}
