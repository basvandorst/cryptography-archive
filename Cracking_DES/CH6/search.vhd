---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/07/97                                  
-- Description	   :  Search Unit
--------------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
--------------------------------------------------------------------------------
entity SEARCH_UNIT is

port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        WRB             : in     std_logic;
        RDB             : in     std_logic;
        SEARCH          : in     std_logic;
        EXTRA_XOR       : in     std_logic;
        USE_CBC       	: in     std_logic;
        ADDR_KEY        : in     std_logic_vector(6  downto 0);

        DATAI       	: in  	 std_logic_vector(7 downto 0);
        PT_BYTE_MASK   	: in  	 std_logic_vector(7 downto 0);
        PT_XOR_MASK   	: in  	 std_logic_vector(63 downto 0);
        PT_VECTOR       : in     std_logic_vector(255 downto 0);
        C0              : in     std_logic_vector(63 downto 0);
        C1              : in     std_logic_vector(63 downto 0);
        KEY_OUT         : out     std_logic_vector(55 downto 0);
        DES_OUTPUT      : out     std_logic_vector(63 downto 0);
--       MATCH_OUT 	: out     std_logic;
        SELECT_ONE 	: out     std_logic;
        SEARCH_OUT 	: out     std_logic;
        CLEAR_SEARCH 	: out     std_logic;
        DATAO        	: out    std_logic_vector(7 downto 0)
    );


end SEARCH_UNIT;

--------------------------------------------------------------------------------
architecture beh of SEARCH_UNIT is
--------------------------------------------------------------------------------
type DATA8_ARRAY is array(7 downto 0) of std_logic_vector(7 downto 0);

signal MESSAGE   	: std_logic_vector(63 downto 0);
signal IP_KEY   	: std_logic_vector(63 downto 0);
signal DES_OUT   	: std_logic_vector(63 downto 0);
signal EXTRA_XOR_OUT   	: std_logic_vector(63 downto 0);
signal SHIFT_REG   	: DATA8_ARRAY;
signal KEY 		: std_logic_vector(55 downto 0);
signal D_KEY 		: std_logic_vector(31 downto 0);
signal MESG_LEFT 	: std_logic_vector(31 downto 0);
signal CNT		: std_logic_vector(4 downto 0);
signal BIT_SHIFT_REG 	: std_logic_vector(7  downto 0);
signal TEMP_VECTOR 	: std_logic_vector(3  downto 0);
signal WR1B 		: std_logic;
signal WR_STROBEB	: std_logic;
signal DONE 		: std_logic;
signal STARTDES 	: std_logic;
signal MATCH 		: std_logic;
signal MATCH_DLY_CYCLE1	: std_logic;
signal MATCH_DLY_CYCLE2	: std_logic;
signal FALSE_MATCH	: std_logic;
signal SEARCH_DLY1 	: std_logic;
signal SEARCH_DLY2 	: std_logic;
signal SEARCH_DLY3 	: std_logic;
signal SEARCHING 	: std_logic;
signal SEARCHING_DLY 	: std_logic;
signal LOAD 		: std_logic;
signal FIRST_TIME1 	: std_logic;
signal FIRST_TIME2 	: std_logic;
signal FIRST_LOAD 	: std_logic;
signal SELECT1 		: std_logic;
signal SELECT1_DLY	: std_logic;
signal KEY_ODD_DLY1	: std_logic_vector(1  downto 0);
signal KEY_ODD_DLY2	: std_logic_vector(1  downto 0);
signal CHECK_SAME_KEY	: std_logic;
signal KEY_INCR		: std_logic;
signal KEY_DECR		: std_logic;
signal PRE_DONE		: std_logic;
signal CNT_EQ_1		: std_logic;
signal CNT_GT_10	: std_logic;
signal CNT_EQ_10	: std_logic;
signal CNT_LE_10	: std_logic;
signal FIRST_DES	: std_logic;
signal RESET_SEARCHING	: std_logic;
signal CLEAR_SEARCH_BAK     : std_logic;

signal EXTRA_SELECT	: std_logic_vector(2 downto 0);

signal BIT_MUX		: std_logic;

component DES
port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        START           : in     std_logic;
        MESSAGE         : in     std_logic_vector(63 downto 0);
        KEY             : in     std_logic_vector(55 downto 0);
        DONE            : out     std_logic;
        CNT         	: out     std_logic_vector(4 downto 0);
        DES_OUT         : out    std_logic_vector(63 downto 0)
    );
end component;


component MUX256
port(   

        SHIFT_OUT       : in     std_logic_vector(7 downto 0);
        PT_VECTOR       : in     std_logic_vector(255 downto 0);
        BIT_MUX         : out     std_logic
    );
end component;

