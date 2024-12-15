---------|---------|---------|---------|---------|---------|---------|---------|
-- -----------------------------------------------------------------------------
-- AUTHOR          :  TOM VU                                                  --
-- DATE            :  10/15/97                                                --
-- TITLE           :  DES  TEST BENCH                                         --
-- FILE            :  des_ctl.vhd                                             --
---------|---------|---------|---------|---------|---------|---------|---------|
--------------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
use ieee.std_logic_textio.all;

entity CTL is

port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        START           : in     std_logic;
        DECR            : in     std_logic;
        DES_OUT         : in     std_logic_vector(63 downto 0);
        DECR_INT        : out    std_logic;
        READ_EN        : out    std_logic;
        START_INT       : out    std_logic;
        MESSAGE         : out    std_logic_vector(63 downto 0);
        KEY          	: out    std_logic_vector(55 downto 0);
        DATA         	: inout  std_logic_vector(31 downto 0)
    );

end CTL;

-- -----------------------------------------------------------------------------
architecture BEH of CTL is
-- -----------------------------------------------------------------------------

signal CNT16           	: std_logic_vector(3 downto 0);
signal MSG0           	: std_logic_vector(31 downto 0);
signal MSG1           	: std_logic_vector(31 downto 0);
signal KEY0           	: std_logic_vector(31 downto 0);
signal KEY1           	: std_logic_vector(23 downto 0);
signal OUT0           	: std_logic_vector(31 downto 0);
signal OUT1           	: std_logic_vector(31 downto 0);
signal DATA_BAK         : std_logic_vector(31 downto 0);
signal START_INT_D	: std_logic;

begin
-- -----------------------------------------------------------------------------
CNT_PR: process(CLK,RST_N) 
-- -----------------------------------------------------------------------------
begin
        if RST_N = '0' then
                CNT16 <= "0000";
     		START_INT <= '0';
     		DECR_INT <= '0';
        elsif CLK'event and CLK = '1' then
     		START_INT <= START_INT_D;
		if CNT16 = 4 then
     			DECR_INT <= DECR;
		end if;
            if START = '1'  then
                CNT16 <= "0001";
            else
                CNT16 <= CNT16 + 1;
            end if;
        end if;
end process;

-- -----------------------------------------------------------------------------
KEY     <= KEY1 & KEY0;
MESSAGE <= MSG1 & MSG0;

START_INT_D <= '1' when CNT16 = 4 else '0';
-- -----------------------------------------------------------------------------
REG_IN_PR: process(RST_N,CLK)
-- -----------------------------------------------------------------------------
begin
if RST_N = '0' then
        MSG0 <= (others => '0');
        MSG1 <= (others => '0');
        KEY0 <= (others => '0');
        KEY1 <= (others => '0');
elsif CLK'event and CLK = '0' then
    case CNT16 is
	when  "0001" =>
        	MSG0 <= DATA;
	when  "0010" =>
        	MSG1 <= DATA;
	when  "0011" =>
        	KEY0 <= DATA;
	when  "0100" =>
        	KEY1 <= DATA(23 downto 0);
	when others =>
		null;
    end case;
end if;

end process REG_IN_PR;
-----------------------------------------------------------------
MESSAGE_OUT_P: process (CNT16,DES_OUT)
begin
    case CNT16 is
	when  "0110" | "0111" =>
        	DATA <= DES_OUT(63 downto 32);
		READ_EN <= '1';
	when  "1000" | "1001" =>
        	DATA <= DES_OUT(31 downto 0);
		READ_EN <= '1';
	when others =>
		DATA <= (others => 'Z');
		READ_EN <= '0';
    end case;
end process;
 
--DATA <= DATA_BAK;
end BEH;
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
use ieee.std_logic_textio.all;
use std.textio.all;


entity DES_CTL is 
port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        START           : in     std_logic;
        DECR            : in     std_logic;
        DONE            : out     std_logic;
        READ_EN         : out     std_logic;
        DATA         	: inout  std_logic_vector(31 downto 0)
    );

end DES_CTL;

--------------------------------------------------------------------------------
architecture beh of DES_CTL is

component DES 
port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        START           : in     std_logic;
        DECR            : in     std_logic;
        MESSAGE         : in     std_logic_vector(63 downto 0);
        KEY             : in     std_logic_vector(55 downto 0);
        DONE            : out     std_logic;
        DES_OUT          : out    std_logic_vector(63 downto 0)
    );

end component;

component CTL 
port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        START           : in     std_logic;
        DECR            : in     std_logic;
        DES_OUT         : in     std_logic_vector(63 downto 0);
        DECR_INT        : out    std_logic;
        READ_EN        	: out    std_logic;
        START_INT       : out    std_logic;
        MESSAGE         : out    std_logic_vector(63 downto 0);
        KEY          	: out    std_logic_vector(55 downto 0);
        DATA         	: inout  std_logic_vector(31 downto 0)
    );
end component;

signal START_INT     	: std_logic;
signal DECR_INT        	: std_logic;
signal MESSAGE         	: std_logic_vector(63 downto 0);
signal KEY             	: std_logic_vector(55 downto 0);
signal DES_OUT         	: std_logic_vector(63 downto 0);

--------------------------------------------
begin

DES_CTL1 : DES 
port map( 
        CLK             => CLK,
        RST_N           => RST_N,
        START           => START_INT,
        DECR            => DECR_INT,
        MESSAGE         => MESSAGE,
        KEY             => KEY,
        DONE            => DONE,
        DES_OUT         => DES_OUT
);

DES_CTL2 : CTL
port map(   CLK 	=> CLK,
        RST_N  		=> RST_N,
        START 		=> START,
        DECR 		=> DECR,
        DES_OUT		=> DES_OUT,
        DECR_INT	=> DECR_INT,
        READ_EN		=> READ_EN,
        START_INT	=> START_INT,
        MESSAGE 	=> MESSAGE,
        KEY		=> KEY,
        DATA		=> DATA
    );

end beh;
