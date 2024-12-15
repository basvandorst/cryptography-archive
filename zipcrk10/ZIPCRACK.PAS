program zipcrack;

{$M 16384, 0, 65536}

{-----------------------------------------------------------------------}
{																		}
{ Program ZIPCRACK Copyright 1993 by Michael A. Quinlan					}
{																		}
{ Brute force attack on PKZIP V2 encryption.							}
{ Based on the APPNOTE.TXT distributed with the registered version		}
{ of PKZIP 2.04g.														}
{																		}
{ Method: Generate all possible passwords; invoke PKUNZIP -t (test)		}
{ option to test each password.											}
{																		}
{ Input: Minimum and maximum password lengths, password character set,	}
{ Zipfile name, name of file to extract.								}
{																		}
{ Options: Interval to save last password attempted; this allows the	}
{ program to be restarted.												}
{																		}
{ Performance improvements: placing PKUNZIP and the Zipfile on a RAM	}
{ disk will improve speed. Increasing the 'save' interval will also		}
{ increase speed. Making the current directory a RAM disk is _NOT_		}
{ recommended, since a crash (power hit, etc.) will lose the saved		}
{ 'last password' and you will have to restart from scratch.			}
{																		}
{-----------------------------------------------------------------------}

uses
	DOS,
	CRT;

const
	SaveFN		= 'ZIPCRACK.$$$';		{ Save file name				}
	WorkDir		= '\ZIPCRACK';			{ Work Subdirectory				}
	MAXPW		= 256;					{ Max Password Length			}
	MAXBUF		= 32768;				{ Max buffer length				}
	K0 			= 305419896;			{ Zipfile Encryption Initializer}
	K1 			= 591751049;			{ Zipfile Encryption Initializer}
	K2 			= 878082192;			{ Zipfile Encryption Initializer}
	ZIPHDRSIG	= $04034B50;			{ Zip Local Header Signature	}
	ZDHDRSIG	= $02014B50;			{ Zip Directory Header Signature}
	ZDENDSIG	= $06054B50;			{ Zip Directory End Signature	}

