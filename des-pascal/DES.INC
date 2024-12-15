(***************************************************************************)
(* Module title : DES implementation                                       *)
(* Written by   : C.Dunkley                                                *)
(***************************************************************************)

type
  Bit       = 0..1;
  BitType   = array [1..64] of Bit;
  Byte8Type = array [1..8] of byte;

var
  R,KE,KD : array [1..16] of BitType;
  SBox : array [1..512] of byte;
  Key1 : array [1..64] of byte;
  S    : BitType;
  BTB  : array [0..255] of array [1..8] of Bit;
(***************************************************************************)

(***************************************************************************)
(* Used to read in the values of the eight S-Boxes. (from des.dat)         *)
(***************************************************************************)
procedure SData(SNum : integer;
                Data : string);

var
  L1 : integer;

begin (* 1 SData *)
for L1 := 1 to 128 do
  SBox[SNum+L1]:=(ord(Data[L1]) shl 2)-191
end;  (* 1 SData *)
(***************************************************************************)

(***************************************************************************)
(* Initialise ordering arrays. Needs to be called once by application      *)
(* program BEFORE any encryption/decryption takes place.                   *)
(* Needs only to be called ONCE.                                           *)
(***************************************************************************)
procedure InitDes;

var
  Off,L1 : integer;
  Data   : string;

begin (* 1 InitDes *)
Data:='iaYQIA91jbZRJB:2kc[SKC;3ld\Tog_WOG?7nf^VNF>6me]UME=5LD<4';
for L1 := 1 to 56 do
  Key1[L1]:=ord(Data[L1])-48;
SData(0,'>4=12?;83:6<59070?74>2=1:6<;953841>8=62;?<973:50?<8249175;3>:06='+
     '?18>6;34972=<05:3=47?28><01:69;50>7;:4=158<6932?=8:13?42;67<05>9');
SData(128,':09>63?51=<7;428=709346:285><;?1=6498?30;12<5:>71:=069874?>3;52<'+
      '7=>3069:1285;<4?=8;56?03472<1:>9:690<;7=?13>52843?06:1=8945;<72>');
SData(256,'2<417:;6853?=0>9>;2<47=150?:3986421;:=78?9<5630>;8<71>2=6?09:453'+
      '<1:?92680=34>75;:?427<9561=>0;389>?528<3704:1=;6432<95?:;>17608=');
SData(384,'4;2>?08=3<975:61=0;7491:>35<2?8614;=<37>:?6805926;=814:7950?>23<'+
        '=2846?;1:93>50<71?=8:374<56;0>927;419<>206:=?35821>74:8=?<90356;');
Off:=1;
for L1:=0 to 15 do          (* set up S box binary conversion lookup table *)
  begin (* 2 *)
  if L1>7          then S[Off]  :=1 else S[Off]:=0;
  if odd(L1 div 4) then S[Off+1]:=1 else S[Off+1]:=0;
  if odd(L1 div 2) then S[Off+2]:=1 else S[Off+2]:=0;
  if odd(L1)       then S[Off+3]:=1 else S[Off+3]:=0;
  inc(Off,4)
  end;  (* 2 *)
for L1:=0 to 255 do                  (* set up bytes to bits lookup tables *)
  begin (* 2 *)
  if L1>127          then BTB[L1][1]:=1 else BTB[L1][1]:=0;
  if odd(L1 div 64)  then BTB[L1][2]:=1 else BTB[L1][2]:=0;
  if odd(L1 div 32)  then BTB[L1][3]:=1 else BTB[L1][3]:=0;
  if odd(L1 div 16)  then BTB[L1][4]:=1 else BTB[L1][4]:=0;
  if odd(L1 div  8)  then BTB[L1][5]:=1 else BTB[L1][5]:=0;
  if odd(L1 div  4)  then BTB[L1][6]:=1 else BTB[L1][6]:=0;
  if odd(L1 div  2)  then BTB[L1][7]:=1 else BTB[L1][7]:=0;
  if odd(L1)         then BTB[L1][8]:=1 else BTB[L1][8]:=0
  end   (* 2 *)
end;  (* 1 InitDes *)
(***************************************************************************)

(***************************************************************************)
(* Convert the supplied key into 64 bits.                                  *)
(***************************************************************************)
procedure InitKey(NKey:Byte8Type);

var
  L1,L2    :integer;
  Key,Temp : BitType;

