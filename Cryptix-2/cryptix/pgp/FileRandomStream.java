/*
 * Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 */

package cryptix.pgp;

import java.io.*;
import java.net.*;
import java.util.*;
import cryptix.security.CSRandomStream;
import cryptix.security.SHA;

/**
 *
 * This class *may* be useful in an applet, but it is unlikely.
 * The browser should really be responsible for providing random
 * streams for applets.<p>
 * The gui is a better source of entropy for applets than the
 * methods here, but is not included here since Cryptix is non gui.<p>
 * Still, this class is usable by applets, hence all the exception
 * handling.
 *
 * <hr>
 * <P>
 * <bold>Copyright</bold> &#169 1995, 1996, 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a>
 * on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">
 * Cryptix Development Team</a>.
 * All rights reserved.
 */
//
// Must be able to add entropy from file
// Key generators should be able to add their own entropy (from gui),
// key generators must create their own generator, and seed with 20
// bytes from this.  That way they can save the seed.
//
// Entropy can be thrown in at any time
// randseed.bin file is updated upon garbage collection
//
public class FileRandomStream extends CSRandomStream
{
	File seed_file;

	public FileRandomStream()
		throws IOException
	{
		super(new byte[20]);
		File f = null;
		init(f);
	}

	public FileRandomStream(File seed_file)
		throws IOException
	{
		super(new byte[20]);
		init(seed_file);
	}

	public void
	init(File seed_file)
		throws IOException
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		DataOutputStream dos = new DataOutputStream(bos);

		try {
			dos.writeLong(System.currentTimeMillis());
			dos.writeLong(System.currentTimeMillis());

			Properties props = System.getProperties();
			dos.writeLong(System.currentTimeMillis());

			String root = (String)props.get("file.separator");
			dos.writeChars(root);

			//
			// Time a garbage collect
			//
			dos.writeLong(System.currentTimeMillis());
			System.gc();
			dos.writeLong(System.currentTimeMillis());
			System.runFinalization();
			dos.writeLong(System.currentTimeMillis());

			// Add the properties readable by applets
			// sha.add(System.getProperty("file.separator"));
			// sha.add(System.getProperty("java.class.version"));
			// sha.add(System.getProperty("java.vendor"));
			// sha.add(System.getProperty("java.vendor.url"));
			// sha.add(System.getProperty("java.version"));
			// sha.add(System.getProperty("line.separator"));
			// sha.add(System.getProperty("os.arch"));
			// sha.add(System.getProperty("os.name"));
			// sha.add(System.getProperty("path.separator"));
			// sha.add(Long.toString(System.currentTimeMillis()));
			dos.writeLong(System.currentTimeMillis());


			//
			//	At this point assume we are an application
			//
			File cwd = new File((String)props.get("user.dir"));
			cwd = new File(cwd.getAbsolutePath());

			String home_dir = (String)props.get("user.home");
			File home = new File(home_dir);
			home = new File(home.getAbsolutePath());

			if (seed_file == null)
				seed_file = new File(home, ".random");
			this.seed_file = seed_file;

			if (seed_file.exists())
				readSeedFile();


			//
			//	Add the system properties
			//
			props.save(dos, "");
			dos.writeLong(System.currentTimeMillis());


			//
			//	Throw in the internet info
			//
			dos.writeChars(InetAddress.getLocalHost().toString());

			InetAddress addrs[] = InetAddress.getAllByName(InetAddress.getLocalHost().getHostName());
			for(int i = 0; i < addrs.length; i++)
				dos.writeChars(addrs[i].toString());

			addrs = InetAddress.getAllByName("localhost");
			for(int i = 0; i < addrs.length; i++)
				dos.writeChars(addrs[i].toString());
			dos.writeLong(System.currentTimeMillis());


			//
			//	Now do a "ls -la" in ., .., /, /tmp, /temp and HOME
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
		catch (Throwable t)
		{
			t.printStackTrace(new PrintStream(dos));
		}

		add(bos.toByteArray());
		updateSeedFile();
	}

	protected void
	finalize()
	{
		try {
			updateSeedFile();
		}
		catch(IOException e) {}
	}

	///
	//  Add some entropy to the generator
	//
	public void
	add(byte[] entropy)
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		DataOutputStream dos = new DataOutputStream(bos);

		try {
			dos.writeLong(System.currentTimeMillis());
		}
		catch (IOException e) {} // Should never happen

		super.add(bos.toByteArray());
		super.add(entropy);
	}


	public synchronized void
	readSeedFile()
	{
		try {
			byte buf[];
			int len = (int)seed_file.length();
			FileInputStream in = new FileInputStream(seed_file);
			new DataInputStream(in).readFully(buf = new byte[len]);
			in.close();
			add(buf);
		}
		catch (IOException ioe) {}
	}

	public synchronized void
	updateSeedFile()
		throws IOException
	{
		byte[] buf = new byte[24];
		this.read(buf);
		FileOutputStream out = new FileOutputStream(seed_file);
		out.write(buf);
		out.close();
	}

	public static final void
	main(String[] argv)
		throws IOException
	{
		FileRandomStream ris;
		if (argv.length > 0)
			ris = new FileRandomStream(new File(argv[0]));
		else
			ris = new FileRandomStream();
		DataInputStream dis = new DataInputStream(ris);
		for (int i = 0; i < 100; i++)
		{
			System.out.println(dis.readByte());
		}
	}

}
