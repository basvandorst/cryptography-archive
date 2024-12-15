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
entity UPI is

port(   RST_N           : in     std_logic;
        BOARD_EN       	: in     std_logic;
        ALE          	: in     std_logic;
        ADDSEL1        	: in     std_logic;
        WRB          	: in     std_logic;
        RDB          	: in     std_logic;
        ADDSEL2        	: in     std_logic;
        AA_IN         	: in     std_logic;
        ADDR       	: in     std_logic_vector(7 downto 0);
        CHIP_ID       	: in     std_logic_vector(7 downto 0);
        SEARCH_OUT     	: in     std_logic_vector(23 downto 0);
        CLEAR_SEARCH     	: in     std_logic_vector(23 downto 0);
        SELECT_ONE  	: in     std_logic_vector(23 downto 0);

        AA_OUT       	: out    std_logic;
        CHIP_AA_OUT   	: out    std_logic;
        EXTRA_XOR       : out    std_logic;
        USE_CBC         : out    std_logic;
        SEARCH_IN    	: OUT     std_logic_vector(23 downto 0);
        PT_XOR_MASK    	: out    std_logic_vector(63 downto 0);
        PT_BYTE_MASK  	: out    std_logic_vector(7 downto 0);
        PT_VECTOR      	: out    std_logic_vector(255 downto 0);
        C0      	: out    std_logic_vector(63 downto 0);
        C1      	: out    std_logic_vector(63 downto 0);
        ADDR_KEY0      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY1      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY2      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY3      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY4      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY5      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY6      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY7      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY8      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY9      	: out    std_logic_vector(6  downto 0);
        ADDR_KEY10     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY11     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY12     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY13     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY14     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY15     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY16     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY17     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY18     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY19     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY20     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY21     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY22     	: out    std_logic_vector(6  downto 0);
        ADDR_KEY23     	: out    std_logic_vector(6  downto 0);
        DATAI       	: in     std_logic_vector(7 downto 0);
        DATAO       	: out    std_logic_vector(7 downto 0)
    );


end UPI;

--------------------------------------------------------------------------------
architecture beh of UPI is
--------------------------------------------------------------------------------


signal SEARCH_IN_BAK 	: std_logic_vector(23  downto 0);
signal CHIP_EN 		: std_logic;

component ADDR_KEY
port(   
        ADDSEL2         : in     std_logic;
        CHIP_EN         : in     std_logic;
        ADDR            : in     std_logic_vector(7 downto 0);

        ADDR_KEY0       : out    std_logic_vector(6  downto 0);
        ADDR_KEY1       : out    std_logic_vector(6  downto 0);
        ADDR_KEY2       : out    std_logic_vector(6  downto 0);
        ADDR_KEY3       : out    std_logic_vector(6  downto 0);
        ADDR_KEY4       : out    std_logic_vector(6  downto 0);
        ADDR_KEY5       : out    std_logic_vector(6  downto 0);
        ADDR_KEY6       : out    std_logic_vector(6  downto 0);
        ADDR_KEY7       : out    std_logic_vector(6  downto 0);
        ADDR_KEY8       : out    std_logic_vector(6  downto 0);
        ADDR_KEY9       : out    std_logic_vector(6  downto 0);
        ADDR_KEY10      : out    std_logic_vector(6  downto 0);
        ADDR_KEY11      : out    std_logic_vector(6  downto 0);
        ADDR_KEY12      : out    std_logic_vector(6  downto 0);
        ADDR_KEY13      : out    std_logic_vector(6  downto 0);
        ADDR_KEY14      : out    std_logic_vector(6  downto 0);
        ADDR_KEY15      : out    std_logic_vector(6  downto 0);
        ADDR_KEY16      : out    std_logic_vector(6  downto 0);
        ADDR_KEY17      : out    std_logic_vector(6  downto 0);
        ADDR_KEY18      : out    std_logic_vector(6  downto 0);
        ADDR_KEY19      : out    std_logic_vector(6  downto 0);
        ADDR_KEY20      : out    std_logic_vector(6  downto 0);
        ADDR_KEY21      : out    std_logic_vector(6  downto 0);
        ADDR_KEY22      : out    std_logic_vector(6  downto 0);
        ADDR_KEY23      : out    std_logic_vector(6  downto 0)
    );
end component;
 