const
	CrcTab : array [0..255] of LongInt =	
		(
			$00000000, $77073096, $EE0E612C, $990951BA,
			$076DC419, $706AF48F, $E963A535, $9E6495A3,
			$0EDB8832, $79DCB8A4, $E0D5E91E, $97D2D988,
			$09B64C2B, $7EB17CBD, $E7B82D07, $90BF1D91,
			$1DB71064, $6AB020F2, $F3B97148, $84BE41DE,
			$1ADAD47D, $6DDDE4EB, $F4D4B551, $83D385C7,
			$136C9856, $646BA8C0, $FD62F97A, $8A65C9EC,
			$14015C4F, $63066CD9, $FA0F3D63, $8D080DF5,
			$3B6E20C8, $4C69105E, $D56041E4, $A2677172,
			$3C03E4D1, $4B04D447, $D20D85FD, $A50AB56B,
			$35B5A8FA, $42B2986C, $DBBBC9D6, $ACBCF940,
			$32D86CE3, $45DF5C75, $DCD60DCF, $ABD13D59,
			$26D930AC, $51DE003A, $C8D75180, $BFD06116,
			$21B4F4B5, $56B3C423, $CFBA9599, $B8BDA50F,
			$2802B89E, $5F058808, $C60CD9B2, $B10BE924,
			$2F6F7C87, $58684C11, $C1611DAB, $B6662D3D,
			$76DC4190, $01DB7106, $98D220BC, $EFD5102A,
			$71B18589, $06B6B51F, $9FBFE4A5, $E8B8D433,
			$7807C9A2, $0F00F934, $9609A88E, $E10E9818,
			$7F6A0DBB, $086D3D2D, $91646C97, $E6635C01,
			$6B6B51F4, $1C6C6162, $856530D8, $F262004E,
			$6C0695ED, $1B01A57B, $8208F4C1, $F50FC457,
			$65B0D9C6, $12B7E950, $8BBEB8EA, $FCB9887C,
			$62DD1DDF, $15DA2D49, $8CD37CF3, $FBD44C65,
			$4DB26158, $3AB551CE, $A3BC0074, $D4BB30E2,
			$4ADFA541, $3DD895D7, $A4D1C46D, $D3D6F4FB,
			$4369E96A, $346ED9FC, $AD678846, $DA60B8D0,
			$44042D73, $33031DE5, $AA0A4C5F, $DD0D7CC9,
			$5005713C, $270241AA, $BE0B1010, $C90C2086,
			$5768B525, $206F85B3, $B966D409, $CE61E49F,
			$5EDEF90E, $29D9C998, $B0D09822, $C7D7A8B4,
			$59B33D17, $2EB40D81, $B7BD5C3B, $C0BA6CAD,
			$EDB88320, $9ABFB3B6, $03B6E20C, $74B1D29A,
			$EAD54739, $9DD277AF, $04DB2615, $73DC1683,
			$E3630B12, $94643B84, $0D6D6A3E, $7A6A5AA8,
			$E40ECF0B, $9309FF9D, $0A00AE27, $7D079EB1,
			$F00F9344, $8708A3D2, $1E01F268, $6906C2FE,
			$F762575D, $806567CB, $196C3671, $6E6B06E7,
			$FED41B76, $89D32BE0, $10DA7A5A, $67DD4ACC,
			$F9B9DF6F, $8EBEEFF9, $17B7BE43, $60B08ED5,
			$D6D6A3E8, $A1D1937E, $38D8C2C4, $4FDFF252,
			$D1BB67F1, $A6BC5767, $3FB506DD, $48B2364B,
			$D80D2BDA, $AF0A1B4C, $36034AF6, $41047A60,
			$DF60EFC3, $A867DF55, $316E8EEF, $4669BE79,
			$CB61B38C, $BC66831A, $256FD2A0, $5268E236,
			$CC0C7795, $BB0B4703, $220216B9, $5505262F,
			$C5BA3BBE, $B2BD0B28, $2BB45A92, $5CB36A04,
			$C2D7FFA7, $B5D0CF31, $2CD99E8B, $5BDEAE1D,
			$9B64C2B0, $EC63F226, $756AA39C, $026D930A,
			$9C0906A9, $EB0E363F, $72076785, $05005713,
			$95BF4A82, $E2B87A14, $7BB12BAE, $0CB61B38,
			$92D28E9B, $E5D5BE0D, $7CDCEFB7, $0BDBDF21,
			$86D3D2D4, $F1D4E242, $68DDB3F8, $1FDA836E,
			$81BE16CD, $F6B9265B, $6FB077E1, $18B74777,
			$88085AE6, $FF0F6A70, $66063BCA, $11010B5C,
			$8F659EFF, $F862AE69, $616BFFD3, $166CCF45,
			$A00AE278, $D70DD2EE, $4E048354, $3903B3C2,
			$A7672661, $D06016F7, $4969474D, $3E6E77DB,
			$AED16A4A, $D9D65ADC, $40DF0B66, $37D83BF0,
			$A9BCAE53, $DEBB9EC5, $47B2CF7F, $30B5FFE9,
			$BDBDF21C, $CABAC28A, $53B39330, $24B4A3A6,
			$BAD03605, $CDD70693, $54DE5729, $23D967BF,
			$B3667A2E, $C4614AB8, $5D681B02, $2A6F2B94,
			$B40BBE37, $C30C8EA1, $5A05DF1B, $2D02EF8D
		);