begin (* 1 InitKey *)
L2:=1;
for L1:=1 to 8 do                   (* convert the 8 byte key into 64 bits *)
  begin (* 2 *)
  Temp[L2]  :=BTB[NKey[L1]][1];
  Temp[L2+1]:=BTB[NKey[L1]][2];
  Temp[L2+2]:=BTB[NKey[L1]][3];
  Temp[L2+3]:=BTB[NKey[L1]][4];
  Temp[L2+4]:=BTB[NKey[L1]][5];
  Temp[L2+5]:=BTB[NKey[L1]][6];
  Temp[L2+6]:=BTB[NKey[L1]][7];
  inc(L2,8)
  end;  (* 2 *)
for L1 :=1 to 56 do
  Key[L1] := Temp[Key1[L1]];
for L1 := 1 to 16 do
  begin (* 2 *)
  if not(L1 in [1,2,9,16]) then
    begin (* 3 Rotate Left *)
    Temp[1]:=Key[1];
    Temp[2]:=Key[2];
    Temp[29]:=Key[29];
    Temp[30]:=Key[30];
    for L2 := 1 to 54 do
      Key[L2]:=Key[L2+2];
    Key[27] := Temp[1];
    Key[28] := Temp[2];
    Key[55] := Temp[29];
    Key[56] := Temp[30]
    end   (* 3 Rotate Left *)
  else
    begin (* 3 Rotate Left *)
    Temp[1]:=Key[1];
    Temp[29]:=Key[29];
    for L2 := 1 to 55 do
      Key[L2]:=Key[L2+1];
    Key[28] := Temp[1];
    Key[56] := Temp[29]
    end;  (* 3 Rotate Left *)
  KE[L1]:=Key;
  KD[17-L1]:=Key
  end  (* 2 *)
end;  (* InitKey *)
(***************************************************************************)

(***************************************************************************)
(* DES routine, used to encipher InputText.                                *)
(***************************************************************************)
procedure DES (    Txt  : Byte8Type;
               var Out : Byte8Type);

var
  L1,Off : integer;
  CP,P   : BitType;

begin (* 1 DES *)
 P[1] := BTB[Txt[8]][2];  P[2] := BTB[Txt[7]][2];  P[3] := BTB[Txt[6]][2];
 P[4] := BTB[Txt[5]][2];  P[5] := BTB[Txt[4]][2];  P[6] := BTB[Txt[3]][2];
 P[7] := BTB[Txt[2]][2];  P[8] := BTB[Txt[1]][2];  P[9] := BTB[Txt[8]][4];
