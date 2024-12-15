---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/19/97                                  
-- Description	   :  UProcessor interface
--------------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
--------------------------------------------------------------------------------
entity START_REG is

port(   RST_N           : in     std_logic;
        CHIP_EN       	: in     std_logic;
        WRB          	: in     std_logic;
        ADDSEL2        	: in     std_logic;
        ADDR       	: in     std_logic_vector(7 downto 0);
        CLEAR_SEARCH    : in     std_logic_vector(23 downto 0);

        SEARCH_IN    	: OUT     std_logic_vector(23 downto 0);
        DATAI       	: in     std_logic_vector(7 downto 0)
    );


end START_REG;

--------------------------------------------------------------------------------
architecture beh of START_REG is
--------------------------------------------------------------------------------


signal SEARCH_IN_REG 	: std_logic_vector(23  downto 0);
signal SEARCH_RST_N_0 	: std_logic;
signal SEARCH_RST_N_1 	: std_logic;
signal SEARCH_RST_N_2 	: std_logic;
signal SEARCH_RST_N_3 	: std_logic;
signal SEARCH_RST_N_4 	: std_logic;
signal SEARCH_RST_N_5 	: std_logic;
signal SEARCH_RST_N_6 	: std_logic;
signal SEARCH_RST_N_7 	: std_logic;
signal SEARCH_RST_N_8 	: std_logic;
signal SEARCH_RST_N_9 	: std_logic;
signal SEARCH_RST_N_10 	: std_logic;
signal SEARCH_RST_N_11 	: std_logic;
signal SEARCH_RST_N_12 	: std_logic;
signal SEARCH_RST_N_13 	: std_logic;
signal SEARCH_RST_N_14 	: std_logic;
signal SEARCH_RST_N_15 	: std_logic;
signal SEARCH_RST_N_16 	: std_logic;
signal SEARCH_RST_N_17 	: std_logic;
signal SEARCH_RST_N_18 	: std_logic;
signal SEARCH_RST_N_19 	: std_logic;
signal SEARCH_RST_N_20 	: std_logic;
signal SEARCH_RST_N_21 	: std_logic;
signal SEARCH_RST_N_22 	: std_logic;
signal SEARCH_RST_N_23 	: std_logic;

begin

--------------------------------------------------------------------------------
SEARCH_IN0_PR: process(SEARCH_RST_N_0, WRB)
--------------------------------------------------------------------------------
begin   
----------------------------------------------------------------
if (SEARCH_RST_N_0 = '0') then
	SEARCH_IN_REG(0) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01000111")) then
	SEARCH_IN_REG(0) <= DATAI(0);
else
	SEARCH_IN_REG(0) <= SEARCH_IN_REG(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN0_PR;
--------------------------------------------------------------------------------
SEARCH_IN1_PR: process(SEARCH_RST_N_1, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_1 = '0') then
	SEARCH_IN_REG(1) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01001111")) then
	SEARCH_IN_REG(1) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN1_PR;
--------------------------------------------------------------------------------
SEARCH_IN2_PR: process(SEARCH_RST_N_2, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_2 = '0') then
	SEARCH_IN_REG(2) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01010111")) then
	SEARCH_IN_REG(2) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN2_PR;
--------------------------------------------------------------------------------
SEARCH_IN3_PR: process(SEARCH_RST_N_3, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_3 = '0') then
	SEARCH_IN_REG(3) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01011111")) then
	SEARCH_IN_REG(3) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN3_PR;
--------------------------------------------------------------------------------
SEARCH_IN4_PR: process(SEARCH_RST_N_4, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_4 = '0') then
	SEARCH_IN_REG(4) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01100111")) then
	SEARCH_IN_REG(4) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN4_PR;
--------------------------------------------------------------------------------
SEARCH_IN5_PR: process(SEARCH_RST_N_5, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_5 = '0') then
	SEARCH_IN_REG(5) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01101111")) then
	SEARCH_IN_REG(5) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN5_PR;
--------------------------------------------------------------------------------
SEARCH_IN6_PR: process(SEARCH_RST_N_6, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_6 = '0') then
	SEARCH_IN_REG(6) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01110111")) then
	SEARCH_IN_REG(6) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN6_PR;
--------------------------------------------------------------------------------
SEARCH_IN7_PR: process(SEARCH_RST_N_7, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_7 = '0') then
	SEARCH_IN_REG(7) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "01111111")) then
	SEARCH_IN_REG(7) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN7_PR;
--------------------------------------------------------------------------------
SEARCH_IN8_PR: process(SEARCH_RST_N_8, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_8 = '0') then
	SEARCH_IN_REG(8) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10000111")) then
	SEARCH_IN_REG(8) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN8_PR;
--------------------------------------------------------------------------------
SEARCH_IN9_PR: process(SEARCH_RST_N_9, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_9 = '0') then
	SEARCH_IN_REG(9) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10001111")) then
	SEARCH_IN_REG(9) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN9_PR;
--------------------------------------------------------------------------------
SEARCH_IN10_PR: process(SEARCH_RST_N_10, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_10 = '0') then
	SEARCH_IN_REG(10) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10010111")) then
	SEARCH_IN_REG(10) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN10_PR;
--------------------------------------------------------------------------------
SEARCH_IN11_PR: process(SEARCH_RST_N_11, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_11 = '0') then
	SEARCH_IN_REG(11) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10011111")) then
	SEARCH_IN_REG(11) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN11_PR;
--------------------------------------------------------------------------------
SEARCH_IN12_PR: process(SEARCH_RST_N_12, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_12 = '0') then
	SEARCH_IN_REG(12) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10100111")) then
	SEARCH_IN_REG(12) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN12_PR;
