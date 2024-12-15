---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu
-- Date            :  09/27/97                                  
-- Description	   :  Generate Schedule Keys to be used by F funtion
-- Function	   :  2 rings of 28 bits each will shift left or right by 1 or
--		      2 positions depends on ENCR/DECR and counter16
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;


-- -----------------------------------------------------------------------------
entity KEY_GEN is

port(   CLK  	   : in     std_logic;
        RST_N  	   : in     std_logic;
        START	   : in     std_logic;
--	DECR	   : in     std_logic;
	KEY_IN     : in     std_logic_vector(55 downto 0);
	DONE	   : out    std_logic;
        CNT        : out    std_logic_vector(4  downto 0);
       	KEY_OUT    : out    std_logic_vector(47 downto 0)
    );

end KEY_GEN;

-- -----------------------------------------------------------------------------
architecture beh of KEY_GEN is
-- -----------------------------------------------------------------------------

component PC1 
port(   
	KEY_IN     : in     std_logic_vector(55 downto 0);
       	KEY_OUT    : out    std_logic_vector(55 downto 0)
    );
end component;

component PC2
port(   
	KEY_IN     : in     std_logic_vector(55 downto 0);
       	KEY_OUT    : out    std_logic_vector(47 downto 0)
    );
end component;

signal cnt16 : std_logic_vector(4 downto 0);
signal PC1_KEY   : std_logic_vector(55 downto 0);
signal PC1_KEY_C : std_logic_vector(27 downto 0);
signal PC1_KEY_D : std_logic_vector(27 downto 0);
signal KEY_REG_C : std_logic_vector(27 downto 0);
signal KEY_REG_D : std_logic_vector(27 downto 0);
signal KEY_REG   : std_logic_vector(55 downto 0);
signal SHIFT1   : std_logic;


begin

-- -----------------------------------------------------------------------------
-- Permutation Choice #1
-- -----------------------------------------------------------------------------

	PC_1: PC1 port map(KEY_IN => KEY_IN,KEY_OUT => PC1_KEY);

-- -----------------------------------------------------------------------------
Split_to_C_and_D: process(PC1_KEY)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 27 loop
	    PC1_KEY_D(i) <= PC1_KEY(i);
	    PC1_KEY_C(i) <= PC1_KEY(i+28);
	end loop;
end process;
-- -----------------------------------------------------------------------------
DONE_P: process(CLK,RST_N) 
-- -----------------------------------------------------------------------------
begin
	if RST_N = '0' then
	    DONE  <= '0';
	    SHIFT1  <= '0';
	elsif CLK'event and CLK = '1' then
	    if CNT16 = 15 and START = '0' then
		DONE  <= '1';
	    else
		DONE  <= '0';
	    end if;

	    if START = '1' or CNT16 = 7 or CNT16= 14 then
		SHIFT1 <= '1' ;
	    else 
		SHIFT1 <= '0' ;
	    end if;
	end if;
end process;
-- -----------------------------------------------------------------------------
COUNTER16_P: process(CLK,RST_N) 
-- -----------------------------------------------------------------------------
begin
	if RST_N = '0' then
		CNT16 <= (others => '0');
	elsif CLK'event and CLK = '1' then
	    if START = '1'  then
		CNT16 <= "00001";
	    else
		CNT16 <= CNT16 + 1;
	    end if;
	end if;
end process;
-- -----------------------------------------------------------------------------
KEY_GEN_REG_P: process(CLK,RST_N) 
-- -----------------------------------------------------------------------------
begin

    if RST_N = '0' then
	KEY_REG_C <= (others => '0');
	KEY_REG_D <= (others => '0');
    elsif CLK'event and CLK = '1' then
--	if DECR = '0' then                  
--	    if START = '1' then -- Load and Shift by 1 from external key
--	    	KEY_REG_C <= PC1_KEY_C(26 downto 0) & PC1_KEY_C(27);
--	    	KEY_REG_D <= PC1_KEY_D(26 downto 0) & PC1_KEY_D(27);
--	    elsif SHIFT1 = '1' then -- Shift Left by 1 for 1st key
--	    	KEY_REG_C <= KEY_REG_C(26 downto 0) & KEY_REG_C(27);
--	    	KEY_REG_D <= KEY_REG_D(26 downto 0) & KEY_REG_D(27);
--	    else
--	    	KEY_REG_C <= KEY_REG_C(25 downto 0) & KEY_REG_C(27 downto 26);
--	    	KEY_REG_D <= KEY_REG_D(25 downto 0) & KEY_REG_D(27 downto 26);
--	    end if;
--	else 
	    if START = '1' then
	    	KEY_REG_C <= PC1_KEY_C; -- Last key was used in Encr
	    	KEY_REG_D <= PC1_KEY_D;
	    elsif SHIFT1 = '1' then
		-- Shift Right by 1 when cnt16 =1,8,15
	    	KEY_REG_C <= KEY_REG_C(0) & KEY_REG_C(27 downto 1);
	    	KEY_REG_D <= KEY_REG_D(0) & KEY_REG_D(27 downto 1);
	    else
		-- Shift Right by 2 when cnt16=others
	    	KEY_REG_C <= KEY_REG_C(1 downto 0) & KEY_REG_C(27 downto 2);
	    	KEY_REG_D <= KEY_REG_D(1 downto 0) & KEY_REG_D(27 downto 2);
	    end if;
 -- 	end if;
    end if;

end process;
-- -----------------------------------------------------------------------------
-- Combine final C and D to KEY_REG
-- -----------------------------------------------------------------------------
	KEY_REG <= KEY_REG_C(27 downto 0) & KEY_REG_D(27 downto 0);
-- -----------------------------------------------------------------------------
-- Permutation Choice #2
-- -----------------------------------------------------------------------------

	PC_2: PC2 port map (KEY_IN => KEY_REG,KEY_OUT => KEY_OUT);

-- -----------------------------------------------------------------------------
CNT <= CNT16;
end beh;
-- -----------------------------------------------------------------------------
