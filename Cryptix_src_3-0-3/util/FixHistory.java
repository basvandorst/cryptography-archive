// $Log: FixHistory.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// Revision 0.1.0.3  1997/10/26  David Hopwood
// + Remove @version tags, and add a CVS Revision line before the first
//   "@" tag.
//
// Revision 0.1.0.2  1997/10/26  David Hopwood
// + Fixed bug where the Log: line would contain the full path of the file,
//   not the filename, if '/' is used as a file separator on Windows.
//
// Revision 0.1.0.1  1997/10/25  David Hopwood
// + Support "##" comments for shell scripts, and "/*" "//" "*/" comments
//   for C files.
// + Allow files to be processed twice.
//
// Revision 0.1.0.0  1997/10/25  David Hopwood
// + Start of history.
//
// $Endlog$
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team.  All rights reserved.
 */

package util;

import java.io.*;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;

/**
 * Utility application to convert old history entries to the desired format for
 * Cryptix 2.2.2.
 * <p>
 * Usage: <code>java util.FixHistory [-unix | -dos | -mac] [--] <i>filename</i>*</code>
 * <p>
 *
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 *
 * <p><b>$Revision: 1.1.1.1 $</b>
 * @author  David Hopwood
 */
class FixHistory {
    private static final String CVSCOMMENT = "$";
    private static String cvsquote(String s) { return CVSCOMMENT + s + CVSCOMMENT; }

    private static final String[] months = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    private String newline;

    public FixHistory(String newline) {
        this.newline = newline;
    }

    public void process(String filename) {
        try {
            File file = new File(filename);
            if (file.isDirectory()) {
                System.err.println(filename + " is a directory");
                return;
            }
            String shortname = file.getName();
            int p = shortname.lastIndexOf('/');
            if (p != -1)
                shortname = shortname.substring(p+1);

            File backup = new File(filename + ".bak");
            backup.delete();
            file.renameTo(backup);
            BufferedReader in = new BufferedReader(new FileReader(backup));
            PrintWriter out = new PrintWriter(new BufferedWriter(
                                  new FileWriter(file)));

            String buf = in.readLine();
            String comment = "//";
            String endcomment = null;
            if (buf != null) {
                if (buf.startsWith("/*")) {
                    out.print("/*" + newline);
                    endcomment = "*/";
                } else if (buf.startsWith("##")) {
                    comment = "##";
                }
            }

            boolean flag = false; // ignore lines until the first history entry.
            for (; buf != null; buf = in.readLine()) {
                if (!buf.startsWith(comment) && endcomment == null)
                    break;

                if (buf.length() >= 4 && buf.charAt(2) == ' ' &&
                    (Character.isDigit(buf.charAt(3)) ||
                     buf.substring(3).startsWith(CVSCOMMENT) ||
                     buf.substring(3).startsWith("Revision"))) {
                    if (!flag) {
                        out.print(comment + " " + cvsquote("Log: " + shortname + ",v ") +
                            newline);
                        flag = true;
                    }
                    if (buf.substring(3).startsWith(CVSCOMMENT)) {
                        continue;
                    }
                    if (buf.substring(3).startsWith("Revision")) {
                        out.print(comment + buf.substring(2) + newline);
                        continue;
                    }

                    StringTokenizer tok = new StringTokenizer(buf.substring(3), " \t");
                    String version = "?",
                           dayofmonth = "??",
                           month = "??",
                           year = "????",
                           author = "?";

                    try {
                        version = tok.nextToken();
                        dayofmonth = tok.nextToken();
                        month = tok.nextToken();
                        year = tok.nextToken();
                        author = tok.nextToken();
                        while (tok.hasMoreTokens()) {
                            author = author + " " + tok.nextToken();
                        }
                    } catch (NoSuchElementException e) {}

                    for (int i = 0; i < months.length; i++) {
                        if (month.startsWith(months[i])) {
                            month = "0" + i;
                            month = month.substring(month.length()-2);
                            break;
                        }
                    }
                    dayofmonth = "00" + dayofmonth;
                    dayofmonth = dayofmonth.substring(dayofmonth.length()-2);

                    out.print(comment + " Revision 0." + version + "  " +
                        year + "/" + month + "/" + dayofmonth + "  " + author + newline);
                } else if (buf.length() >= 2) {
                    if (endcomment != null && buf.indexOf(endcomment) != -1)
                        break;

                    if (!flag && buf.indexOf("lock") != -1) {
                        out.print(comment + buf.substring(2) + newline + comment +
                            newline);
                    } else if (flag) {
                        out.print(comment);
                        // change spacing of left margin
                        for (int i = 2; i < buf.length(); i++) {
                            char c = buf.charAt(i);
                            if (!Character.isWhitespace(c)) {
                                out.print(c == '+' ? " " : "   ");
                                out.print(buf.substring(i));
                                break;
                            }
                        }
                        out.print(newline);
                    }
                }
            }

            if (!flag) {
                out.print(comment + " " + cvsquote("Log: " + shortname + ",v ") + newline);
                flag = true;
            }
            out.print(comment + " " + cvsquote("Endlog") + newline);
            if (endcomment != null) {
                out.print(endcomment + newline);
                buf = in.readLine();
            }

            flag = false;
            String revisionLine = " * <p><b>" + cvsquote("Revision") + "</b>";
            for (; buf != null; buf = in.readLine()) {
                if (!flag && buf.equals(revisionLine))
                    continue;

                if (buf.startsWith(" * @")) {
                    if (!flag) {
                        out.print(revisionLine + newline);
                        flag = true;
                    }
                    if (buf.substring(4).startsWith("version"))
                        continue;
                }
                out.print(buf + newline);
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

    static final String usage =
"Convert old history entries to the desired format for Cryptix 2.2.2.\n" +
"\n" +
"Usage:\n" +
"    java util.FixHistory [-unix | -dos | -mac] [--] filename*\n" +
"\n" +
"Options:\n" +
"    -unix         convert to Unix (LF) line endings.\n" +
"    -dos          convert to DOS/Windows (CR LF) line endings.\n" +
"    -mac          convert to Macintosh (CR) line endings.\n" +
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

        FixHistory fh = new FixHistory(nl);
        for (; i < args.length; i++) {
            fh.process(args[i]);
        }
    }
}
