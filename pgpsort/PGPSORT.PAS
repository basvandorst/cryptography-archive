program PGPSort;

{                                                                           }
{ PGPSort v1.04 by St†le Schumacher/Felix Softworks 1994                    }
{                                                                           }
{ Syntax  : PGPSORT +U  [-R] [<keyring>] - sorts on user ID                 }
{           PGPSORT +K  [-R] [<keyring>] - sorts on 8-digit key ID          }
{           PGPSORT +K6 [-R] [<keyring>] - sorts on 6-digit key ID          }
{           PGPSORT +S  [-R] [<keyring>] - sorts on key size                }
{           PGPSORT +D  [-R] [<keyring>] - sorts on date of creation        }
{           PGPSORT      -R  [<keyring>] - removes bad keys (no sorting)    }
{                                                                           }
{           Use '-' instead of '+' to sort in descending order              }
{           Use '-R' to remove bad keys                                     }
{                                                                           }
{ Synopsis: Sorts PGP key rings.                                            }
{                                                                           }
{ History : v1.04 - Added capability to remove bad keys                     }
{           v1.03 - Sorts secret keyrings, ascending/descending order       }
{           v1.02 - Sorts key IDs according to last 8 digits (PGP 2.6)      }
{           v1.01 - Improved sorting of 'unstandard' user IDs               }
{           v1.00 - Original version                                        }
{                                                                           }
{ Examples: PGPSORT +u             - Sorts the main public key ring         }
{                                    (PUBRING.PGP) according to the last    }
{                                    names in the user IDs on the keys      }
{           PGPSORT -d BIGRING.PGP - Sorts the key ring BIGRING.PGP         }
{                                    according to the date of creation of   }
{                                    the keys, newest keys first            }
{                                                                           }
{ The files PGPSORT.PAS and PGPSORT.EXE are placed in the public domain and }
{ may be freely distributed and modified. Any questions should be addressed }
{ to the author at:                                                         }
{                                                                           }
{                 Internet       : staalesc@ifi.uio.no                      }
{                                                                           }
{                 RIME           : STALE SCHUMACHER ->334                   }
{                                                                           }
{                 Snail mail     : St†le Schumacher                         }
{                                  Gyldenlovesgate 24                       }
{                                  N-0260 Oslo                              }
{                                  NORWAY                                   }
{                                                                           }

{$A+,B-}
{$M 65520,0,655360}

uses
  Crt,Dos;

const
  Version      = '1.04';
  RevisionDate = '1994/09/05';

  MaxKeys      = 10000;

type
  KeyPtr = ^KeyRec;
  KeyRec = record
             fPos,
             length    : longint;
             secret    : boolean;
             size      : integer;
             keyID     : longint;
             date      : longint;
             userName  : string[25];
             remove    : boolean;
           end;

var
  SortCriterion: (KeyID8,KeyID6,UserID,Size,Date,None);
  SortOrder    : (Ascending,Descending);
  RemoveBadKeys: boolean;

  keys,removed : integer;
  key          : array[0..MaxKeys] of KeyPtr;

procedure Error(const msg: string);
  begin
    WriteLn(msg);
    Halt(1);
  end;

function FileExists(const fileName: PathStr): boolean;
  var
    DirInfo: SearchRec;
  begin
    FindFirst(fileName,Archive,DirInfo);
    FileExists:=(DosError=0) and (fileName<>'');
  end;