P[10] := BTB[Txt[7]][4]; P[11] := BTB[Txt[6]][4]; P[12] := BTB[Txt[5]][4];
P[13] := BTB[Txt[4]][4]; P[14] := BTB[Txt[3]][4]; P[15] := BTB[Txt[2]][4];
P[16] := BTB[Txt[1]][4]; P[17] := BTB[Txt[8]][6]; P[18] := BTB[Txt[7]][6];
P[19] := BTB[Txt[6]][6]; P[20] := BTB[Txt[5]][6]; P[21] := BTB[Txt[4]][6];
P[22] := BTB[Txt[3]][6]; P[23] := BTB[Txt[2]][6]; P[24] := BTB[Txt[1]][6];
P[25] := BTB[Txt[8]][8]; P[26] := BTB[Txt[7]][8]; P[27] := BTB[Txt[6]][8];
P[28] := BTB[Txt[5]][8]; P[29] := BTB[Txt[4]][8]; P[30] := BTB[Txt[3]][8];
P[31] := BTB[Txt[2]][8]; P[32] := BTB[Txt[1]][8]; P[33] := BTB[Txt[8]][1];
P[34] := BTB[Txt[7]][1]; P[35] := BTB[Txt[6]][1]; P[36] := BTB[Txt[5]][1];
P[37] := BTB[Txt[4]][1]; P[38] := BTB[Txt[3]][1]; P[39] := BTB[Txt[2]][1];
P[40] := BTB[Txt[1]][1]; P[41] := BTB[Txt[8]][3]; P[42] := BTB[Txt[7]][3];
P[43] := BTB[Txt[6]][3]; P[44] := BTB[Txt[5]][3]; P[45] := BTB[Txt[4]][3];
P[46] := BTB[Txt[3]][3]; P[47] := BTB[Txt[2]][3]; P[48] := BTB[Txt[1]][3];
P[49] := BTB[Txt[8]][5]; P[50] := BTB[Txt[7]][5]; P[51] := BTB[Txt[6]][5];
P[52] := BTB[Txt[5]][5]; P[53] := BTB[Txt[4]][5]; P[54] := BTB[Txt[3]][5];
P[55] := BTB[Txt[2]][5]; P[56] := BTB[Txt[1]][5]; P[57] := BTB[Txt[8]][7];
P[58] := BTB[Txt[7]][7]; P[59] := BTB[Txt[6]][7]; P[60] := BTB[Txt[5]][7];
P[61] := BTB[Txt[4]][7]; P[62] := BTB[Txt[3]][7]; P[63] := BTB[Txt[2]][7];
P[64] := BTB[Txt[1]][7];
for L1 := 1 to 16 do
  begin (* 2 process 64 bits *)
  CP:=P;
   P[1]:=P[1+32];   P[2]:=P[2+32];   P[3]:=P[3+32];   P[4]:=P[4+32];
   P[5]:=P[5+32];   P[6]:=P[6+32];   P[7]:=P[7+32];   P[8]:=P[8+32];
   P[9]:=P[9+32];  P[10]:=P[10+32]; P[11]:=P[11+32]; P[12]:=P[12+32];
  P[13]:=P[13+32]; P[14]:=P[14+32]; P[15]:=P[15+32]; P[16]:=P[16+32];
  P[17]:=P[17+32]; P[18]:=P[18+32]; P[19]:=P[19+32]; P[20]:=P[20+32];
  P[21]:=P[21+32]; P[22]:=P[22+32]; P[23]:=P[23+32]; P[24]:=P[24+32];
  P[25]:=P[25+32]; P[26]:=P[26+32]; P[27]:=P[27+32]; P[28]:=P[28+32];
  P[29]:=P[29+32]; P[30]:=P[30+32]; P[31]:=P[31+32]; P[32]:=P[32+32];
  Off:=SBox[(CP[32+32] xor R[L1][14]) shl 5+
            (CP[5+32]  xor R[L1][5])  shl 4+
            (CP[1+32]  xor R[L1][17]) shl 3+
            (CP[2+32]  xor R[L1][11]) shl 2+
            (CP[3+32]  xor R[L1][24]) shl 1+
            (CP[4+32]  xor R[L1][1])+1];
  P[9+32] :=CP[9]  xor S[Off];   P[17+32]:=CP[17] xor S[Off+1];
  P[23+32]:=CP[23] xor S[Off+2]; P[31+32]:=CP[31] xor S[Off+3];
  Off:=SBox[65+(CP[32+4] xor R[L1][3])  shl 5+
               (CP[32+9] xor R[L1][10]) shl 4+
               (CP[32+5] xor R[L1][28]) shl 3+
               (CP[32+6] xor R[L1][15]) shl 2+
               (CP[32+7] xor R[L1][6])  shl 1+
               (CP[32+8] xor R[L1][21])];
  P[13+32]:=CP[13] xor S[Off];   P[28+32]:=CP[28] xor S[Off+1];
  P[2+32] :=CP[2]  xor S[Off+2]; P[18+32]:=CP[18] xor S[Off+3];
  Off:=SBox[129+(CP[32+8]  xor R[L1][23]) shl 5+
                (CP[32+13] xor R[L1][8])  shl 4+
                (CP[32+9]  xor R[L1][19]) shl 3+
                (CP[32+10] xor R[L1][12]) shl 2+
                (CP[32+11] xor R[L1][4])  shl 1+
                (CP[32+12] xor R[L1][26])];
  P[24+32]:=CP[24] xor S[Off];   P[16+32]:=CP[16] xor S[Off+1];
  P[30+32]:=CP[30] xor S[Off+2]; P[6+32]:=CP[6] xor S[Off+3];
  Off:=SBox[193+(CP[32+12] xor R[L1][16]) shl 5+
                (CP[32+17] xor R[L1][2])  shl 4+
                (CP[32+13] xor R[L1][7])  shl 3+
                (CP[32+14] xor R[L1][27]) shl 2+
                (CP[32+15] xor R[L1][20]) shl 1+
                (CP[32+16] xor R[L1][13])];
  P[26+32]:=CP[26] xor S[Off];   P[20+32]:=CP[20] xor S[Off+1];
  P[10+32]:=CP[10] xor S[Off+2]; P[1+32]:=CP[1] xor S[Off+3];
  Off:=SBox[257+(CP[32+16] xor R[L1][41]) shl 5+
                (CP[32+21] xor R[L1][55]) shl 4+
                (CP[32+17] xor R[L1][52]) shl 3+
                (CP[32+18] xor R[L1][31]) shl 2+
                (CP[32+19] xor R[L1][37]) shl 1+
                (CP[32+20] xor R[L1][47])];
  P[8+32] :=CP[8]  xor S[Off];   P[14+32]:=CP[14] xor S[Off+1];
  P[25+32]:=CP[25] xor S[Off+2]; P[3+32] :=CP[3]  xor S[Off+3];
  Off:=SBox[321+(CP[32+20] xor R[L1][30]) shl 5+
                (CP[32+25] xor R[L1][48]) shl 4+
                (CP[32+21] xor R[L1][40]) shl 3+
                (CP[32+22] xor R[L1][51]) shl 2+
                (CP[32+23] xor R[L1][45]) shl 1+
                (CP[32+24] xor R[L1][33])];
  P[4+32] :=CP[4]  xor S[Off];   P[29+32]:=CP[29] xor S[Off+1];
  P[11+32]:=CP[11] xor S[Off+2]; P[19+32]:=CP[19] xor S[Off+3];
  Off:=SBox[385+(CP[32+24] xor R[L1][44]) shl 5+
                (CP[32+29] xor R[L1][53]) shl 4+
                (CP[32+25] xor R[L1][49]) shl 3+
                (CP[32+26] xor R[L1][39]) shl 2+
                (CP[32+27] xor R[L1][56]) shl 1+
                (CP[32+28] xor R[L1][34])];
  P[32+32]:=CP[32] xor S[Off];   P[12+32]:=CP[12] xor S[Off+1];
  P[22+32]:=CP[22] xor S[Off+2]; P[7+32] :=CP[7]  xor S[Off+3];
  Off:=SBox[449+(CP[32+28] xor R[L1][46]) shl 5+
                (CP[32+1] xor R[L1][32])  shl 4+
                (CP[32+29] xor R[L1][42]) shl 3+
                (CP[32+30] xor R[L1][50]) shl 2+
                (CP[32+31] xor R[L1][36]) shl 1+
                (CP[32+32] xor R[L1][29])];
  P[5+32] :=CP[5]  xor S[Off];   P[27+32]:=CP[27] xor S[Off+1];
  P[15+32]:=CP[15] xor S[Off+2]; P[21+32]:=CP[21] xor S[Off+3]
  end;  (* 2 process 64 bits *)