type
	CopyBufT	= array [1..MAXBUF] of char;{ Copy Buffer				}
	CopyBufTP	= ^COpyBufT;				{ Ptr to Copy Buffer		}
	Buf12T      = array [0..11] of Char;	{ 12-byte buffer			}
	SetOfCharT	= Set of Char;				{ Set of characters			}
	CharArrayT	= Array [0..255] of Char;	{ List of characters		}	
	CharSetT	= record					{ Character Set for Zip PW	}
					n : 0..256;				{ ..# of chars in the set	}	
					c : CharArrayT;			{ ..List of PW chars		}
					s : SetOfCharT;			{ ..PW chars in set format	}
				  end;
	ZipHdrT     = record					{ Zip File Header			}
					Sig		: LongInt;		{ ..Signature				}
					VerReqd	: Word;			{ ..Version reqd to unzip	}
					BitFlag	: Word;			{ ..Bit Flag				}
					Method	: Word;			{ ..Compress Method			}
					LModTime: Word;			{ ..Last Mod Time			}
					LModDate: Word;			{ ..Last Mod Date			}
					CRC32	: LongInt;		{ ..File CRC				}
					CmpSize : LongInt;		{ ..Compressed Size			}
					UncmpSz	: LongInt;		{ ..Uncompressed Size		}
					FNLen	: Word;			{ ..File Name Length		}
					EFLen	: Word;			{ ..Extra Field Length		}
				  end;
	ZDHdrT		= Record					{ Directory File Header		}
					Sig		: LongInt;		{ ..Signature				}
					Version	: Word;			{ ..Version made by			}
					VerReqd	: Word;			{ ..Version reqd to extract	}
					BitFlag	: Word;			{ ..Bit Flag				}
					Method	: Word;			{ ..Compression Method		}
					LModTime: Word;			{ ..Last Mod time			}
					LModDate: Word;			{ ..Last Mod Date			}
					CRC32	: LongInt;		{ ..CRC or 0				}
					CmpSize	: LongInt;		{ ..Compressed Size			}
					UncmpSz	: LongInt;		{ ..Uncompressed Size		}
					FNLen	: Word;			{ ..File Name Length		}
					EFLen	: Word;			{ ..Extra Field Length		}
					FCLen	: Word;			{ ..File Comment Length		}
					DiskNo	: Word;			{ ..Starting Disk Number	}
					IFAttr	: Word;			{ ..Internal File Attributes}
					EFAttr	: LongInt;		{ ..External File Attributes}
					LHOff	: LongInt;		{ ..Offset of local header	}
				  end;
	ZDEndT	 	= Record					{ Directory End Record		}
					Sig		: LongInt;		{ ..Signature				}
					DiskNo	: Word;			{ ..Number of this disk		}
					ZDDisk	: Word;			{ ..Disk w/ start of dir	}
					ZDETD	: Word;			{ ..Dir ents this disk		}
					ZDEnts	: Word;			{ ..Total dir ents			}
					ZDSize	: LongInt;		{ ..Dir size				}
					ZDStart	: LongInt;		{ ..Offset to start of Dir	}
					CmtLen	: Word;			{ ..Zip Comment Length		}
				  end;

var	PkunzipPath : String;				{ Path & File name for PKUNZIP	}
	ZipfilePath	: String;				{ Path & File name for Zipfile	}
	ZipfileName	: String;				{ File name for Zipfile			}
	RamPath		: String;				{ Path on RAM Drive				}
	MemberName	: String;				{ Zipfile Member Name			}
	MinPWLen	: Integer;				{ Minimum password length		}
	MaxPWLen	: Integer;				{ Maximum password length		}
	PWCharSet	: CharSetT;				{ Password character set		}
	PWSaveInt	: LongInt;				{ Password Save Interval		}
	UseRamDisk	: Boolean;				{ Use RAM Disk?					}
	RamDrive	: Char;					{ Ram Disk Drive Letter			}
	NextPW		: array [1..MAXPW] of Byte;{ Next password to try		}
	rc		 	: Integer;
	PWLen		: Integer;
	PW			: String;
	Key0		: LongInt;				{ Zip Encryption Key 0			}
	Key1		: LongInt;				{ Zip Encryption Key 1			}
	Key2		: LongInt;				{ Zip Encryption Key 2			}
	ZipBuf		: Buf12T;				{ Zip Encryption Buffer			}
	ZipFile		: File;
	ZDEnd		: ZDEndT;				{ Zip Directory End Record		}
	ZDHdr		: ZDHdrT;				{ Zip Directory Header Record	}
	ZipHdr		: ZipHdrT;				{ Zip Local Header Record		}
	Ok			: Boolean;


function crc32(crc : LongInt; c : Char) : LongInt;
begin
	crc32 := ((crc shr 8) and $00FFFFFF) xor CrcTab[(Ord(c) xor (crc and $00FF))  and $00FF];
end;

procedure ZipPWUpdateKeys(C : Char);
begin
	Key0 := crc32(Key0, C);
	Key1 := Key1 + (Key0 and $000000FF);
	Key1 := Key1 * 134775813 + 1;
	Key2 := crc32(Key2, Chr((Key1 shr 24) and $000000FF));
end;

function ZipPWDecryptByte : Char;
var Temp : Word;
begin
	Temp := (Key2 or 2) and $0000FFFF;
	ZipPWDecryptByte := Chr(((Temp * (Temp xor 1)) shr 8) and $00FF);
end;

procedure ZipPWInitKeys(PW : String);
var n : Integer;
begin
	Key0 := K0;
	Key1 := K1;
	Key2 := K2;
	for n := 1 to Length(PW) do ZipPWUpdateKeys(PW[n]);
end;

procedure ZipPWUpdateBuf(var Buf : Buf12T);
var i : Integer;
	c : Char;
begin
	for i := 0 to 11 do begin
		c := Chr(Ord(Buf[i]) xor Ord(ZipPWDecryptByte));
		ZipPWUpdateKeys(c);
		Buf[i] := c;
	end;
end;

function ZipPWCheck(PW : String; Buf : Buf12T; crc : LongInt) : Boolean;
begin
	ZipPWInitKeys(PW);
	ZipPWUpdateBuf(Buf);
	ZipPWCheck := Ord(Buf[11]) = ((crc shr 24) and $000000FF);
end;

function ZipOpen(var F : File; Name : String; var ZDEnd : ZDEndT) : Boolean;
var FMSave 	: Word;
	SeekPos	: LongInt;
begin
	if Pos('.', Name) = 0 then Name := Name + '.ZIP';
	Assign(F, Name);
	FMSave := FileMode;
	FileMode := 0;
	{$I-} Reset(F, 1); {$I+}
	FileMode := FMSave;
	if IOResult <> 0 then begin
 		WriteLn(Name, ': Cannot open file');
		ZipOpen := FALSE;
		Exit;
	end;
	SeekPos := FileSize(F) - sizeof(ZDEnd) + 1;
	while TRUE do begin
		if SeekPos <= 0 then begin
			WriteLn(Name, ': Cannot find ZIP Directory');
			Close(F);
			ZipOpen := FALSE;
			Exit;
		end;
		Dec(SeekPos);
		Seek(F, SeekPos);
		BlockRead(F, ZDEnd, sizeof(ZDEnd));
		if ZDEnd.Sig = ZDENDSIG then begin
			ZipOpen := TRUE;
			Exit;
		end;
	end;
end;

function ZipFindZDHdr(var F : File; Name : String; var ZDEnd : ZDEndT; var ZDHdr : ZDHdrT) : Boolean;
var n		: Word;
	SeekPos	: LongInt;
	Buf		: String;
	FNLen	: Integer;
	i		: Integer;
begin
	FNLen := Length(Name);
	Buf[0] := Chr(FNLen);
	for i := 1 to FNLen do Name[i] := UpCase(Name[i]);
	SeekPos := ZDEnd.ZDStart;
	for n := 1 to ZDEnd.ZDEnts do begin
		Seek(F, SeekPos);
		BlockRead(F, ZDHdr, sizeof(ZDHdr));
		if ZDHdr.FNLen = FNLen then begin
			BlockRead(F, Buf[1], FNLen);
			for i := 1 to FNLen do Buf[i] := UpCase(Buf[i]);
			if Name = Buf then begin
				ZipFindZDHdr := TRUE;
				Exit;
			end;
		end;
		SeekPos := SeekPos + sizeof(ZDHdr) + ZDHdr.FNLen + ZDHdr.EFLen + ZDHdr.FCLen;
	end;
	ZipFindZDHdr := FALSE;
end;

function ZipFindFile(var F : File; Name : String; var ZDEnd : ZDEndT; var ZDHdr : ZDHdrT; var ZipHdr : ZipHdrT) : Boolean;
var Ok : Boolean;
begin
	Ok := ZipFindZDHdr(F, Name, ZDEnd, ZDHdr);
	if not Ok then begin
		ZipFindFile := FALSE;
		Exit;
	end;
	Seek(F, ZDHdr.LHOff);
	BlockRead(F, ZipHdr, sizeof(ZipHdr));
	Seek(F, ZDHdr.LHOff + sizeof(ZipHdr) + ZipHdr.FNLen + ZipHdr.EFLen);
	ZipFindFile := TRUE;
end;

procedure AddCharToCharSet(var SC : CharSetT; c : Char);
begin
	if SC.n = 0 then SC.s := [];
	if not (c in SC.s) then begin
		SC.c[SC.n] := c;
		SC.s := SC.s + [c];
		inc(SC.n);
	end;
end;

procedure AddStringToCharSet(var SC : CharSetT; S : String);
var n : Integer;
begin
	for n := 1 to length(S) do AddCharToCharSet(SC, S[n]);
end;

procedure AddSetToCharSet(var SC : CharSetT; S : SetOfCharT);
var n : Integer;
begin
	for n := 0 to 255 do begin
		if Chr(n) in S then AddCharToCharSet(SC, Chr(n));
	end;
end;

function PromptChar(p : String; r : String) : Char;
var K	 : Char;
	S	 : String;
	Done : Boolean;
begin
	Done := FALSE;
	while not Done do begin
		Write(p, '? ');
		ReadLn(S);
		if length(s) = 0 then K := #$00
		else K := S[1];
		if Pos(K, r) <> 0 then Done := TRUE
		else WriteLn('Enter one of: ', r);
	end;
	PromptChar := K;
end;

function PromptString(p : String) : String;
var S : String;
begin
	Write(p, '? ');
	ReadLn(S);
	PromptString := S;
end;

function PromptNumber(p : String; Min, Max : LongInt) : LongInt;
var S	 : String;
	Code : Integer;
	R	 : LongInt;
	Done : Boolean;
begin
	Done := FALSE;
	while not Done do begin
		S := PromptString(p);
		val(S, R, Code);
		if (Code <> 0) or (R < Min) or (R > Max) then
			WriteLn('Enter an integer from ', Min, ' to ', Max)
		else Done := TRUE;
	end;
	PromptNumber := R;
end;

procedure PromptCharSet(p : String; var SC : CharSetT);
var K	 : Char;
begin
	SC.n := 0;
	WriteLn(p, ':');
	K := PromptChar('  Lower case letters [a..z]', 'YyNn');
	if UpCase(K) = 'Y' then AddSetToCharSet(SC, ['a'..'z']);
	K := PromptChar('  Upper case letters [A..Z]', 'YyNn');
	if UpCase(K) = 'Y' then AddSetToCharSet(SC, ['A'..'Z']);
	K := PromptChar('  Digits [0..9]', 'YyNn');
	if UpCase(K) = 'Y' then AddSetToCharSet(SC, ['0'..'9']);
	K := PromptChar('  Blank', 'YyNn');
	if UpCase(K) = 'Y' then AddStringToCharSet(SC, ' ');
	K := PromptChar('  Punctuation and special characters', 'YyNn');
	if UpCase(K) = 'Y' then AddStringToCharSet(SC, '`~!@#$%^&*()_-+=[{]}\|;:",<.>/?''');
end;

function PromptFilename(p : String; ext : String; path : String) : String;
var fn	 : String;
	fn2	 : String;
	Done : Boolean;
	i	 : Integer;
begin
	Done := FALSE;
	while not DONE do begin
		fn := PromptString(p);
		if pos('.', fn) = 0 then fn := fn + '.' + ext;
		for i:=1 to length(fn) do fn[i] := UpCase(fn[i]);
		fn2 := FSearch(fn, path);
		if fn2 = '' then WriteLn('Unable to locate ', fn)
		else Done := TRUE;
	end;
	fn := FExpand(fn2);
	for i:=1 to length(fn) do fn[i] := UpCase(fn[i]);
	PromptFilename := fn;
end;

function GetRestartData : Boolean;
var Key	  : Char;
	SaveF : File;
begin
	FillChar(NextPW, MAXPW, 0);
	MinPWLen := 0;
	MaxPWLen := 0;
	PWCharSet.n := 0;
	PWLen := 0;
	GetRestartData := FALSE;
	if (FSearch(SaveFN, '') <> '') then begin
		Key := PromptChar('Restart from last password', 'YyNn');
		if upcase(Key) = 'Y' then begin
			Assign(SaveF, SaveFN);
			FileMode := 0;
			Reset(SaveF, 1);
			FileMode := 2;
			BlockRead(SaveF, MinPWLen,  sizeof(MinPWLen));
			BlockRead(SaveF, MaxPWLen,  sizeof(MaxPWLen));
			BlockRead(SaveF, PWCharSet, sizeof(PWCharSet));
			BlockRead(SaveF, PWLen,     sizeof(PWLen));
			BlockRead(SaveF, NextPW,    sizeof(NextPW));
			Close(SaveF);
			GetRestartData := TRUE;
		end;
	end;
end;

function ExecPkunzip(cmdline : String) : Integer;
begin
	SwapVectors;
	Exec(PkunzipPath, cmdline);
	SwapVectors;
	if DosError <> 0 then begin
		WriteLn('DOS Error ', DosError, ' executing ', PkunzipPath);
		Halt(3);
	end;
	ExecPkunzip := DosExitCode;
end;

procedure GetInput;
var Key	 : Char;
	D	 : DirStr;
	N	 : NameStr;
	E	 : ExtStr;
	Done : Boolean;
	rc	 : Integer;
begin
	if not GetRestartData then begin
		MinPWLen := PromptNumber('Minimum password length', 1, MAXPW);
		if MinPWLen = MAXPW then MaxPWLen := MAXPW
		else MaxPWLen := PromptNumber('Maximum password length', MinPWLen, MAXPW);
		PromptCharSet('Password character set', PWCharSet);
		if PWCharSet.n = 0 then begin
			WriteLn('No characters in password character set!');
			Halt(3);
		end;
	end;
	PWSaveInt := PromptNumber('Password save interval', 0, 1000000);
	Key := PromptChar('Use RAM Disk', 'YyNn');
	if UpCase(Key) <> 'Y' then UseRamDisk := FALSE
	else begin
		UseRamDisk := TRUE;
		Key := PromptChar('RAM Disk drive letter', 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ');
		RamDrive := UpCase(Key);
		RamPath := RamDrive + ':' + WorkDir;
	end;

	PkunzipPath := FSearch('PKUNZIP.EXE', GetEnv('PATH'));
	if PkunzipPath <> '' then PkunzipPath := FExpand(PkunzipPath)
	else PkunzipPath := PromptFilename('PKUNZIP file name', 'EXE', GetEnv('PATH'));

	ZipfilePath := PromptFilename('Zip file name', 'ZIP', '');
	FSplit(ZipfilePath, D, N, E);
	ZipfileName := N + E;
	Done := FALSE;
	while not Done do begin
		MemberName := PromptString('File to crack');
		rc := ExecPkunzip('-# -v ' + ZipfilePath + ' ' + MemberName);
		if rc <> 0 then WriteLn('Unable to locate ', MemberName, ' in ', ZipfilePath)
		else Done := TRUE;
	end;
end;

function CopyFile(FromFile, ToFile : String) : Boolean;
var pBuf     : CopyBufTP;
	FromF    : File;
	ToF	     : File;
	Count	 : Word;
	Written	 : Word;
begin
	{$I-}
	Assign(FromF, FromFile);
	FileMode := 0;
	Reset(FromF, 1);
	FileMode := 2;
	if IOResult <> 0 then begin
		CopyFile := FALSE;
		Exit;
	end;
	Assign(ToF, ToFile);
	Rewrite(ToF, 1);
	if IOResult <> 0 then begin
		Close(FromF);
		CopyFile := FALSE;
		Exit;
	end;
	{$I+}
	New(pBuf);
	repeat
		BlockRead(FromF, pBuf^, MAXBUF, Count);
		BlockWrite(ToF, pBuf^, Count, Written);
		if Written <> Count then begin
			Close(FromF);
			Close(ToF);
			CopyFile := FALSE;
			Dispose(pBuf);
			Exit;
		end;
	until Count = 0;
	Dispose(pBuf);
	Close(FromF);
	Close(ToF);
	CopyFile := TRUE;
end;

procedure SavePW;
var SaveF : File;
begin
	Assign(SaveF, SaveFN);
	Rewrite(SaveF, 1);
	BlockWrite(SaveF, MinPWLen,  sizeof(MinPWLen));
	BlockWrite(SaveF, MaxPWLen,  sizeof(MaxPWLen));
	BlockWrite(SaveF, PWCharSet, sizeof(PWCharSet));
	BlockWrite(SaveF, PWLen,     sizeof(PWLen));
	BlockWrite(SaveF, NextPW,    sizeof(NextPW));
	Close(SaveF);
end;

function IncPW : Boolean;
var n : Integer;
begin
	n := PWLen;
	while TRUE do begin
		if n = 0 then begin
			IncPW := FALSE;
			Exit;
		end;
		inc(NextPW[n]);
		if NextPW[n] < PWCharSet.n then break
		else NextPW[n] := 0;
		dec(n);
	end;
	IncPW := TRUE;
end;

procedure BuildPW(Escape : Boolean);
var n : Integer;
	m : Integer;
	c : Char;
begin
	PW[0] := Chr(PWLen);
	m := 1;
	for n := 1 to PWLen do begin
		c := PWCharSet.c[NextPW[n]];
		if Escape and ((c = '"') or (c = '\')) then begin
			PW[m] := '\';
			inc(m);
			inc(PW[0]);
		end;
		PW[m] := c;
		inc(m);
	end;
end;

function CheckAllPWs : Boolean;
var NextSave : LongInt;
	Ok		 : Boolean;
begin

	NextSave := 1;

	while TRUE do begin

		if NextSave <> 0 then begin
			if NextSave <> 1 then dec(NextSave)
			else begin
				SavePW;
				NextSave := PWSaveInt;
			end;
		end;

		BuildPW(FALSE);
		Ok := ZipPWCheck(PW, ZipBuf, ZDHdr.Crc32);

		if Ok then begin
			BuildPW(TRUE);
			rc := ExecPkunzip('-# -t -s"' + PW + '" ' + ZipfilePath + ' ' + MemberName);
			if rc = 0 then begin
				CheckAllPWs := TRUE;
				Exit;
			end;
		end;

		Ok := IncPW;
		if not Ok then begin
			CheckAllPWs := FALSE;
			Exit;
		end;
	end;
end;

begin

	WriteLn('ZipCrack v1.0 Copyright 1993 by Michael A. Quinlan');

	GetInput;

	if UseRamDisk then begin
		{$I-} MkDir(RamPath); {$I+}
		if IOResult <> 0 then
			;
		if not CopyFile(PkunzipPath, RamPath + '\PKUNZIP.EXE') then begin
			WriteLn('Unable to copy ', PkunzipPath, ' to ', RamPath + '\PKUNZIP.EXE');
			Exit;
		end
		else PkunzipPath := RamPath + '\PKUNZIP.EXE';
		if not CopyFile(ZipfilePath, RamPath + '\' + ZipFilename) then begin
			WriteLn('Unable to copy ', ZipfilePath, ' to ', RamPath + '\' + ZipFilename);
			Exit;
		end
		else ZipfilePath := RamPath + '\' + ZipFilename;
	end;

{ Validate that PKUNZIP, the Zipfile, and the member of the Zipfile are }
{ still accessible.														}

	rc := ExecPkunzip('-# -v ' + ZipfilePath + ' ' + MemberName);
	if rc <> 0 then begin
		WriteLn('Unable to locate ', MemberName, ' in ', ZipfilePath);
		Halt(3);
	end;

	Ok := ZipOpen(ZipFile, ZipfilePath, ZDEnd);
	if not Ok then Halt(3);

	Ok := ZipFindFile(ZipFile, MemberName, ZDEnd, ZDHdr, ZIpHdr);
	if not Ok then Halt(3);

	BlockRead(ZipFile, ZipBuf, sizeof(ZipBuf));

	if PWLen = 0 then PWLen := MinPWLen;
	Writeln('Testing passwords...');
	for PWLen := PWLen to MaxPWLen do begin
		if CheckAllPWs then begin
			Writeln('Password = "', PW, '"');
			IncPW;
			SavePW;
			Halt(0);
		end;
	end;

	WriteLn('Password not found!!!');
	Halt(1);
end.
