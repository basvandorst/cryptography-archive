// This file is currently unlocked (change this line if you lock the file)
//
// $Log: FixLineEndings.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.5  1997/07/24  David Hopwood
// + Improved usage string.
//
// Revision 0.1.0.4  1997/07/14  David Hopwood
// + The fix of 1.0.3 wasn't quite correct.
//
// Revision 0.1.0.3  1997/07/08  David Hopwood
// + Fixed bug in the tab expansion where parts of lines immediately
//   after a tab were being missed out.
//
// Revision 0.1.0.2  1997/07/06  David Hopwood
// + Fixed bug where "--" was being treated as a filename.
//
// Revision 0.1.0.1  1997/07/03  David Hopwood
// + Changed tab expansion to expand to a variable number of spaces,
//   controlled by -t option.
// + Added -mac option for Macintosh.
//
// Revision 0.1.0.0  1997/04/09  David Hopwood
// + Initial version.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package util;

import java.io.*;

/**
 * Utility application to fix line endings and expand tabs in text files.
 * <p>
 * Usage: <code>java util.FixLineEndings [-unix | -dos | -mac] [-t<i>tabsize</i>] [--] <i>filename</i>*</code>
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
class FixLineEndings {
    private String newline;
    private int tabsize;

    public FixLineEndings(String newline, int tabsize) {
        this.newline = newline;
        this.tabsize = tabsize;
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
                out.print(expandTabs(buf, tabsize));
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

    static String expandTabs(String s, int tabsize) {
        StringBuffer sb = new StringBuffer("");
        int p = 0;

        while (p < s.length()) {
            int oldp = p;
            p = s.indexOf("\t", p);
            if (p == -1) {
                sb.append(s.substring(oldp));
                break;
            }
            sb.append(s.substring(oldp, p));
            int nspaces = tabsize-(sb.length() % tabsize);
            for (int i = 0; i < nspaces; i++) {
                sb.append(' ');
            }
            p++;
        }
        return sb.toString();
    }

    static final String usage =
"Expand tabs to spaces, and fix line endings in a text file.\n" +
"\n" +
"Usage:\n" +
"    java util.FixLineEndings [-unix | -dos | -mac] [-t<tabsize>] [--] filename*\n" +
"\n" +
"Options:\n" +
"    -unix         convert to Unix (LF) line endings.\n" +
"    -dos          convert to DOS/Windows (CR LF) line endings.\n" +
"    -mac          convert to Macintosh (CR) line endings.\n" +
"    -t<tabsize>   set the number of spaces between tab stops (default 4).\n" +
"    --            treat everything after this as a filename, even if it\n" +
"                    begins with \"-\".\n" +
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
        int tabsize = 4;

        int i;
        for (i = 0; i < args.length; i++) {
            String arg = args[i];
            if (arg.equals("-unix")) {
                nl = "\n";
            } else if (arg.equals("-dos")) {
                nl = "\r\n";
            } else if (arg.equals("-mac")) {
                nl = "\r";
            } else if (arg.startsWith("-t")) {
                try {
                    tabsize = Integer.parseInt(arg.substring(2));
                    if (tabsize < 0) throw new NumberFormatException();
                } catch (NumberFormatException e) {
                    System.err.println("Invalid tab size: " + arg.substring(2));
                    System.exit(1);
                }
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

        FixLineEndings fle = new FixLineEndings(nl, tabsize);
        for (; i < args.length; i++) {
            fle.process(args[i]);
        }
    }
}