begin
M256: MUX256
port map(   	SHIFT_OUT   	=> SHIFT_REG(7),
	        PT_VECTOR   	=> PT_VECTOR,
      	 	BIT_MUX  	=> BIT_MUX
    );

DES1: DES
port map(   	CLK             => CLK,
        	RST_N           => RST_N,
	        START           => STARTDES,
      	 	MESSAGE         => MESSAGE,
      		KEY             => KEY,
        	DONE            => DONE,
        	CNT         	=> CNT,
        	DES_OUT         => DES_OUT
    );
MESSAGE 	<= C0 when (SELECT1 = '0') else C1;
--------------------------------------------------------------------------------
PCSETSEARCH_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin   
if RST_N = '0' then
        FIRST_TIME1 <= '0';
        FIRST_TIME2 <= '0';
        SEARCH_DLY1 <= '0';
        SEARCH_DLY2 <= '0';
        SEARCH_DLY3 <= '0';
	for i in 0 to 7 loop
        SHIFT_REG(i) <= (others => '0');
	end loop;
elsif CLK'event and CLK = '1' then

	
        FIRST_TIME2 <= FIRST_TIME1;
	if (DONE = '1') then

		if (SEARCH = '1') then
       	 	FIRST_TIME1 <= '1';
		end if;

        SEARCH_DLY1 <= SEARCH;
        SEARCH_DLY2 <= SEARCH_DLY1 ;
        SEARCH_DLY3 <= SEARCH_DLY2 ;
	end if;
	if (CNT_EQ_1 = '1') then
		SHIFT_REG(7) <= EXTRA_XOR_OUT(63 downto 56);
		SHIFT_REG(6) <= EXTRA_XOR_OUT(55 downto 48);
		SHIFT_REG(5) <= EXTRA_XOR_OUT(47 downto 40);
		SHIFT_REG(4) <= EXTRA_XOR_OUT(39 downto 32);
		SHIFT_REG(3) <= EXTRA_XOR_OUT(31 downto 24);
		SHIFT_REG(2) <= EXTRA_XOR_OUT(23 downto 16);
		SHIFT_REG(1) <= EXTRA_XOR_OUT(15 downto  8);
		SHIFT_REG(0) <= EXTRA_XOR_OUT( 7 downto  0);
	else
		for i in 0 to 6 loop
		SHIFT_REG(i+1) <= SHIFT_REG(i);		
		end loop;
	end if;
end if;  
 
end process PCSETSEARCH_PR;


--------------------------------------------------------------------------------
---- Use to clear away invalid matches before PC loads   -----------------------
FIRST_LOAD <= FIRST_TIME1 and not(FIRST_TIME2);
--------------------------------------------------------------------------------


BIT_SHIFT_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
	BIT_SHIFT_REG <= (others => '1');
elsif CLK'event and CLK = '1' then

-------	 SHIFT --------------------------
	if (CNT_LE_10 = '1') then
		for i in 0 to 6 loop
		BIT_SHIFT_REG(i+1) <= BIT_SHIFT_REG(i);		
		end loop;
		BIT_SHIFT_REG(0) <= BIT_MUX;
	end if;
-------	
end if;
end process BIT_SHIFT_PR;

--------------------------------------------------------------------------------
MATCH_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
        MATCH <= '0';
 	MATCH_DLY_CYCLE1 <= '0';
 	MATCH_DLY_CYCLE2 <= '0';
 	KEY_ODD_DLY1     <= "00";
 	KEY_ODD_DLY2     <= "00";
elsif CLK'event and CLK = '1' then
	if (CNT = 10) then
		if ((BIT_SHIFT_REG(0) = '1' or (PT_BYTE_MASK(0) = '1')) and
		    (BIT_SHIFT_REG(1) = '1' or (PT_BYTE_MASK(1) = '1')) and
		    (BIT_SHIFT_REG(2) = '1' or (PT_BYTE_MASK(2) = '1')) and
		    (BIT_SHIFT_REG(3) = '1' or (PT_BYTE_MASK(3) = '1')) and
		    (BIT_SHIFT_REG(4) = '1' or (PT_BYTE_MASK(4) = '1')) and
		    (BIT_SHIFT_REG(5) = '1' or (PT_BYTE_MASK(5) = '1')) and
		    (BIT_SHIFT_REG(6) = '1' or (PT_BYTE_MASK(6) = '1')) and
		    (BIT_SHIFT_REG(7) = '1' or (PT_BYTE_MASK(7) = '1'))) then

		MATCH <= '1';
		else
		MATCH <= '0';
		end if;
	end if;
---------------------
	if (FIRST_LOAD = '1') then
		MATCH <= '0';
	end if;
---------------------
	if (CNT = 10) then
		MATCH_DLY_CYCLE2 <= MATCH_DLY_CYCLE1;
		MATCH_DLY_CYCLE1 <= MATCH ;
	end if;