Out[1]:= P[8] shl 7 + P[40] shl 6 + P[16] shl 5 + P[48] shl 4+
        P[24] shl 3 + P[56] shl 2 + P[32] shl 1 + P[64];
Out[2]:= P[7] shl 7 + P[39] shl 6 + P[15] shl 5 + P[47] shl 4+
        P[23] shl 3 + P[55] shl 2 + P[31] shl 1 + P[63];
Out[3]:= P[6] shl 7 + P[38] shl 6 + P[14] shl 5 + P[46] shl 4+
        P[22] shl 3 + P[54] shl 2 + P[30] shl 1 + P[62];
Out[4]:= P[5] shl 7 + P[37] shl 6 + P[13] shl 5 + P[45] shl 4+
        P[21] shl 3 + P[53] shl 2 + P[29] shl 1 + P[61];
Out[5]:= P[4] shl 7 + P[36] shl 6 + P[12] shl 5 + P[44] shl 4+
        P[20] shl 3 + P[52] shl 2 + P[28] shl 1 + P[60];
Out[6]:= P[3] shl 7 + P[35] shl 6 + P[11] shl 5 + P[43] shl 4+
        P[19] shl 3 + P[51] shl 2 + P[27] shl 1 + P[59];
Out[7]:= P[2] shl 7 + P[34] shl 6 + P[10] shl 5 + P[42] shl 4+
        P[18] shl 3 + P[50] shl 2 + P[26] shl 1 + P[58];
Out[8]:= P[1] shl 7 + P[33] shl 6 +  P[9] shl 5 + P[41] shl 4+
        P[17] shl 3 + P[49] shl 2 + P[25] shl 1 + P[57]
end;  (* 1 DES *)
(***************************************************************************)
