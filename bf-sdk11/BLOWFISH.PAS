
{
        BLOWFISH.PAS
        This Unit implements the Blowfish Algorithm as it is defined by
        Bruce Schneier in DDJ 10/95 and "Applied Cryptography, 2nd Edition"

        (c)1996 AtmuteSoft
        programmer  : Markus Hahn
}

unit Blowfish;

interface

{**************************************************************************}

{ The following part was originally declared in a special
  unit called GENERAL.PAS, so it could be used throughout
  the whole application. It isn't necessary, but you might
  want to regenerate GENERAL.PAS }

type

{ own basic data types,
  to avoid system dependencies }

  ULONG = Longint;     { unsigned 32bit }
  UINT  = Word;        { unsigned 16bit }
  UCHAR = Byte;        { unsigned 8bit }
  BOOL  = Word;        { 16bit boolean }


{ redef. of special true/false for better portability }
const

  MAXDATA = 65535;


{ some large-buffer declarations }
type
P_UCHAR_Buffer = ^T_UCHAR_Buffer;
T_UCHAR_Buffer = array[0..MAXDATA-1] of UCHAR;
P_UINT_Buffer = ^T_UINT_Buffer;
T_UINT_Buffer = array[0..(MAXDATA div 2)-1] of UINT;
P_ULONG_Buffer = ^T_ULONG_Buffer;
T_ULONG_Buffer = array[0..(MAXDATA div 4)-1] of ULONG;




{**************************************************************************}



{ The exported functions }

procedure Blowfish_Init(pKey : P_UCHAR_Buffer; unKeySize : UINT);
procedure Blowfish_ECBEncrypt(pBuffer : P_ULONG_Buffer; unCount : UINT);
procedure Blowfish_ECBDecrypt(pBuffer : P_ULONG_Buffer; unCount : UINT);
procedure Blowfish_CBCEncrypt(pBuffer : P_ULONG_Buffer; unCount : UINT;
                              var ulCBCLeft, ulCBCRight : ULONG);
procedure Blowfish_CBCDecrypt(pBuffer : P_ULONG_Buffer; unCount : UINT;
                              var ulCBCLeft, ulCBCRight : ULONG);
procedure Blowfish_Done;
procedure Blowfish_SetBoxes(pBuffer : P_ULONG_Buffer);
procedure Blowfish_GetBoxes(pBuffer : P_ULONG_Buffer);
function  Blowfish_SetRounds(Rounds : UINT) : UINT;
function  Blowfish_GetBoxPointer : Pointer;
function  Blowfish_WeakKey : BOOL;

implementation

{$L bfeng386.obj}


procedure Blowfish_Init(pKey : P_UCHAR_Buffer; unKeySize : UINT); external;
procedure Blowfish_ECBEncrypt(pBuffer : P_ULONG_Buffer; unCount : UINT); external;
procedure Blowfish_ECBDecrypt(pBuffer : P_ULONG_Buffer; unCount : UINT); external;
procedure Blowfish_CBCEncrypt(pBuffer : P_ULONG_Buffer; unCount : UINT;
                              var ulCBCLeft, ulCBCRight : ULONG); external;
procedure Blowfish_CBCDecrypt(pBuffer : P_ULONG_Buffer; unCount : UINT;
                              var ulCBCLeft, ulCBCRight : ULONG); external;
procedure Blowfish_Done; external;
procedure Blowfish_SetBoxes(pBuffer : P_ULONG_Buffer); external;
procedure Blowfish_GetBoxes(pBuffer : P_ULONG_Buffer); external;
function  Blowfish_SetRounds(Rounds : UINT): UINT; external;
function  Blowfish_GetBoxPointer : Pointer; external;
function  Blowfish_WeakKey : BOOL; external;

{ No Init }
begin
end.
