(*      Any program using these procedures must declare "uses DOS"     *)

const
TimerSet                          : Boolean = False;

var
ticks                             : longint;
TimerString                       : string[35];

procedure SetTimer;
{set ticks equal to the number of 100ths of a second since midnight}

var
h, m, s, cs                       : word;
Hour, Minute, Second, Sec100      : longint;

begin
TimerSet := True;
GetTime(h, m, s, cs);
Hour := h; Minute := m; Second := s; Sec100 := cs;
ticks := ((Hour*60 + Minute)*60 + Second)*100 + Sec100
end;        {of procedure SetTimer}

procedure ReadTimer;       {set TimerString = elapsed time}

var
h, m, s, cs                       : word;
Hour, Minute, Second, Sec100, t   : longint;
St1                               : string[2];
St                                : string[35];

begin
if TimerSet
then
  begin
    GetTime(h, m, s, cs);
    Hour := h; Minute := m; Second := s; Sec100 := cs;
    t := ((Hour*60 + Minute)*60 + Second)*100 + Sec100 - ticks;
    if t < 0 then t := t + 8640000;
    Sec100 := t MOD 100;
    t := t DIV 100;
    Second := t MOD 60;
    t := t DIV 60;
    Minute := t MOD 60;
    Hour := t DIV 60;
    Str(hour, st1);
    St := st1 + ' Hour';
    if Hour <> 1 then St := St + 's';
    Str(Minute, st1);
    St := St + ', ' + st1 + ' Minute';
    if Minute <> 1 then St := St + 's';
    Str(Second, st1);
    St := St + ', ' + st1 + '.';
    if (Sec100 < 10) then St := St + '0';
    Str(sec100, st1);
    St := St + st1 +  ' Seconds';
    TimerString := St
  end
else
  WriteLn('Timer not set -- use "SetTimer".')
end;