--------------------------------------------------------------------------------
SEARCH_IN13_PR: process(SEARCH_RST_N_13, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_13 = '0') then
	SEARCH_IN_REG(13) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10101111")) then
	SEARCH_IN_REG(13) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN13_PR;
--------------------------------------------------------------------------------
SEARCH_IN14_PR: process(SEARCH_RST_N_14, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_14 = '0') then
	SEARCH_IN_REG(14) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10110111")) then
	SEARCH_IN_REG(14) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN14_PR;
--------------------------------------------------------------------------------
SEARCH_IN15_PR: process(SEARCH_RST_N_15, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_15 = '0') then
	SEARCH_IN_REG(15) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "10111111")) then
	SEARCH_IN_REG(15) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN15_PR;
--------------------------------------------------------------------------------
SEARCH_IN16_PR: process(SEARCH_RST_N_16, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_16 = '0') then
	SEARCH_IN_REG(16) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11000111")) then
	SEARCH_IN_REG(16) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN16_PR;
--------------------------------------------------------------------------------
SEARCH_IN17_PR: process(SEARCH_RST_N_17, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_17 = '0') then
	SEARCH_IN_REG(17) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11001111")) then
	SEARCH_IN_REG(17) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN17_PR;
--------------------------------------------------------------------------------
SEARCH_IN18_PR: process(SEARCH_RST_N_18, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_18 = '0') then
	SEARCH_IN_REG(18) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11010111")) then
	SEARCH_IN_REG(18) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN18_PR;
--------------------------------------------------------------------------------
SEARCH_IN19_PR: process(SEARCH_RST_N_19, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_19 = '0') then
	SEARCH_IN_REG(19) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11011111")) then
	SEARCH_IN_REG(19) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN19_PR;
--------------------------------------------------------------------------------
SEARCH_IN20_PR: process(SEARCH_RST_N_20, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_20 = '0') then
	SEARCH_IN_REG(20) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11100111")) then
	SEARCH_IN_REG(20) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN20_PR;
--------------------------------------------------------------------------------
SEARCH_IN21_PR: process(SEARCH_RST_N_21, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_21 = '0') then
	SEARCH_IN_REG(21) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11101111")) then
	SEARCH_IN_REG(21) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN21_PR;
--------------------------------------------------------------------------------
SEARCH_IN22_PR: process(SEARCH_RST_N_22, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_22 = '0') then
	SEARCH_IN_REG(22) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11110111")) then
	SEARCH_IN_REG(22) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN22_PR;
--------------------------------------------------------------------------------
SEARCH_IN23_PR: process(SEARCH_RST_N_23, WRB)
--------------------------------------------------------------------------------
begin   
if (SEARCH_RST_N_23 = '0') then
	SEARCH_IN_REG(23) <= '0';
elsif (WRB'event and WRB= '1') then
if ((CHIP_EN = '1') and (ADDSEL2 = '0') and (ADDR = "11111111")) then
	SEARCH_IN_REG(23) <= DATAI(0);
end if;  
end if;  
----------------------------------------------------------------
end process SEARCH_IN23_PR;
----------------------------------------------------------------

SEARCH_RST_N_0  <= RST_N and not(CLEAR_SEARCH(0));
SEARCH_RST_N_1  <= RST_N and not(CLEAR_SEARCH(1));
SEARCH_RST_N_2  <= RST_N and not(CLEAR_SEARCH(2));
SEARCH_RST_N_3  <= RST_N and not(CLEAR_SEARCH(3));
SEARCH_RST_N_4  <= RST_N and not(CLEAR_SEARCH(4));
SEARCH_RST_N_5  <= RST_N and not(CLEAR_SEARCH(5));
SEARCH_RST_N_6  <= RST_N and not(CLEAR_SEARCH(6));
SEARCH_RST_N_7  <= RST_N and not(CLEAR_SEARCH(7));
SEARCH_RST_N_8  <= RST_N and not(CLEAR_SEARCH(8));
SEARCH_RST_N_9  <= RST_N and not(CLEAR_SEARCH(9));
SEARCH_RST_N_10 <= RST_N and not(CLEAR_SEARCH(10));
SEARCH_RST_N_11 <= RST_N and not(CLEAR_SEARCH(11));
SEARCH_RST_N_12 <= RST_N and not(CLEAR_SEARCH(12));
SEARCH_RST_N_13 <= RST_N and not(CLEAR_SEARCH(13));
SEARCH_RST_N_14 <= RST_N and not(CLEAR_SEARCH(14));
SEARCH_RST_N_15 <= RST_N and not(CLEAR_SEARCH(15));
SEARCH_RST_N_16 <= RST_N and not(CLEAR_SEARCH(16));
SEARCH_RST_N_17 <= RST_N and not(CLEAR_SEARCH(17));
SEARCH_RST_N_18 <= RST_N and not(CLEAR_SEARCH(18));
SEARCH_RST_N_19 <= RST_N and not(CLEAR_SEARCH(19));
SEARCH_RST_N_20 <= RST_N and not(CLEAR_SEARCH(20));
SEARCH_RST_N_21 <= RST_N and not(CLEAR_SEARCH(21));
SEARCH_RST_N_22 <= RST_N and not(CLEAR_SEARCH(22));
SEARCH_RST_N_23 <= RST_N and not(CLEAR_SEARCH(23));
 
SEARCH_IN 	<= SEARCH_IN_REG;

--------------------------------------------------------------------------------
end beh;
--------------------------------------------------------------------------------

