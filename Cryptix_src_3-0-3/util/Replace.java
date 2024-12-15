// This file is currently unlocked (change this line if you lock the file)
//
// $Log: Replace.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.3  1997/07/24  David Hopwood
// + Improved usage string.
//
// Revision 0.1.0.2  1997/07/06  David Hopwood
// + Fixed bug where "--" was being treated as a filename.
//
// Revision 0.1.0.1  1997/07/05  David Hopwood
// + Merged in changes from FixLineEndings.
//
// Revision 0.1.0.0  1997/04/09  David Hopwood
//   Initial version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package util;

import java.io.*;

/**
 * Utility application to replace a string with another string in text files.
 * <p>
 * Usage: <code>Usage: java util.Replace [-unix | -dos | -mac] [--] <i>match-string</i> <i>repl-string</i> <i>filename</i>*</code>
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * @author  David Hopwood
 * @version $Revision: 1.1.1.1 $
 */
class Replace {
    private String newline;
    private String match;
    private String repl;

    public Replace(String newline, String match, String repl) {
        this.newline = newline;
        this.match = match;
        this.repl = repl;
    }

    public void process(String filename) {
        try {
            File file = new File(filename);
            if (file.isDirectory()) {
                System.err.println(filename + " is a directory");
                return;
            }
            File backup = new File(filename + ".bak");
            backup.delete();
            file.renameTo(backup);
            BufferedReader in = new BufferedReader(new FileReader(backup));
            PrintWriter out = new PrintWriter(new BufferedWriter(
                                  new FileWriter(file)));
            String buf;
            while ((buf = in.readLine()) != null) {
                out.print(replace(buf, match, repl));
                out.print(newline);
            }
            out.close();
            in.close();
            if (file.length() == 0) {
                System.err.println("Write failed for " + filename + "?");
            } else {
                backup.delete();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static String replace(String s, String match, String repl) {
        StringBuffer sb = new StringBuffer("");
        int p = 0;

        while (p < s.length()) {
            int oldp = p;
            p = s.indexOf(match, p);
            if (p == -1) {
                sb.append(s.substring(oldp));
                break;
            }
            sb.append(s.substring(oldp, p));
            sb.append(repl);
            p += match.length();
        }
        return sb.toString();
    }

    static final String usage =
"Replace a string in a text file with another string, wherever it occurs.\n" +
"\n" +
"Usage:\n" +
"    java util.Replace [-unix | -dos | -mac] [--] match-string repl-string\n" +
"        filename*\n" +
"\n" +
"Options:\n" +
"    -unix         convert to Unix (LF) line endings.\n" +
"    -dos          convert to DOS/Windows (CR LF) line endings.\n" +
"    -mac          convert to Macintosh (CR) line endings.\n" +
"    --            treat everything after this as a filename, even if it\n" +
"                    begins with \"-\".\n" +
"    match-string  the string to find (can be enclosed in quotes).\n" +
"    repl-string   the string to replace with (can be enclosed in quotes).\n" +
"\n" +
"If none of -unix, -dos or -mac are used, the file will be written in the\n" +
"native format for this operating system.\n" +
"\n" +
"More than one filename can be specified, separated by spaces. Files are changed\n" +
"in-place. If an I/O error occurs, a backup will be left with a .bak extension.";

    public static void main(String[] args) {
        if (args.length == 0) {
            System.err.println(usage);
            System.exit(1);
        }

        String nl = System.getProperty("line.separator");

        int i;
        for (i = 0; i < args.length; i++) {
            String arg = args[i];
            if (arg.equals("-unix")) {
                nl = "\n";
            } else if (arg.equals("-dos")) {
                nl = "\r\n";
            } else if (arg.equals("-mac")) {
                nl = "\r";
            } else if (arg.equals("--")) {
                i++;
                break;
            } else if (arg.startsWith("-")) {
                System.err.println("Invalid option: " + arg);
                System.exit(1);
            } else {
                break;
            }
        }

        if (i+2 > args.length) {
            System.err.println(usage);
            System.exit(1);
        }

        Replace r = new Replace(nl, args[i], args[i+1]);
        for (i += 2; i < args.length; i++) {
            r.process(args[i]);
        }
    }
}
