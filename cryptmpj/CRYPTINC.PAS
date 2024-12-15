procedure getinitial;
  begin
            assign(keyfile, keyname);
            {$I- I/O checking off }
            reset(keyfile);
            {$I+ I/O checking back on }
            if ioresult <> 0 then
              begin
                writeln('Using default initialization vector.');
              end
            else
              begin
                for i := 15 downto 0 do
                  if not eof(keyfile) then read(keyfile, number);
                for i := 15 downto 0 do
                  begin
                    if eof(keyfile) then
                      writeln('Initialization vector component ',i,' defaulting to ',initial[i])
                    else
                      read(keyfile, number);
                      if (number < 0) or (number > 255) then
                        begin
                          writeln('Error reading in initialization vector component ',i,' from ',keyname);
                          close(keyfile);
                          halt;
                        end
                      else
                        initial[i] := number;
                  end;
              end;
  end;

procedure keygen;
  begin
        assign(keyfile, keyname);
        {$I- I/O checking off }
        reset(keyfile);
        {$I+ I/O checking back on }
        if ioresult <> 0 then
          begin
            writeln('I can',ap,'t find key file ',keyname);
            halt
          end;
        for i := 15 downto 0 do
          begin
            if eof(keyfile) then
              begin
                writeln('There are not enough numbers in the key file ',keyname);
                halt
              end;
            read(keyfile, number);
            if (number < 0) or (number > 255) then
              begin
                writeln('Error reading in key byte ',i,' from ',keyname,' (0 is LSB at right)');
                close(keyfile);
                halt;
              end
            else
              key[i] := number;
          end;
        for i := 15 downto 0 do
          begin
            if eof(keyfile) then
              writeln('Initialization vector component ',i,' defaulting to ',initial[i])
            else
              read(keyfile, number);
              if (number < 0) or (number > 255) then
                begin
                  writeln('Error reading in initialization vector component ',i,' from ',keyname);
                  close(keyfile);
                  halt;
                end
              else
                initial[i] := number;
          end;
        close(keyfile);
        writeln('Generating internal keys.');
        makesbox(key);
        if intkeyname <> 'NO' then
          begin
            assign(intkeyfile, intkeyname);
            rewrite(intkeyfile);
            write(intkeyfile, s^);
          end;
  end;

