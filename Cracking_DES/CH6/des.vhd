---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/27/97                                  
-- Description	   :  Left and Right 32-bit registers               
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
-- -----------------------------------------------------------------------------
entity MESG is

port(   CLK  	   	: in     std_logic;
        RST_N  	   	: in     std_logic;
        START	   	: in     std_logic;
	DONE     	: in     std_logic;
	MESSAGE     	: in     std_logic_vector(63 downto 0);
	SUBKEY     	: in     std_logic_vector(47 downto 0);
       	RESULT   	: out    std_logic_vector(63 downto 0)
    );

end MESG;

-- -----------------------------------------------------------------------------
architecture beh of MESG is
-- -----------------------------------------------------------------------------

signal IP_KEY   	: std_logic_vector(63 downto 0);
signal MESG_LEFT 	: std_logic_vector(31 downto 0);
signal MESG_RIGHT 	: std_logic_vector(31 downto 0);
signal NEW_L  		: std_logic_vector(31 downto 0);
signal L      		: std_logic_vector(31 downto 0);
signal R      		: std_logic_vector(31 downto 0);
signal EXPANDED_R	: std_logic_vector(47 downto 0);
signal X_KEY		: std_logic_vector(47 downto 0);
signal S_OUT   		: std_logic_vector(31 downto 0);
signal FP_IN 		: std_logic_vector(63 downto 0);
signal FP_OUT 		: std_logic_vector(63 downto 0);
signal P_IN 		: std_logic_vector(31 downto 0);
signal P_OUT 		: std_logic_vector(31 downto 0);

component EX
port(   
        EX_IN           : in    std_logic_vector(31 downto 0);
        EX_OUT          : out    std_logic_vector(47 downto 0));
end component;

component IP
port(
        IP_IN           : in     std_logic_vector(63 downto 0);
        IP_OUT          : out    std_logic_vector(63 downto 0));
end component;

component FP
port(
        FP_IN           : in     std_logic_vector(63 downto 0);
        FP_OUT          : out    std_logic_vector(63 downto 0));
end component;

component P
port(
        P_IN           : in     std_logic_vector(31 downto 0);
        P_OUT          : out    std_logic_vector(31 downto 0));
end component;

component S_TABLE
port(  KEY        : in     std_logic_vector(47 downto 0);
       S_OUT      : out    std_logic_vector(31 downto 0));
end component;

begin

MESSAGE1: EX
port map(      
	EX_IN           => R,
      	EX_OUT         	=> EXPANDED_R); 

MESSAGE2: IP
port map(      
	IP_IN           => MESSAGE,
      	IP_OUT         	=> IP_KEY); 

MESSAGE3: S_TABLE
port map(      
	KEY           	=> X_KEY,
      	S_OUT         	=> S_OUT); 

MESSAGE4: P
port map(      
	P_IN           	=> S_OUT,
      	P_OUT         	=> P_OUT); 


MESSAGE5: FP
port map(      
	FP_IN           => FP_IN,
      	FP_OUT         	=> FP_OUT); 


-- -----------------------------------------------------------------------------
-- Split_to_LEFT_and_RIGHT: process(IP_KEY)
-- -----------------------------------------------------------------------------
-- begin
-- 	for i in 0 to 31 loop
-- 	    MESG_RIGHT(i) 	<= IP_KEY(i);
-- 	    MESG_LEFT(i) 	<= IP_KEY(i+32);
-- 	end loop;
-- end process Split_to_LEFT_and_RIGHT;

MESG_RIGHT <= IP_KEY(31 downto 0);
MESG_LEFT  <= IP_KEY(63 downto 32);
-- -----------------------------------------------------------------------------
L_AND_R_REG_PR: process(RST_N,CLK)
-- -----------------------------------------------------------------------------
begin
if RST_N = '0' then
	L <= (others => '0');
	R <= (others => '0');
elsif CLK'event and CLK = '1' then
if (START = '1') then
	L <= MESG_LEFT;
	R <= MESG_RIGHT;
else 
	L <= R;
	R <= NEW_L;
end if;
end if;

end process L_AND_R_REG_PR;

-- -----------------------------------------------------------------------------
KEY_XOR_PR: process(SUBKEY,EXPANDED_R)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 47 loop
	    X_KEY(i) 	<= SUBKEY(i) xor EXPANDED_R(i);
	end loop;
end process KEY_XOR_PR;

-- -----------------------------------------------------------------------------
L_XOR_PR: process(L,P_OUT)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 31 loop
	    NEW_L(i) 	<= L(i) xor P_OUT(i);
	end loop;
end process L_XOR_PR;

-- -----------------------------------------------------------------------------
-- Combine final L and R to FP
-- -----------------------------------------------------------------------------
	FP_IN <= NEW_L(31 downto 0) & R(31 downto 0);

-- -----------------------------------------------------------------------------
RESULT_PR: process(RST_N,CLK)
-- -----------------------------------------------------------------------------
begin
if RST_N = '0' then
	RESULT <= (others => '0');
elsif CLK'event and CLK = '1' then
if (DONE = '1') then
	RESULT <= FP_OUT;
end if;
end if;

end process RESULT_PR;

-- -----------------------------------------------------------------------------
end beh;
-- -----------------------------------------------------------------------------

library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
-- -----------------------------------------------------------------------------
entity DES is

port(   CLK  	   	: in     std_logic;
        RST_N  	   	: in     std_logic;
        START	   	: in     std_logic;
	MESSAGE     	: in     std_logic_vector(63 downto 0);
	KEY     	: in     std_logic_vector(55 downto 0);
	DONE     	: out     std_logic;
	CNT     	: out     std_logic_vector(4 downto 0);
       	DES_OUT   	: out    std_logic_vector(63 downto 0)
    );

end DES;

-- -----------------------------------------------------------------------------
architecture beh of DES is
-- -----------------------------------------------------------------------------
signal SUBKEY		: std_logic_vector(47 downto 0);
signal DONE_BAK		: std_logic;

component MESG
port(   CLK  	   	: in     std_logic;
        RST_N  	   	: in     std_logic;
        START	   	: in     std_logic;
	DONE     	: in     std_logic;
	MESSAGE     	: in     std_logic_vector(63 downto 0);
	SUBKEY     	: in     std_logic_vector(47 downto 0);
       	RESULT   	: out    std_logic_vector(63 downto 0)
    );
end component;

component KEY_GEN 
port(   CLK        : in     std_logic;
        RST_N      : in     std_logic;
        START      : in     std_logic;
        KEY_IN     : in     std_logic_vector(55 downto 0);
        DONE       : out    std_logic;
        CNT        : out    std_logic_vector(4 downto 0);
        KEY_OUT    : out    std_logic_vector(47 downto 0));
end component;

begin

DES1: MESG
port map(   
	CLK  	   	=> CLK,
        RST_N  	   	=> RST_N,
        START	   	=> START,
	DONE     	=> DONE_BAK,
	MESSAGE     	=> MESSAGE,
	SUBKEY     	=> SUBKEY,
       	RESULT   	=> DES_OUT
    	);
DES2: KEY_GEN
port map(      
        CLK        	=> CLK,
        RST_N      	=> RST_N,
        START      	=> START,
        KEY_IN     	=> KEY,
        DONE       	=> DONE_BAK,
        CNT       	=> CNT,
        KEY_OUT    	=> SUBKEY
	); 

DONE <= DONE_BAK;

end beh;