---------------------
	if (PRE_DONE = '1') then
	 	KEY_ODD_DLY2     <= KEY_ODD_DLY1;
 		KEY_ODD_DLY1     <= KEY(1 downto 0);
	end if;
end if;
end process MATCH_PR;

--------------------------------------------------------------------------------
WRITE_STROBE_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
        WR1B <= '1';
        WR_STROBEB <= '1';
elsif CLK'event and CLK = '1' then
        WR_STROBEB <= WR1B;
        WR1B <= WRB;
end if;
end process WRITE_STROBE_PR;

--------------------------------------------------------------------------------
KEY_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if (RST_N = '0')then
        KEY <= (others => '0');
elsif CLK'event and CLK = '1' then
        if (WR1B = '0'and ADDR_KEY(0) = '1') then
                KEY(7  downto  0) <= DATAI;
        elsif (PRE_DONE = '1') then
                KEY(7  downto 0) <= D_KEY(7  downto 0);
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(1) = '1') then
                KEY(15 downto  8) <= DATAI;
        elsif (PRE_DONE = '1') then
                KEY(15 downto  8) <= D_KEY(15 downto  8);
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(2) = '1') then
                KEY(23 downto 16) <= DATAI;
        elsif (PRE_DONE = '1') then
                KEY(23 downto 16) <= D_KEY(23 downto 16);
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(3) = '1') then
                KEY(31 downto 24) <= DATAI;
        elsif (PRE_DONE = '1') then
                KEY(31 downto 24) <= D_KEY(31 downto 24);
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(4) = '1') then
                KEY(39 downto 32) <= DATAI;
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(5) = '1') then
                KEY(47 downto 40) <= DATAI;
        end if;
-----
        if (WR1B = '0'and ADDR_KEY(6) = '1') then
                KEY(55 downto 48) <= DATAI;
        end if;



end if;
end process KEY_PR;

--------------------------------------------------------------------------------
READ_KEY_PR: process(ADDR_KEY, RDB, KEY)
--------------------------------------------------------------------------------
begin
if (RDB = '0') then

	if (ADDR_KEY(0) = '1') then
	DATAO <= KEY(7  downto  0) ;
	elsif (ADDR_KEY(1) = '1') then
	DATAO <= KEY(15 downto  8) ;
	elsif (ADDR_KEY(2) = '1') then
	DATAO <= KEY(23 downto 16) ;
	elsif (ADDR_KEY(3) = '1') then
	DATAO <= KEY(31 downto 24) ;
	elsif (ADDR_KEY(4) = '1') then
	DATAO <= KEY(39 downto 32) ;
	elsif (ADDR_KEY(5) = '1') then
	DATAO <= KEY(47 downto 40) ;
	elsif (ADDR_KEY(6) = '1') then
	DATAO <= KEY(55 downto 48) ;
	else
	DATAO <= (others  => 'Z');
	end if;
else
	DATAO <= (others  => 'Z');
end if;
end process READ_KEY_PR;


--------------------------------------------------------------------------------
KEY_ALU_PR: process(KEY_DECR,KEY_INCR,KEY)
--------------------------------------------------------------------------------
begin


if (KEY_INCR = '1') and (KEY_DECR = '0')then
	D_KEY <= KEY(31 downto 0) + 1;
elsif (KEY_DECR = '1')  and (KEY_INCR = '0') then
	D_KEY <= KEY(31 downto 0) - 1;
else
	D_KEY <= KEY(31 downto 0);
end if;

end process KEY_ALU_PR;

--------------------------------------------------------------------------------
EXTRA_XOR_PR: process(PT_XOR_MASK,EXTRA_SELECT, DES_OUT, C0)
--------------------------------------------------------------------------------
begin
case EXTRA_SELECT is
when "000" => 
        EXTRA_XOR_OUT <= DES_OUT xor PT_XOR_MASK;
when "001" => 
        EXTRA_XOR_OUT <= DES_OUT xor PT_XOR_MASK;
when "010" => 
        EXTRA_XOR_OUT <= ((DES_OUT(63 downto 56)  xor DES_OUT(31 downto 24)) &
			  (DES_OUT(55 downto 48)  xor DES_OUT(23 downto 16)) &
			  (DES_OUT(47 downto 40)  xor DES_OUT(15 downto  8)) &
			  (DES_OUT(39 downto 32)  xor DES_OUT( 7 downto  0)) &
			   DES_OUT(31 downto  0)) xor PT_XOR_MASK; 
-----------------------------
when "101" => 
        EXTRA_XOR_OUT <= DES_OUT xor C0;
when "110" => 
        EXTRA_XOR_OUT <= (DES_OUT(63 downto 56) xor DES_OUT(31 downto 24)) &
			 (DES_OUT(55 downto 48) xor DES_OUT(23 downto 16)) &
			 (DES_OUT(47 downto 40) xor DES_OUT(15 downto  8)) &
			 (DES_OUT(39 downto 32) xor DES_OUT( 7 downto  0)) &
			 DES_OUT(31 downto  0) ; 