procedure getinput;
  var choice: char;
  begin
      writeln('Optional command line syntax:');
      writeln('CRYPTMPJ keyfile intkeyfile|NO E|D filespec [filespec] [filespec]');
      writeln(' keyfile is the name of an ASCII text file that contains the key as a list of');
      writeln('  numbers 16 numbers between 0 and 255 ($FF) and the initialization vector as');
      writeln('  16 more numbers, all separated by spaces.');
      writeln(' intkeyfile is a file to store the internal keys.  If this file exists, then');
      writeln('  the key in it will be used, thus saving the few minutes it takes');
      writeln('  to generate internal keys from an external key.  If the file does not exist,');
      writeln('  it will be created unless NO is specified.');
      writeln(' E is to encrypt, D is to decrypt.');
      writeln(' filespec is a file specification that may include drive, path, & wildcards.');
      write('Press a key to continue . . .');
      ch := readkey;
      if ch = chr(0) then ch := readkey;
      clrscr;
      writeln('This program encrypts and decrypts in place.  In other words, the plain text');
      writeln('will be overwritten with the cipher text, or vise versa.  If you do not want');
      writeln('to do this, exit this program by pressing the ESC key and use the DOS copy');
      writeln('command to make a copy of the input file before running this program again,');
      writeln('and run this program on the copy.');
      writeln;
      write('Do you want to Encrypt or Decrypt (E, D, or ESC)?  ');
      ch := readkey;
      if ch = chr(0) then ch := readkey;
      if ch = chr(27) {ESC} then halt;
      if upcase(ch) = 'D' then
        begin
          writeln('Decrypt');
          decryption := true
        end
      else
        begin
          writeln('Encrypt');
          decryption := false
        end;
      writeln;
      writeln('If you would like to encrypt more than one file with the same key and');
      writeln('initialization vector, you may include the wild card characters * and ? in');
      writeln('the file name(s).  * matches any number of characters, and ? matches any');
      writeln('one character in the file name.');
      writeln;
      writeln('Please enter up to 16 file specifications.  Press enter when you are done.');
      writeln;
      que := -1;
      repeat
        inc(que);
        if decryption then
          write('File(s) to decrypt?  ')
        else
          write('File(s) to encrypt?  ');
        readln(fileque[que]);
      until (que >= 15) or (fileque[que] = '');
      clrscr;
      writeln('You may enter the key interactively, specify a key file with an ASCII');
      writeln('key and initialization vector in it, or specify a key file with internal');
      writeln('keys in it generated on a previous run of CRYPTMPJ.');
      writeln;
      writeln('Do you want to:');
      writeln(' 1.  Type the key');
      writeln(' 2.  use an External key file, or');
      write(' 3.  use an Internal key file?  ');
      choice := readkey;
      if ord(choice) = 0 then choice := readkey;
      if ord(choice) = 27 then halt;
      choice := upcase(choice);
      writeln(choice);
      writeln;
      if (choice = '2') or (choice = 'E') then
       begin
        write('Name of external key file with ASCII key?  ');
        readln(keyname);
        writeln('You may generate an internal binary key file to save time later.  It must be');
        writeln('protected like the external key file, as they are two different forms of the');
        writeln('same key, and either may be used to unlock the data.');
        writeln;
        write('Do you want to create an internal key file?  ');
        ch := readkey;
        if ord(ch) = 0 then
          begin
            ch := readkey;
            halt
          end;
        if upcase(ch) = 'N' then
          begin
            writeln('No.');
            intkeyname := 'NO'
          end
        else
          begin
            if ord(ch) = 27 then halt;
            writeln('Yes.');
            write('Internal key file name?  ');
            readln(intkeyname);
          end;
        keygen;
       end
      else
       begin
        if (choice = '3') or (choice = 'I') then
         begin
           writeln;
           write('Internal key file name?  ');
           readln(intkeyname);
           writeln;
           assign(intkeyfile,intkeyname);
           {$I- I/O checking off }
           reset(intkeyfile);
           {$I+ I/O checking back on }
           if (ioresult <> 0) then
             begin
               writeln('I can',ap,'t find ',intkeyname);
               halt
             end;
           new(s);
           {$I- I/O checking off }
           read(intkeyfile, s^);
           {$I+ I/O checking back on }
           if ioresult <> 0 then
             begin
               writeln(intkeyname,' is not a valid internal key file name.');
               halt;
             end
           else
             begin
               close(intkeyfile);
               writeln('Internal keys read in from ',intkeyname);
             end;

           write('Do you want to enter an initialization vector?  ');
           ch := readkey;
           if ch = chr(0) then ch := readkey;
           if upcase(ch) = 'Y' then
             begin
               writeln('Yes.');
               writeln('Do you want to:');
               writeln('1.  Type it in, or');
               write('2.  Read it in from an external key file?  ');
               ch := readkey;
               if ord(ch) = 27 then halt;
               ch := upcase(ch);
               if (ch = '1') or (ch = 'T') then
                 begin
                   writeln('Type it.');
                   writeln('Please enter your initialization vector as 16 integers from 0 to 255,');
                   writeln('separated by spaces.');
                   write(chr(16));
                   for i := 15 downto 0 do read(initial[i]);
                   writeln;
                 end
               else
                 begin
                   writeln('Read it.');
                   write('External key file name?  ');
                   readln(keyname);
                   getinitial
                 end
             end
           else
             begin
               writeln('No.');
               writeln('Using default initialization vector:');
               for i := 15 downto 0 do write(initial[i],' ');
               writeln;
               writeln;
             end;


         end
        else
         begin
           writeln('Please enter the key to use as 16 numbers between 0 and 255, with spaces in');
           writeln('between them.  If you prefer, you may use hexadecimal numbers between $0 and');
           writeln('$FF if you preface the numbers with a dollar sign.');
           write(chr(16));
           for i := 15 downto 0 do read(key[i]);
           clrscr;
           write('Do you want to enter an initialization vector?  ');
           ch := readkey;
           if ch = chr(0) then ch := readkey;
           if upcase(ch) = 'Y' then
             begin
               writeln('Yes.');
               writeln;
               writeln('Please enter your initialization vector in the same format as the key.');
               write(chr(16));
               for i := 15 downto 0 do read(initial[i]);
               writeln;
             end
           else
             begin
               writeln('No.');
               writeln('Using default initialization vector:');
               for i := 15 downto 0 do write(initial[i],' ');
               writeln;
               writeln;
             end;
           write('Do you want to generate an internal key file to save time next time?  ');
           ch := readkey;
           if ord(ch) = 27 then halt;
           if ord(ch) = 0 then ch := readkey;
           if upcase(ch) = 'N' then
             begin
               writeln('No.');
               writeln('Generating internal keys . . .');
               makesbox(key)
             end
           else
             begin
               writeln('Yes.');
               writeln;
               intkeyname := '';
               readln(intkeyname);
               while intkeyname = '' do
                 begin
                   write('Name of internal key file to make?  ');
                   readln(intkeyname);
                 end;
               writeln('Generating internal keys . . .');
               makesbox(key);
               writeln('Saving internal keys in ',intkeyname);
               assign(intkeyfile, intkeyname);
               rewrite(intkeyfile);
               write(intkeyfile, s^);
               close(intkeyfile);
               writeln;
             end;
         end;
       end;
  end;

