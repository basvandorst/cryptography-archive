/*
// $Log: AddHistory.java,v $
// Revision 1.1.1.1  1997/11/03 22:36:55  hopwood
// + Imported to CVS (tagged as 'start').
//
// $Endlog$
*/

package util;

import java.io.*;

class AddHistory {
    AddHistory(String newline, String sourcename, String destname) {
        try {
            File source = new File(sourcename);
            if (!source.exists()) {
                System.err.println(sourcename + " does not exist");
                return;
            }
            if (source.isDirectory()) {
                System.err.println(sourcename + " is a directory");
                return;
            }
            File dest = new File(destname);
            if (!dest.exists()) {
                System.err.println(destname + " does not exist");
                return;
            }
            if (dest.isDirectory()) {
                System.err.println(destname + " is a directory");
                return;
            }
            File backup = new File(destname + ".bak");
            backup.delete();
            dest.renameTo(backup);
            BufferedReader in1 = new BufferedReader(new FileReader(source));
            BufferedReader in2 = new BufferedReader(new FileReader(backup));
            PrintWriter out = new PrintWriter(new BufferedWriter(
                                  new FileWriter(dest)));
            String buf;
            while ((buf = in1.readLine()) != null && buf.startsWith("//")) {
                out.print(buf);
                out.print(newline);
            }
            while ((buf = in2.readLine()) != null && buf.startsWith("//")) {
                /* skip */
            }
            while (buf != null) {
                out.print(buf);
                out.print(newline);
                buf = in2.readLine();
            }
            out.close();
            in1.close();
            in2.close();
            if (dest.length() == 0) {
                System.err.println("Write failed for " + destname + "?");
            } else {
                backup.delete();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    static final String usage =
        "Usage: java util.AddHistory [-unix | -dos | --] sourcefile destfile";

    public static void main(String[] args) {
        if (args.length == 0) {
            System.err.println(usage);
            System.exit(1);
        }

        int i = 0;
        String nl, arg = args[0];
        if (arg.equalsIgnoreCase("-unix")) {
            nl = "\n";
        } else if (arg.equalsIgnoreCase("-dos")) {
            nl = "\r\n";
        } else {
            nl = System.getProperty("line.separator");
        }
        if (arg.startsWith("-")) i++;
        if (args.length < i+2) {
            System.err.println(usage);
            System.exit(1);
        }

        new AddHistory(nl, args[i], args[i+1]);
    }
}
