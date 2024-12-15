{$R-}    {Range checking off}
{$S-}    {Stack checking off}
{$I+}    {I/O checking on}
{$N-}    {Don't use 80x87 numeric coprocessor}
program cryptmpj;
{ Encryption designed to exceed DES in security value and be implementable on
  an IBM PC compatible machine. }

Uses Crt, Dos;  { Include screen handling & MS DOS interface functions. }

const maxinbuffer = 16;
      ap = #39;                            { Apostrophe for write statements. }

type blocks = array[0..15] of byte;             { 16 byte (128 bit) blocks. }
     stype = array[0..9,0..15,0..255] of byte;  { Holds substitution boxes. }
     ptrstype = ^stype;  { Dynamic variable required to get beyond 64K limit. }
     string80 = string[80];                     { Long character strings. }
     string15 = string[15];                     { Short character strings. }

var initial,                  { Initialiazation vector. }
    feedback,                 { Chaining feedback array. }
    key,                      { Encryption/decryption key. }
    buffer, outbuf:  blocks;  { File read/write buffers. }
    s,                        { Substitution boxes. }
    si: ptrstype;             { Inverse substitution boxes. }
    bytesdone,                { Number of bytes done. }
    lastbytes,                { Last number of bytes done. }
    number: longint;          { Used to read in key & initialization vector. }
    que,                      { Position in file specification que. }
    i, j, k,                  { Iteration & array indexes. }
    actualread: integer;      { Actual number of bytes read in from file. }
    inputfile: file;          { File to encrypt or decrypt in place. }
    keyfile: text;            { External key file. }
    intkeyfile: file of stype; {Holds s and si. }
    keyname,                  { Name of external key file. }
    intkeyname,               { Name of internal key file. }
    path,                     { Used in parsing file specification. }
    temp: string[255];        { Used in determining encrypt/decrypt option. }
    inputfilename,            { MS-DOS name of inputfile. }
    filespec: string80;       { File specification that may include wild cards}
    fileque: array[0..15] of string80; { Names of multiple file specifications}
    name: string15;           { Name of next file to process. }
    decryption,               { True iff decryption is desired. }
    filefound: boolean;       { At least one file was found to process.}
    ch: char;                 { Character input. }
    search: searchrec;        { Used in finding matches to wild cards. }

procedure permute(x: blocks; var y: blocks);
{ This procedure is designed to make each bit of the output dependent on as
  many bytes of the input as possible, especially after repeated application.
  Each output byte takes its least significant bit from the corresponding
  input byte.  The next higher bit comes from the corresponding bit of the
  input byte to the left.  This is done until all bits of the output byte
  are filled.  Where there is no byte to the left, the byte at the far right
  is used. }

  begin
    y[0] := (x[0] and 1) or (x[1] and 2) or (x[2] and 4) or
            (x[3] and 8) or (x[4] and 16) or (x[5] and 32) or
            (x[6] and 64) or (x[7] and 128);
    y[1] := (x[1] and 1) or (x[2] and 2) or (x[3] and 4) or
            (x[4] and 8) or (x[5] and 16) or (x[6] and 32) or
            (x[7] and 64) or (x[8] and 128);
    y[2] := (x[2] and 1) or (x[3] and 2) or (x[4] and 4) or
            (x[5] and 8) or (x[6] and 16) or (x[7] and 32) or
            (x[8] and 64) or (x[9] and 128);
    y[3] := (x[3] and 1) or (x[4] and 2) or (x[5] and 4) or
            (x[6] and 8) or (x[7] and 16) or (x[8] and 32) or
            (x[9] and 64) or (x[10] and 128);
    y[4] := (x[4] and 1) or (x[5] and 2) or (x[6] and 4) or
            (x[7] and 8) or (x[8] and 16) or (x[9] and 32) or
            (x[10] and 64) or (x[11] and 128);
    y[5] := (x[5] and 1) or (x[6] and 2) or (x[7] and 4) or
            (x[8] and 8) or (x[9] and 16) or (x[10] and 32) or
            (x[11] and 64) or (x[12] and 128);
    y[6] := (x[6] and 1) or (x[7] and 2) or (x[8] and 4) or
            (x[9] and 8) or (x[10] and 16) or (x[11] and 32) or
            (x[12] and 64) or (x[13] and 128);
    y[7] := (x[7] and 1) or (x[8] and 2) or (x[9] and 4) or
            (x[10] and 8) or (x[11] and 16) or (x[12] and 32) or
            (x[13] and 64) or (x[14] and 128);
    y[8] := (x[8] and 1) or (x[9] and 2) or (x[10] and 4) or
            (x[11] and 8) or (x[12] and 16) or (x[13] and 32) or
            (x[14] and 64) or (x[15] and 128);
    y[9] := (x[9] and 1) or (x[10] and 2) or (x[11] and 4) or
            (x[12] and 8) or (x[13] and 16) or (x[14] and 32) or
            (x[15] and 64) or (x[0] and 128);
    y[10] := (x[10] and 1) or (x[11] and 2) or (x[12] and 4) or
            (x[13] and 8) or (x[14] and 16) or (x[15] and 32) or
            (x[0] and 64) or (x[1] and 128);
    y[11] := (x[11] and 1) or (x[12] and 2) or (x[13] and 4) or
            (x[14] and 8) or (x[15] and 16) or (x[0] and 32) or
            (x[1] and 64) or (x[2] and 128);
    y[12] := (x[12] and 1) or (x[13] and 2) or (x[14] and 4) or
            (x[15] and 8) or (x[0] and 16) or (x[1] and 32) or
            (x[2] and 64) or (x[3] and 128);
    y[13] := (x[13] and 1) or (x[14] and 2) or (x[15] and 4) or
            (x[0] and 8) or (x[1] and 16) or (x[2] and 32) or
            (x[3] and 64) or (x[4] and 128);
    y[14] := (x[14] and 1) or (x[15] and 2) or (x[0] and 4) or
            (x[1] and 8) or (x[2] and 16) or (x[3] and 32) or
            (x[4] and 64) or (x[5] and 128);
    y[15] := (x[15] and 1) or (x[0] and 2) or (x[1] and 4) or
            (x[2] and 8) or (x[3] and 16) or (x[4] and 32) or
            (x[5] and 64) or (x[6] and 128);
  end;

procedure ipermute(x: blocks; var y: blocks);
{ This is the inverse of the procedure permute. }

  begin
    y[0] := (x[0] and 1) or (x[15] and 2) or (x[14] and 4) or
            (x[13] and 8) or (x[12] and 16) or (x[11] and 32) or
            (x[10] and 64) or (x[9] and 128);
    y[1] := (x[1] and 1) or (x[0] and 2) or (x[15] and 4) or
            (x[14] and 8) or (x[13] and 16) or (x[12] and 32) or
            (x[11] and 64) or (x[10] and 128);
    y[2] := (x[2] and 1) or (x[1] and 2) or (x[0] and 4) or
            (x[15] and 8) or (x[14] and 16) or (x[13] and 32) or
            (x[12] and 64) or (x[11] and 128);
    y[3] := (x[3] and 1) or (x[2] and 2) or (x[1] and 4) or
            (x[0] and 8) or (x[15] and 16) or (x[14] and 32) or
            (x[13] and 64) or (x[12] and 128);
    y[4] := (x[4] and 1) or (x[3] and 2) or (x[2] and 4) or
            (x[1] and 8) or (x[0] and 16) or (x[15] and 32) or
            (x[14] and 64) or (x[13] and 128);
    y[5] := (x[5] and 1) or (x[4] and 2) or (x[3] and 4) or
            (x[2] and 8) or (x[1] and 16) or (x[0] and 32) or
            (x[15] and 64) or (x[14] and 128);
    y[6] := (x[6] and 1) or (x[5] and 2) or (x[4] and 4) or
            (x[3] and 8) or (x[2] and 16) or (x[1] and 32) or
            (x[0] and 64) or (x[15] and 128);
    y[7] := (x[7] and 1) or (x[6] and 2) or (x[5] and 4) or
            (x[4] and 8) or (x[3] and 16) or (x[2] and 32) or
            (x[1] and 64) or (x[0] and 128);
    y[8] := (x[8] and 1) or (x[7] and 2) or (x[6] and 4) or
            (x[5] and 8) or (x[4] and 16) or (x[3] and 32) or
            (x[2] and 64) or (x[1] and 128);
    y[9] := (x[9] and 1) or (x[8] and 2) or (x[7] and 4) or
            (x[6] and 8) or (x[5] and 16) or (x[4] and 32) or
            (x[3] and 64) or (x[2] and 128);
    y[10] := (x[10] and 1) or (x[9] and 2) or (x[8] and 4) or
            (x[7] and 8) or (x[6] and 16) or (x[5] and 32) or
            (x[4] and 64) or (x[3] and 128);
    y[11] := (x[11] and 1) or (x[10] and 2) or (x[9] and 4) or
            (x[8] and 8) or (x[7] and 16) or (x[6] and 32) or
            (x[5] and 64) or (x[4] and 128);
    y[12] := (x[12] and 1) or (x[11] and 2) or (x[10] and 4) or
            (x[9] and 8) or (x[8] and 16) or (x[7] and 32) or
            (x[6] and 64) or (x[5] and 128);
    y[13] := (x[13] and 1) or (x[12] and 2) or (x[11] and 4) or
            (x[10] and 8) or (x[9] and 16) or (x[8] and 32) or
            (x[7] and 64) or (x[6] and 128);
    y[14] := (x[14] and 1) or (x[13] and 2) or (x[12] and 4) or
            (x[11] and 8) or (x[10] and 16) or (x[9] and 32) or
            (x[8] and 64) or (x[7] and 128);
    y[15] := (x[15] and 1) or (x[14] and 2) or (x[13] and 4) or
            (x[12] and 8) or (x[11] and 16) or (x[10] and 32) or
            (x[9] and 64) or (x[8] and 128);
  end;

procedure makesbox(key: blocks);
{ This procedure generates internal keys by filling the substitution box array
  s^ based on the external key given as input. }
  var i, j, k: integer;
  procedure makeonebox(i, j: integer; key: blocks);
    var pos, m, n, p, startbit, bitmask, startbyte, keybyte: word;
        empty: array[0..255] of boolean;
    begin
      for m := 0 to 255 do    { The empty array is used to make sure that }
        empty[m] := true;     { each byte of the array is filled only once. }
      startbit := 1;
      startbyte := 0;
      for n := 255 downto 128 do  { n counts the number of bytes left to fill }
        begin
          keybyte := startbyte;
          bitmask := startbit;
          m := 0;
          for p := 0 to 7 do   { m is obtained by bit selection on the key }
            begin
              m := m or (key[keybyte] and bitmask);
              bitmask := bitmask shl 1;
              if bitmask > 128 then
                begin
                  bitmask := 1;
                  inc(keybyte);
                  if keybyte > 15 then keybyte := 0;
                end;
            end;
          pos := (n * m) div 255;  { pos is the position among the UNFILLED }
          m := 0;                  { components of the s^ array that the    }
          p := 0;                  { number n should be placed.  }
          while m < pos do
            begin
              inc(p);
              if empty[p] then inc(m);
            end;
          while not empty[p] do inc(p);
          s^[i, j, p] := n;
          empty[p] := false;
          startbit := startbit shl 1;  { The starting position of the bit  }
          if startbit > 128 then       { selection for the key is rotated  }
            begin                      { left one bit for the next n.      }
              startbit := 1;
              inc(startbyte);
              if startbyte > 15 then startbyte := 0
            end;
        end;
      startbyte := 0;
      startbit := 1;
      for n := 127 downto 1 do         { This half of the algorithm is the   }
        begin                          { same as the upper half, except that }
          keybyte := startbyte;        { only 7 bits are selected for m.     }
          bitmask := startbit;
          m := 0;
          for p := 0 to 6 do
            begin
              m := m or (key[keybyte] and bitmask);
              bitmask := bitmask shl 1;
              if bitmask > 64 then
                begin
                  bitmask := 1;
                  keybyte := keybyte + 3;
                  if keybyte > 15 then keybyte := keybyte - 16;
                end;
            end;
          pos := (n * m) div 127;
          m := 0;
          p := 0;
          while m < pos do
            begin
              inc(p);
              if empty[p] then inc(m);
            end;
          while not empty[p] do inc(p);
          s^[i, j, p] := n;
          empty[p] := false;
          startbit := startbit shl 1;
          if startbit > 64 then
            begin
              startbit := 1;
              inc(startbyte);
              if startbyte > 15 then startbyte := 0
            end;
        end;
      p := 0;
      while not empty[p] do
        inc(p);
      s^[i,j,p] := 0;
    end;

  begin
    new(s);
    for i := 0 to 9 do
      for j := 0 to 15 do
        begin
          write(#13,'Filling substitution boxes for round ',i+1,' of 10, byte ',
                j+1,' of 16.  ');
          makeonebox(i, j, key);
          permute(key, key);            { Shuffle key bit positions.        }
          for k := 0 to 15 do           { Run key through last s-box before }
            key[k] := s^[i, j, key[k]]; { making the next s-box.            }
        end;
    writeln;
  end;

procedure makesi;
{ This procedure fills the inverse substitution box array si^.  It is not
  necessary to call this procedure unless the decryption mode is used.  }
  var i, j, k: integer;
  begin
    new(si);
    for i := 0 to 9 do
      for j := 0 to 15 do
        for k := 0 to 255 do
          si^[i, j, s^[i, j, k]] := k;
  end;

{$I CRYPTINC.PAS}   { Include additional file handling & user interface. }

procedure substitute(round: integer; x: blocks; var y: blocks);
var i: integer;
begin
  for i := 0 to 15 do
    y[i] := s^[round,i,x[i]];
end;

procedure isubst(round: integer; x: blocks; var y: blocks);
var i: integer;
begin
  for i := 0 to 15 do
    y[i] := si^[round,i,x[i]];
end;

procedure encrypt(x: blocks; var y: blocks);
{ Encrypt a block of 16 bytes. }
var z: blocks;
begin
  substitute(0, x, y);
  permute(y, z);
  substitute(1, z, y);
  permute(y, z);
  substitute(2, z, y);
  permute(y, z);
  substitute(3, z, y);
  permute(y, z);
  substitute(4, z, y);
  permute(y, z);
  substitute(5, z, y);
  permute(y, z);
  substitute(6, z, y);
  permute(y, z);
  substitute(7, z, y);
  permute(y, z);
  substitute(8, z, y);
  permute(y, z);
  substitute(9, z, y);
end;

procedure decrypt(x: blocks; var y: blocks);
{ Decrypt a block of 16 bytes. }
var z: blocks;
begin
  isubst(9, x, y);
  ipermute(y, z);
  isubst(8, z, y);
  ipermute(y, z);
  isubst(7, z, y);
  ipermute(y, z);
  isubst(6, z, y);
  ipermute(y, z);
  isubst(5, z, y);
  ipermute(y, z);
  isubst(4, z, y);
  ipermute(y, z);
  isubst(3, z, y);
  ipermute(y, z);
  isubst(2, z, y);
  ipermute(y, z);
  isubst(1, z, y);
  ipermute(y, z);
  isubst(0, z, y);
end;

begin
  startup;  { Initialize files, call makesbox (and makesi if required). }
  que := 0;
  while (que < 16) and (fileque[que] <> '') do
    begin
      filespec := fileque[que];
      inputfilename := filespec;
      path := '';
      repeat
        i := pos('\',inputfilename);
        if i > 0 then
          begin
            path := path + copy(inputfilename, 1, i);
            inputfilename := copy(inputfilename, i+1, length(inputfilename));
          end;
      until i = 0;
      findfirst(filespec, $22, search);
      while doserror = 0 do
        begin
          inputfilename := path + search.name;
          assign(inputfile, inputfilename);
          reset(inputfile,1);
          filefound := true;
          bytesdone := 0;
          lastbytes := 0;
          for i := 0 to 15 do
            feedback[i] := initial[i];
          if decryption then
           begin
            writeln('Decrypting ',inputfilename);
            while not eof(inputfile) do
              begin
                blockread(inputfile,buffer,maxinbuffer,actualread);
                encrypt(feedback,outbuf);
                { Note that decrypt is not ever called when using block
                  chaining with ciphertext feedback.  It would be called
                  if the electronic codebook mode were used, instead. }
                for i := 0 to 15 do
                  begin
                    outbuf[i] := outbuf[i] xor buffer[i];
                    feedback[i] := buffer[i]
                  end;
                seek(inputfile, filepos(inputfile) - actualread);
                blockwrite(inputfile, outbuf, actualread);
                bytesdone := bytesdone + actualread;
                if bytesdone - lastbytes > 256 then
                  begin
                    write(#13, bytesdone:9,' bytes decrypted. ');
                    lastbytes := bytesdone
                  end;
              end;
            writeln(#13, bytesdone:9,' bytes decrypted.  ');
           end
          else
           begin
            writeln('Encrypting ',inputfilename);
            while not eof(inputfile) do
              begin
                blockread(inputfile,buffer,maxinbuffer,actualread);
                encrypt(feedback,outbuf);
                for i := 0 to 15 do
                  begin
                    outbuf[i] := outbuf[i] xor buffer[i];
                    feedback[i] := outbuf[i]
                  end;
                seek(inputfile, filepos(inputfile) - actualread);
                blockwrite(inputfile, outbuf, actualread);
                bytesdone := bytesdone + actualread;
                if bytesdone - lastbytes > 256 then
                  begin
                    write(#13, bytesdone:9,' bytes encrypted. ');
                    lastbytes := bytesdone
                  end;
              end;
            writeln(#13, bytesdone:9,' bytes encrypted.  ');
           end;
          close(inputfile);
          findnext(search);
        end;
      inc(que);
    end;
  if not filefound then writeln('No matching files found.');
  writeln('Done.');
end.

