---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/07/97                                  
-- Description	   :  TOP level for DES KEY Search array
--------------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
--------------------------------------------------------------------------------
entity TOP is

port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        BOARD_EN        : in     std_logic;
        ALE             : in     std_logic;
        ADDSEL1         : in     std_logic;
        WRB             : in     std_logic;
        RDB             : in     std_logic;
        ADDSEL2         : in     std_logic;
        AA_IN           : in     std_logic;
        ADDR            : in     std_logic_vector(7 downto 0);
        CHIP_ID         : in     std_logic_vector(7 downto 0);
        AA_OUT          : out    std_logic;
        CHIP_AA_OUT    	: out    std_logic;
        DATA       	: inout	 std_logic_vector(7 downto 0)
    );


end TOP;

--------------------------------------------------------------------------------
architecture beh of TOP is
--------------------------------------------------------------------------------
type DATA8_ARRAY is array(7 downto 0) of std_logic_vector(7 downto 0);
type DATA7_ARRAY is array(23 downto 0) of std_logic_vector(6 downto 0);

signal SHIFT_REG   	: DATA8_ARRAY;
signal SELECT_ONE 		: std_logic_vector(23 downto 0);
signal SEARCH_IN 	: std_logic_vector(23 downto 0);
signal SEARCH_OUT 	: std_logic_vector(23 downto 0);
signal CLEAR_SEARCH 	: std_logic_vector(23 downto 0);
signal PT_XOR_MASK       : std_logic_vector(63 downto 0);
signal PT_BYTE_MASK     : std_logic_vector(7 downto 0);
signal PT_VECTOR        : std_logic_vector(255 downto 0);
signal C0               : std_logic_vector(63 downto 0);
signal C1               : std_logic_vector(63 downto 0);

signal USE_CBC 		: std_logic;
signal EXTRA_XOR	: std_logic;
signal TEMPS 		: std_logic;
signal KEY 		: std_logic_vector(55 downto 0);
 signal DATAO		: std_logic_vector(7 downto 0);
 signal DATAI		: std_logic_vector(7 downto 0);
signal ADDR_KEY		: DATA7_ARRAY;

component SEARCH_UNIT
port(   CLK             : in     std_logic;
        RST_N           : in     std_logic;
        WRB             : in     std_logic;
        RDB             : in     std_logic;
        SEARCH          : in     std_logic;
        PT_BYTE_MASK    : in     std_logic_vector(7 downto 0);
        PT_XOR_MASK    	: in     std_logic_vector(63 downto 0);
        ADDR_KEY        : in     std_logic_vector(6  downto 0);
        EXTRA_XOR        : in     std_logic;
        USE_CBC          : in     std_logic;


        DATAI           : in     std_logic_vector(7 downto 0);
        PT_VECTOR       : in     std_logic_vector(255 downto 0);
        C0              : in     std_logic_vector(63 downto 0);
        C1              : in     std_logic_vector(63 downto 0);
        SEARCH_OUT      : out     std_logic;
        CLEAR_SEARCH      	: out     std_logic;
        SELECT_ONE   	: out     std_logic;
        DATAO           : out    std_logic_vector(7 downto 0)
    );

end component;


component UPI
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
        SELECT_ONE      : in     std_logic_vector(23 downto 0);
        SEARCH_IN       : OUT    std_logic_vector(23 downto 0);
        SEARCH_OUT      : in     std_logic_vector(23 downto 0);
        CLEAR_SEARCH   	: in     std_logic_vector(23 downto 0);
        EXTRA_XOR       : out    std_logic;
        USE_CBC         : out    std_logic;
        CHIP_AA_OUT    	: out    std_logic;
        AA_OUT          : out    std_logic;


        PT_XOR_MASK     : out    std_logic_vector(63 downto 0);
        PT_BYTE_MASK    : out    std_logic_vector(7 downto 0);
        PT_VECTOR       : out    std_logic_vector(255 downto 0);
        C0              : out    std_logic_vector(63 downto 0);
        C1              : out    std_logic_vector(63 downto 0);
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
        ADDR_KEY23      : out    std_logic_vector(6  downto 0);
        DATAI           : in  std_logic_vector(7 downto 0);
        DATAO           : out  std_logic_vector(7 downto 0)
    );