procedure commandline;
  var code: integer;
  begin
    intkeyname := paramstr(2);
    for i := 1 to length(intkeyname) do intkeyname[i] := upcase(intkeyname[i]);
    if intkeyname <> 'NO' then
      begin
        assign(intkeyfile,intkeyname);
        {$I- I/O checking off }
        reset(intkeyfile);
        {$I+ I/O checking back on }
      end;
    if (ioresult = 0) and (intkeyname <> 'NO') then
      begin
        new(s);
        {$I- I/O checking off }
        read(intkeyfile, s^);
        {$I+ I/O checking back on }
        if ioresult <> 0 then
          begin
            writeln(intkeyname,' is not a valid internal key file name.');
            halt;
          end
        else
          begin
            close(intkeyfile);
            writeln('Internal keys read in from ',intkeyname);
            keyname := paramstr(1);
            getinitial;
          end;
        close(keyfile);
      end
    else
      begin
        keyname := paramstr(1);
        keygen;
      end;
    temp := paramstr(3);
    if upcase(temp[1]) = 'D' then
      decryption := true
    else
      decryption := false;
    (* if decryption then makesi; *)
    que := 0;
    for i := 4 to paramcount do
      begin
        if que < 16 then fileque[que] := paramstr(i);
        inc(que);
      end;
  end;

procedure startup;
begin
  clrscr;
  writeln('ษออออออออออออออออออออออออออออออออออออออหออออออออออออออออออออออออออออออออออออป');
  writeln('บ  CRYPTMPJ   1.1  3 December 1988     บ  The MPJ Encryption Algorithm is   บ');
  writeln('บ  Encrypts files in place.            บ  donated to the public domain.     บ');
  writeln('บ  Copy files first if you want to     บ  This specific program is          บ');
  writeln('บ  keep the original.  For a           บ  protected by copyright.  While    บ');
  writeln('บ  registered copy of this program,    บ  this program is believed to be    บ');
  writeln('บ  please send your request and $20    บ  reliable, it is the user',ap,'s        บ');
  writeln('บ  to:                                 บ  responsibility to determine its   บ');
  writeln('บ                                      บ  fitness for use.  No liability    บ');
  writeln('บ  Mike Johnson                        บ  will be assumed by the author.    บ');
  writeln('บ  P. O. Box 1151                      บ  Copyright (C) 1988 Mike Johnson.  บ');
  writeln('บ  Longmont, CO 80502-1151             บ  All rights reserved.              บ');
  writeln('ศออออออออออออออออออออออออออออออออออออออสออออออออออออออออออออออออออออออออออออผ');
  for que := 0 to 15 do
    fileque[que] := '';
  filemode := 2;
  initial[0] := 103;
  initial[1] := 21;
  initial[2] := 12;
  initial[3] := 79;
  initial[4] := 107;
  initial[5] := 171;
  initial[6] := 102;
  initial[7] := 182;
  initial[8] := 108;
  initial[9] := 146;
  initial[10] := 247;
  initial[11] := 65;
  initial[12] := 15;
  initial[13] := 56;
  initial[14] := 126;
  initial[15] := 80;
  filefound := false;
  if paramcount < 4 then
    getinput
  else
    commandline;
end;