component REG_RDWR
port(   RST_N           : in     std_logic;
        BOARD_EN        : in     std_logic;
        ALE             : in     std_logic;
        ADDSEL1         : in     std_logic;
        WRB             : in     std_logic;
        RDB             : in     std_logic;
        ADDSEL2         : in     std_logic;
        AA_IN           : in     std_logic;
        ADDR            : in     std_logic_vector(7 downto 0);
        CHIP_ID         : in     std_logic_vector(7 downto 0);
        SEARCH_OUT      : in     std_logic_vector(23 downto 0);
        SELECT_ONE      : in     std_logic_vector(23 downto 0);
        SEARCH_IN       : in      std_logic_vector(23 downto 0);

        CHIP_EN         : out    std_logic;
        AA_OUT          : out    std_logic;
        CHIP_AA_OUT     : out    std_logic;
        EXTRA_XOR       : out    std_logic;
        USE_CBC         : out    std_logic;
        PT_XOR_MASK     : out    std_logic_vector(63 downto 0);
        PT_BYTE_MASK    : out    std_logic_vector(7 downto 0);
        PT_VECTOR       : out    std_logic_vector(255 downto 0);
        C0              : out    std_logic_vector(63 downto 0);
        C1              : out    std_logic_vector(63 downto 0);
        DATAI           : in     std_logic_vector(7 downto 0);
        DATAO           : out    std_logic_vector(7 downto 0)
    );
end component;

 

component  START_REG
port(   RST_N           : in     std_logic;
        CHIP_EN         : in     std_logic;
        WRB             : in     std_logic;
        ADDSEL2         : in     std_logic;
        ADDR            : in     std_logic_vector(7 downto 0);
        CLEAR_SEARCH    : in     std_logic_vector(23 downto 0);

        SEARCH_IN       : OUT     std_logic_vector(23 downto 0);
        DATAI           : in     std_logic_vector(7 downto 0)
    );
end component;

begin
--------------------------------------------------------------------------------
ADDR_KEYX : ADDR_KEY
port map(
        ADDSEL2         => ADDSEL2,
        CHIP_EN         => CHIP_EN,
        ADDR            => ADDR,

        ADDR_KEY0       => ADDR_KEY0,
        ADDR_KEY1       => ADDR_KEY1,
        ADDR_KEY2       => ADDR_KEY2,
        ADDR_KEY3       => ADDR_KEY3,
        ADDR_KEY4       => ADDR_KEY4,
        ADDR_KEY5       => ADDR_KEY5,
        ADDR_KEY6       => ADDR_KEY6,
        ADDR_KEY7       => ADDR_KEY7,
        ADDR_KEY8       => ADDR_KEY8,
        ADDR_KEY9       => ADDR_KEY9,
        ADDR_KEY10      => ADDR_KEY10,
        ADDR_KEY11      => ADDR_KEY11,
        ADDR_KEY12      => ADDR_KEY12,
        ADDR_KEY13      => ADDR_KEY13,
        ADDR_KEY14      => ADDR_KEY14,
        ADDR_KEY15      => ADDR_KEY15,
        ADDR_KEY16      => ADDR_KEY16,
        ADDR_KEY17      => ADDR_KEY17,
        ADDR_KEY18      => ADDR_KEY18,
        ADDR_KEY19      => ADDR_KEY19,
        ADDR_KEY20      => ADDR_KEY20,
        ADDR_KEY21      => ADDR_KEY21,
        ADDR_KEY22      => ADDR_KEY22,
        ADDR_KEY23      => ADDR_KEY23
	);

REG_RDWRX : REG_RDWR
port map(RST_N          => RST_N,
        BOARD_EN        => BOARD_EN,
        ALE             => ALE,
        ADDSEL1         => ADDSEL1,
        WRB             => WRB,
        RDB             => RDB,
        ADDSEL2         => ADDSEL2,
        AA_IN           => AA_IN,
        ADDR            => ADDR,
        CHIP_ID         => CHIP_ID,
        SEARCH_OUT      => SEARCH_OUT,
        SELECT_ONE      => SELECT_ONE,
        SEARCH_IN       => SEARCH_IN_BAK,

        CHIP_EN         => CHIP_EN,
        AA_OUT          => AA_OUT,
        CHIP_AA_OUT     => CHIP_AA_OUT,
        EXTRA_XOR       => EXTRA_XOR,
        USE_CBC         => USE_CBC,
        PT_XOR_MASK     => PT_XOR_MASK,
        PT_BYTE_MASK    => PT_BYTE_MASK,
        PT_VECTOR       => PT_VECTOR,
        C0              => C0,
        C1              => C1,
        DATAI           => DATAI,
        DATAO           => DATAO
    );
START_REGX :  START_REG
port map(RST_N          => RST_N,
        CHIP_EN         => CHIP_EN,
        WRB             => WRB,
        ADDSEL2         => ADDSEL2,
        ADDR            => ADDR,
        CLEAR_SEARCH    => CLEAR_SEARCH,

        SEARCH_IN       => SEARCH_IN_BAK,
        DATAI           => DATAI
    );
SEARCH_IN <= SEARCH_IN_BAK;
end beh;
--------------------------------------------------------------------------------