function NoDirInName(const fileName: PathStr): boolean;
  var
    i: Integer;
  begin
    NoDirInName:=True;
    for i:=1 to Length(fileName) do
      if fileName[i] in [':','\'] then
        NoDirInName:=False;
  end;

function DirWithSlash(const dir: DirStr): DirStr;
  begin
    if (dir<>'') and (Copy(dir,Length(dir),1)<>'\') then
      DirWithSlash:=dir+'\'
    else
      DirWithSlash:=dir;
  end;

function UpperCase(s: string): string; near; assembler;
  asm
    PUSH    DS
    LDS     SI,[BP+4]
    LES     DI,[BP+8]
    CLD
    LODSB
    STOSB
    XOR     CH,CH
    MOV     CL,AL
    JCXZ    @3
  @1:
    LODSB
    CMP     AL,'a'
    JB      @2
    CMP     AL,'z'
    JA      @2
    SUB     AL,'a'-'A'
  @2:
    STOSB
    LOOP    @1
  @3:
    POP     DS
  end;

procedure QuickSort;

  function Sorted(a,b: integer): boolean;
    begin
      if SortOrder=Ascending then
        case SortCriterion of
          KeyID6,
          KeyID8: if (key[a]^.keyID < 0) and (key[b]^.keyID > 0) then
                    Sorted := false
                  else if (key[a]^.keyID > 0) and (key[b]^.keyID < 0) then
                    Sorted := true
                  else
                    Sorted := key[a]^.keyID < key[b]^.keyID;
          UserID: Sorted := key[a]^.userName < key[b]^.userName;
          Size  : Sorted := key[a]^.size < key[b]^.size;
          Date  : Sorted := key[a]^.date < key[b]^.date;
        end
      else {SortOrder=Descending}
        case SortCriterion of
          KeyID6,
          KeyID8: if (key[a]^.keyID > 0) and (key[b]^.keyID < 0) then
                    Sorted := false
                  else if (key[a]^.keyID < 0) and (key[b]^.keyID > 0) then
                    Sorted := true
                  else
                    Sorted := key[a]^.keyID > key[b]^.keyID;
          UserID: Sorted := key[a]^.userName > key[b]^.userName;
          Size  : Sorted := key[a]^.size > key[b]^.size;
          Date  : Sorted := key[a]^.date > key[b]^.date;
        end;
    end;

  procedure SwapKeys(a,b: integer);
    var
      x: KeyPtr;
    begin
      x:=key[a];
      key[a]:=key[b];
      key[b]:=x;
    end;

  procedure Sort(left,right: integer);
    var
      i,j: integer;
    begin
      i:=left; j:=right;
      key[0]^:=key[(left+right) div 2]^;
      repeat
        while Sorted(i,0) do inc(i);
        while Sorted(0,j) do dec(j);
        if i<=j then
          begin
            SwapKeys(i,j);
            inc(i); dec(j);
          end;
      until i>j;
      if left<j then
        Sort(left,j);
      if i<right then
        Sort(i,right);
    end;

  begin
    Sort(1,keys);
  end;

function ReadPacketLength(var f: file; CTB: byte): longint;
  const
    LengthOfLengthField: array[0..3] of byte = (1,2,4,0);
  var
    i,l,b : byte;
    Length: longint;
  begin
    l:=LengthOfLengthField[CTB and 3];
    Length:=0;
    for i:=1 to l do
       begin
         BlockRead(f,b,1);
         Length:=(Length SHL 8)+b;
       end;
    ReadPacketLength:=Length;
  end;

function SwapLong(l: longint): longint; near; assembler;
  asm
    MOV DH,[BP+4];
    MOV DL,[BP+5];
    MOV AH,[BP+6];
    MOV AL,[BP+7];
  end;

function ReadLongint(var f: file): longint;
  var
    l : longint;
  begin
    BlockRead(f,l,4);
    ReadLongint:=SwapLong(l);
  end;

function WordAsHex(w: word): string;
  const
    HexDigit: array [0..$F] of Char = '0123456789ABCDEF';
  begin
    WordAsHex[0]:=char(4);
    WordAsHex[1]:=HexDigit[Hi(W) shr $4];
    WordAsHex[2]:=HexDigit[Hi(W) and $F];
    WordAsHex[3]:=HexDigit[Lo(W) shr $4];
    WordAsHex[4]:=HexDigit[Lo(W) and $F];
  end;

function WordAsStr(w: word; n: byte): string;
  var
    s: string;
  begin
    Str(w:n,s);
    WordAsStr:=s;
  end;

function UnixDate(seconds: longint): string;
  var
    days,month,year: word;

  function DaysInYear: word;
    begin
      if (year mod 4)=0 then
        DaysInYear:=366
      else
        DaysInYear:=365;
    end;

  function DaysInMonth: word;
    begin
      case month of
         1: DaysInMonth:=31;
         2: if DaysInYear=366 then
              DaysInMonth:=29
            else
              DaysInMonth:=28;
         3: DaysInMonth:=31;
         4: DaysInMonth:=30;
         5: DaysInMonth:=31;
         6: DaysInMonth:=30;
         7: DaysInMonth:=31;
         8: DaysInMonth:=31;
         9: DaysInMonth:=30;
        10: DaysInMonth:=31;
        11: DaysInMonth:=30;
        12: DaysInMonth:=31;
      end;
    end;

  function DayAsStr(w: word): string;
    var
      s: string;
    begin
      Str(w,s);
      if w<10 then
        DayAsStr:='0'+s
      else
        DayAsStr:=s;
    end;

  begin
    seconds:=seconds shr 1; {Longints are signed!}
    days:=seconds div 43200;
    year:=1970;
    while days>=daysInYear do
      begin
        dec(days,daysInYear);
        inc(year);
      end;
    month:=1;
    while days>=daysInMonth do
      begin
        dec(days,daysInMonth);
        inc(month);
      end;
    UnixDate:=WordAsStr(year,0)+'/'+DayAsStr(month)++'/'+DayAsStr(days+1);
  end;

function LongAsHex(w: Longint): string;
  begin
    LongAsHex:=WordAsHex(w shr 16)+WordAsHex(w and 65535);
  end;

procedure SortKeyRing(const keyRing: PathStr);
  var
    f,newF       : file;
    b,CTB        : byte;
    fileLength,
    fPos,keyPos,
    packetLength : longint;
    userID       : string;
    isPubRing,
    firstUserID  : boolean;
    dir          : DirStr;
    name         : NameStr;
    ext          : ExtStr;
    buf          : array[1..4096] of byte;
    bakName      : string;
    i,
    bytes        : integer;
    drive        : byte;
    removeAll    : array[1..3] of boolean;
  const
    NoUserID     = 1;
    BogusUserID  = 2;
    SecretKey    = 3;

  procedure ShowProgress(fPos,fLen: longint);
    begin
      GotoXY(10,WhereY);
      Write((fPos * 100) div fLen:3,'%');
    end;

  procedure PromptForRemoval(keyType: byte; secret: boolean);
    var
      c: char;
    begin
      GotoXY(1,WhereY);
      case keyType of
        NoUserID   : WriteLn('The following key has no attached userID:');
        BogusUserID: WriteLn('The following key seems to have a bogus user ID:');
        SecretKey  : WriteLn('The keyring contains a secret key:');
      end;
      WriteLn;
      if secret then Write('sec') else Write('pub');
      WriteLn('  ',key[keys]^.size:4,'/',LongAsHex(key[keys]^.keyID),' ',
              UnixDate(key[keys]^.date),'  ',userID);
      WriteLn;
      if removeAll[keyType] then
        begin
          c:='Y';
          WriteLn('Key removed.');
        end
      else
        begin
          Write('Remove it <Y/N/A>? ');
          repeat
            repeat until KeyPressed;
            c:=Upcase(ReadKey);
          until c in ['Y','N','A'];
          WriteLn(c);
          if c='A' then
            begin
              c:='Y';
              removeAll[keyType]:=true;
            end;
        end;
      if c='Y' then
        begin
          key[keys]^.remove:=true;
          inc(removed);
        end;
      WriteLn;
      Write('Reading: ');
    end;

  begin
    keys:=0;
    removed:=0;
    fPos:=0;
    removeAll[NoUserID]   :=false;
    removeAll[BogusUserID]:=false;
    removeAll[SecretKey]  :=false;

    Assign(f,keyRing); {$I-} Reset(f,1); {$I+}
    If IOResult<>0 then
      Error(keyRing+': access denied.');
    fileLength:=FileSize(f);

    If keyRing[2]=':' then drive:=byte(keyRing[1])-64 else drive:=0;
    If DiskFree(drive)<fileLength then
      begin
        Close(f);
        Error('Disk full: cannot backup old keyring.');
      end;

    GetMem(key[0],SizeOf(KeyRec));
    Write('Reading: ');
    while fPos<fileLength do
      begin
        if fPos mod 20=0 then
          ShowProgress(fPos,fileLength);
        Seek(f,fPos); keyPos:=fPos;
        BlockRead(f,CTB,1);
        PacketLength:=ReadPacketLength(f,CTB); fPos:=FilePos(f);
        CTB:=CTB and 60;

        case CTB of
          20,24:                  {Public/Secret key packet}
            begin
              if RemoveBadKeys and (keys>0) and (userID='') and (key[keys]^.remove=false) then
                PromptForRemoval(NoUserID,key[keys]^.secret);
              inc(keys);
              if keys=1 then
                isPubRing:=CTB=24;
              if keys>MaxKeys then
                Error('Out of memory: '+keyRing+' is too long to sort.');
              GetMem(key[keys],SizeOf(KeyRec));
              if key[keys]=nil then
                Error('Out of memory: '+keyRing+' is too long to sort.');
              key[keys]^.fPos:=keyPos;
              key[keys-1]^.length:=keyPos-key[keys-1]^.fPos;
              key[keys]^.secret:=CTB=20;
              firstUserID:=true;
              Seek(f,FilePos(f)+1);
              key[keys]^.date:=ReadLongint(f);
              Seek(f,FilePos(f)+3);
              BlockRead(f,key[keys]^.size,2); key[keys]^.size:=Swap(key[keys]^.size);
              Seek(f,FilePos(f)+((key[keys]^.size+7) div 8)-4);
              key[keys]^.keyID:=ReadLongint(f);
              if SortCriterion=KeyID6 then
                key[keys]^.keyID:=key[keys]^.keyID and $FFFFFF;
              key[keys]^.userName:=''; userID:='';
              key[keys]^.remove:=false;
            end;
          52:                     {User ID packet}
            begin
              userID[0]:=char(PacketLength);
              BlockRead(f,userID[1],PacketLength);
              if RemoveBadKeys and (key[keys]^.remove=false) and key[keys]^.secret and isPubRing then
                PromptForRemoval(SecretKey,true);
              if RemoveBadKeys and (key[keys]^.remove=false) then
                if (Pos('UCK',userID)>0) or (Pos('DICK',userID)>0)
                or (Pos('SHIT',userID)>0) or (Pos('BEGIN',userID)>0)
                or (Pos('@whitehouse.gov',userID)>0) or (Length(userID)>150) then
                  PromptForRemoval(BogusUserID,key[keys]^.secret);
              if firstUserID then
                begin
                  firstUserID:=false;
                  userID:=UpperCase(userID);
                  b:=2;
                  while (b<=Length(userID)) and (b>0) and
                  not (((userID[b] in ['0'..'9']) and (userID[b-1]<>'-'))
                       or (userID[b] in ['[','(','{','<','>','+','#','*',':','/','|'])
                       or (Copy(userID,b,2)=' -')
                       or (Copy(userID,b,3)=' II')
                       or (Copy(userID,b,4)=' JR.')
                       or (Copy(userID,b,5)=' M.D.')) do
                    inc(b);
                  userID[0]:=CHAR(b-1);
                  if userID[1] in ['<','*'] then userID:=Copy(userID,2,255);
                  while userID[Length(userID)] in [' ',',','.'] do dec(userID[0]);

                  {Derive name from internet address?}
                  if (Pos(' ',userID)=0) and (Pos('@',userID)>0) then
                    begin
                      userID[0]:=char(Pos('@',userID)-1);
                      b:=Pos('.',userID);
                      if b>0 then userID[b]:=' ';
                    end
                  else
                    for b:=1 to Length(userID)-1 do
                      if (userID[b]='.') and (userID[b+1]<>' ') then
                        userID:=Copy(userID,1,b)+' '+Copy(userID,b+1,255);

                  {Split first and last names}
                  if Pos(' ',userID)=0 then
                    key[keys]^.userName:=userID
                  else
                    begin
                      b:=Pos(', ',userID);
                      if (b>0) and (b+1=Pos(' ',userID)) then
                        key[keys]^.userName:=userID
                      else
                        begin
                          b:=Length(userID);
                          while (userID[b]<>' ') do dec(b);
                          key[keys]^.userName:=Copy(userID,b+1,Length(userID))+', '+Copy(userID,1,b-1);
                        end;
                    end;
                  if (Copy(userID,1,2)='!!') or (Copy(userID,1,2)='**') then
                    firstUserID:=true;
                end;
            end;
          48:                     {Keyring trust packet}
            {Nothing to do};
          8:                      {Signature packet}
            {Nothing to do};
          else                    {Unknown packet}
            begin
              WriteLn(CTB);
              Error(keyRing+' is not a key ring.');
            end;
        end;
        fPos:=fPos+PacketLength;
      end;
    key[keys]^.length:=FileSize(f)-key[keys]^.fPos;
    if RemoveBadKeys and (userID='') and (key[keys]^.remove=false) then
      PromptForRemoval(NoUserID,key[keys]^.secret);
    Close(f);
    if keys=0 then
      Error(keyRing+' is not a key ring.');

    {Sort keys}
    if SortCriterion<>None then
      begin
        GotoXY(1,WhereY); Write('Sorting:   0%');
        QuickSort;
      end;

    {Backup old keyring}
    FSplit(KeyRing,Dir,Name,Ext);
    bakName:=Dir+Name+'.BAK';
    Assign(f,bakName); {$I-} Erase(f); {$I+}
    if IOResult<>0 then {Old backup not found};
    Assign(f,KeyRing); Rename(f,bakName);

    {Write new keyring}
    GotoXY(1,WhereY); Write('Writing:');
    Assign(f,bakName); Reset(f,1);
    Assign(newF,KeyRing); Rewrite(newF,1);
    for i:=1 to keys do
      if not key[i]^.remove then
        begin
          if i mod 20=0 then
            ShowProgress(i,keys);
          Seek(f,key[i]^.fPos);
          while key[i]^.length>0 do
            begin
              bytes:=key[i]^.length;
              if bytes>SizeOf(buf) then bytes:=SizeOf(buf);
              BlockRead(f,buf,bytes);
              BlockWrite(newF,buf,bytes);
              dec(key[i]^.length,bytes);
            end;
        end;
    Close(f); Close(newF);
    GotoXY(1,WhereY);

    for i:=0 to keys do
      FreeMem(key[i],SizeOf(KeyRec));
  end;

procedure WriteSyntax;
  begin
    WriteLn('Syntax: PGPSORT +U  [-R] [<keyring>] - tries to sort on last name in User ID');
    WriteLn('        PGPSORT +K  [-R] [<keyring>] - sorts on 8-digit Key ID (PGP 2.6)');
    WriteLn('        PGPSORT +K6 [-R] [<keyring>] - sorts on 6-digit Key ID (PGP 2.3)');
    WriteLn('        PGPSORT +S  [-R] [<keyring>] - sorts on key Size');
    WriteLn('        PGPSORT +D  [-R] [<keyring>] - sorts on Date of creation');
    WriteLn('        PGPSORT      -R  [<keyring>] - Removes bad keys (no sorting)');
    WriteLn;
    WriteLn('        Use ''-'' instead of ''+'' to sort in descending order');
    WriteLn('        Use ''-R'' to remove bad keys');
    Halt(1);
  end;

var
  mode,
  KeyRing: string;

begin
  WriteLn;
  WriteLn('PGPSort v',Version,' (C) 1994 Felix Softworks');
  WriteLn('Written by St†le Schumacher ',RevisionDate);
  WriteLn;

  KeyRing:='PUBRING.PGP';
  SortCriterion:=None;
  RemoveBadKeys:=false;

  if ParamCount in [1..3] then
    begin
      mode:=ParamStr(1);
      if mode[1] in ['+','-'] then
        begin
          case mode[1] of
            '+': SortOrder:=Ascending;
            '-': SortOrder:=Descending;
          end;
          mode:=UpperCase(Copy(mode,2,255));
          if (mode='K') or (mode='KEYID') or (mode='K8') or (mode='KEYID8') then
            SortCriterion:=KeyID8
          else if (mode='K6') or (mode='KEYID6') then
            SortCriterion:=KeyID6
          else if (mode='U') or (mode='USERID') then
            SortCriterion:=UserID
          else if (mode='S') or (mode='SIZE') then
            SortCriterion:=Size
          else if (mode='D') or (mode='DATE') then
            SortCriterion:=Date
          else if (mode='R') then
            RemoveBadKeys:=true
          else if (mode='H') or (mode='?') then
            WriteSyntax
          else
            begin
              WriteLn('Illegal parameter: ',ParamStr(1)); WriteLn;
              WriteSyntax;
            end;
        end
      else
        WriteSyntax;

      if ParamCount>=2 then
        if UpperCase(ParamStr(2))='-R' then
          begin
            RemoveBadKeys:=true;
            if ParamCount=3 then
              KeyRing:=UpperCase(ParamStr(3));
          end
        else if ParamCount=3 then
          WriteSyntax
        else
          KeyRing:=UpperCase(ParamStr(2));
    end
  else
    WriteSyntax;

  if not FileExists(KeyRing) then
    begin
      if NoDirInName(KeyRing) then
        KeyRing:=DirWithSlash(UpperCase(GetEnv('PGPPATH')))+KeyRing;
      if not FileExists(KeyRing) then
        Error(KeyRing+' not found.');
    end;

  SortKeyRing(KeyRing);

  if SortCriterion<>None then
    Write(KeyRing,' sorted on ');
  case SortCriterion of
    KeyID6,
    KeyID8: WriteLn('key ID.');
    UserID: WriteLn('user ID.');
    Size  : WriteLn('size.');
    Date  : WriteLn('date.');
  end;
  if RemoveBadKeys then
    if removed=1 then
      WriteLn('1 key removed.')
    else
      WriteLn(removed,' keys removed.');
end.