end component;

begin
UPI0: UPI
port map(
        RST_N           => RST_N,
        BOARD_EN        => BOARD_EN,
        ALE             => ALE,
        ADDSEL1         => ADDSEL1,
        WRB             => WRB,
        RDB             => RDB,
        ADDSEL2             => ADDSEL2,
        ADDR            => ADDR,
        CHIP_ID         => CHIP_ID,
        SEARCH_IN     	=> SEARCH_IN,
        SELECT_ONE  	=> SELECT_ONE,
        SEARCH_OUT     	=> SEARCH_OUT,
        EXTRA_XOR     	=> EXTRA_XOR,
        USE_CBC     	=> USE_CBC,
        CLEAR_SEARCH    => CLEAR_SEARCH,
        AA_IN     	=> AA_IN,
        AA_OUT     	=> AA_OUT,
        CHIP_AA_OUT    	=> CHIP_AA_OUT,

        PT_XOR_MASK     => PT_XOR_MASK,
        PT_BYTE_MASK    => PT_BYTE_MASK,
        PT_VECTOR       => PT_VECTOR,
        C0              => C0,
        C1              => C1,
        ADDR_KEY0       => ADDR_KEY(0) ,
        ADDR_KEY1       => ADDR_KEY(1) ,
        ADDR_KEY2       => ADDR_KEY(2) ,
        ADDR_KEY3       => ADDR_KEY(3) ,
        ADDR_KEY4       => ADDR_KEY(4) ,
        ADDR_KEY5       => ADDR_KEY(5) ,
        ADDR_KEY6       => ADDR_KEY(6) ,
        ADDR_KEY7       => ADDR_KEY(7) ,
        ADDR_KEY8       => ADDR_KEY(8) ,
        ADDR_KEY9       => ADDR_KEY(9) ,
        ADDR_KEY10      => ADDR_KEY(10),
        ADDR_KEY11      => ADDR_KEY(11),
        ADDR_KEY12      => ADDR_KEY(12),
        ADDR_KEY13      => ADDR_KEY(13),
        ADDR_KEY14      => ADDR_KEY(14),
        ADDR_KEY15      => ADDR_KEY(15),
        ADDR_KEY16      => ADDR_KEY(16),
        ADDR_KEY17      => ADDR_KEY(17),
        ADDR_KEY18      => ADDR_KEY(18),
        ADDR_KEY19      => ADDR_KEY(19),
        ADDR_KEY20      => ADDR_KEY(20),
        ADDR_KEY21      => ADDR_KEY(21),
        ADDR_KEY22      => ADDR_KEY(22),
        ADDR_KEY23      => ADDR_KEY(23),
        DATAI            => DATAI,
        DATAO            => DATAO
    );

--------------------------------------------------------------------------------
gen0: for i in 0 to 23 generate
SEARCH_UNITX: SEARCH_UNIT
port map(CLK             => CLK,
        RST_N            => RST_N,
        WRB              => WRB,
        RDB              => RDB,
        PT_BYTE_MASK     => PT_BYTE_MASK,
        PT_XOR_MASK      => PT_XOR_MASK,
        SEARCH           => SEARCH_IN(i),
        SELECT_ONE       => SELECT_ONE(i),
        ADDR_KEY         => ADDR_KEY(i),
        EXTRA_XOR      	 => EXTRA_XOR,
        USE_CBC     	 => USE_CBC,
 
        DATAI            => DATAI,
        PT_VECTOR        => PT_VECTOR,
        C0               => C0,
        C1               => C1,
        SEARCH_OUT       => SEARCH_OUT(i),
        CLEAR_SEARCH     => CLEAR_SEARCH(i),
        DATAO            => DATAO
    );
end generate;
--------------------------------------------------------------------------------
 DATAI <= DATA;
 DATA <= DATAO when (RDB = '0' and ADDSEL2 = '0') else (others => 'Z');
end beh;
--------------------------------------------------------------------------------