when others => 
	EXTRA_XOR_OUT <= DES_OUT;

end case;
end process EXTRA_XOR_PR;
EXTRA_SELECT <= SELECT1_DLY & EXTRA_XOR & USE_CBC;
--EXTRA_SELECT <= SELECT1 & EXTRA_XOR & USE_CBC;
--------------------------------------------------------------------------------
STARTDES_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
	STARTDES <= '0';
elsif CLK'event and CLK = '1' then
--	STARTDES <= DONE or LOAD;  --  17 clocks
	STARTDES <= PRE_DONE or LOAD;  --  16 clocks

end if;
end process STARTDES_PR;

--------------------------------------------------------------------------------
KEY_INCR_DECR_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
	KEY_INCR <= '0';
	KEY_DECR <= '0';
elsif CLK'event and CLK = '1' then
--
KEY_INCR  <=  (CNT_GT_10 and not(DONE) and SEARCHING_DLY) and (
	not(MATCH)  or  --- normal case
	SELECT1 or  	--- false match
	FIRST_DES);
KEY_DECR <= 	(CNT_GT_10 and not(DONE) and SEARCHING_DLY) and  --timing
		(MATCH and not(SELECT1))  -- only backup if match on C0
		and not(FIRST_DES);  

end if;
end process KEY_INCR_DECR_PR;

FALSE_MATCH  <= '1' when (MATCH_DLY_CYCLE2 = '1') and (MATCH = '0') and (SEARCHING_DLY = '1')
			else '0';
-------------------------------------------------
---- timing block, sensitive to START  ----------
-------------------------------------------------
PRE_DONE <= '1' when (CNT = "01111") else '0';
RESET_SEARCHING <= '1' when (CNT = "01100") else '0';

CNT_EQ_1  <= '1' when (CNT = 1 ) else '0';
CNT_LE_10 <= '1' when (CNT > 1 and CNT < 10) else '0';
CNT_EQ_10 <= '1' when (CNT = 10) else '0';
CNT_GT_10 <= '1' when (CNT > 10) else '0';
--------------------------------------------------------------------------------
SEARCHING_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
        SEARCHING <= '0';
        SEARCHING_DLY <= '0';
        CLEAR_SEARCH        <= '0';
elsif CLK'event and CLK = '1' then
        SEARCHING_DLY <= SEARCHING;

--------------- search active -------------------
	if ((LOAD = '1') or (SEARCHING = '1')) then
		SEARCHING  <= '1';
	end if;

--------------- found C1 ------------------------
	if (CLEAR_SEARCH_BAK = '1') then
		SEARCHING  <= '0';
	end if;

---------------
	CLEAR_SEARCH        <= CLEAR_SEARCH_BAK;
end if;
 
end process SEARCHING_PR;
	CHECK_SAME_KEY <= '1' when (KEY(1 downto 0) = KEY_ODD_DLY2) else '0';

--------------------------------------------------------------------------------
SELECT1_PR: process(RST_N,CLK)
--------------------------------------------------------------------------------
begin
if RST_N = '0' then
        SELECT1 <= '1';
elsif CLK'event and CLK = '1' then

--------------- found C0, look for C1  ----------
	if ((MATCH = '1') and (SELECT1 = '0') and (PRE_DONE = '1'))  then
		SELECT1  <= '1';
	end if;

--------------- Restart by PC or C1 is not a match ----------
	if  -- (LOAD = '1')  or  
	((SELECT1 = '1') and (PRE_DONE = '1') and (SEARCHING_DLY = '1')) then
		SELECT1  <= '0';
	end if;
---------------
	if (PRE_DONE = '1') then
		SELECT1_DLY <= SELECT1 ;
	end if;
end if;
 
end process SELECT1_PR;
--------------------------------------------------------------------------------
SEARCH_OUT 	<= SEARCHING;
LOAD 		<= SEARCH_DLY1 and PRE_DONE and not(SEARCH_DLY2); -- 17 clocks
FIRST_DES 	<= SEARCH_DLY2 and not(SEARCH_DLY3); 
CLEAR_SEARCH_BAK 	<= 
		'1' when ((MATCH = '1') and (SELECT1 = '0') 
                and (SELECT1_DLY = '1') and (RESET_SEARCHING = '1') 
		and (SEARCHING = '1')) else '0';

SELECT_ONE	<= SELECT1;
 KEY_OUT 	<= KEY;
 DES_OUTPUT	<= DES_OUT;
-- MATCH_OUT	<= MATCH;
--------------------------------------------------------------------------------
end beh;
--------------------------------------------------------------------------------

