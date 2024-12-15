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
entity REG_RDWR is

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
        SELECT_ONE  	: in     std_logic_vector(23 downto 0);
        SEARCH_IN    	: in      std_logic_vector(23 downto 0);

        CHIP_EN       	: out    std_logic;
        AA_OUT       	: out    std_logic;
        CHIP_AA_OUT   	: out    std_logic;
        EXTRA_XOR       : out    std_logic;
        USE_CBC         : out    std_logic;
        PT_XOR_MASK    	: out    std_logic_vector(63 downto 0);
        PT_BYTE_MASK  	: out    std_logic_vector(7 downto 0);
        PT_VECTOR      	: out    std_logic_vector(255 downto 0);
        C0      	: out    std_logic_vector(63 downto 0);
        C1      	: out    std_logic_vector(63 downto 0);
        DATAI       	: in     std_logic_vector(7 downto 0);
        DATAO       	: out    std_logic_vector(7 downto 0)
    );


end REG_RDWR;

--------------------------------------------------------------------------------
architecture beh of REG_RDWR is
--------------------------------------------------------------------------------
type DATA32_ARRAY is array(31 downto 0) of std_logic_vector(7 downto 0);
type DATA8_ARRAY  is array(7  downto 0) of std_logic_vector(7 downto 0);

signal PT_VECTOR_REG        	: DATA32_ARRAY;

signal PT_XOR_MASK_REG       	: DATA8_ARRAY;
signal CIPHER0        		: DATA8_ARRAY;
signal CIPHER1        		: DATA8_ARRAY;


signal SEARCH_INFO_REG 	: std_logic_vector(7  downto 0);
signal PT_BYTE_MASK_REG	: std_logic_vector(7  downto 0);
signal CHIP_REG 	: std_logic_vector(7  downto 0);
signal CHIP_EN_BAK	: std_logic;
signal ALL_ACTIVE 	: std_logic;
signal BAA_EN 	: std_logic;
signal AA_OUT_BAK 	: std_logic;

begin
CHIP_EN_BAK <= '1' when ((CHIP_ID = CHIP_REG) and BOARD_EN = '1') else '0';

--------------------------------------------------------------------------------
CHIP_ID_REG_PR: process(RST_N, ALE)
--------------------------------------------------------------------------------
begin   
if (RST_N = '0') then
	CHIP_REG <= (others => '0');
elsif (ALE'event and ALE= '1') then

if ((BOARD_EN = '1') and (ADDSEL1 = '1')) then
        CHIP_REG <= ADDR;
end if;  
end if;  
 
end process CHIP_ID_REG_PR;

--------------------------------------------------------------------------------
READ_PR: process(PT_VECTOR_REG, PT_XOR_MASK_REG,
		PT_BYTE_MASK_REG, SEARCH_INFO_REG, CIPHER0, CIPHER1,
		SEARCH_IN, SELECT_ONE, ALL_ACTIVE, AA_OUT_BAK,
		CHIP_EN_BAK, ADDSEL2, RDB, ADDR,BAA_EN)
--------------------------------------------------------------------------------
begin   
if ((CHIP_EN_BAK = '1') and (ADDSEL2 = '0') and (RDB = '0')) then
	case ADDR is

	when "00000000" => DATAO <= PT_VECTOR_REG(0);
	when "00000001" => DATAO <= PT_VECTOR_REG(1);
	when "00000010" => DATAO <= PT_VECTOR_REG(2);
	when "00000011" => DATAO <= PT_VECTOR_REG(3);
	when "00000100" => DATAO <= PT_VECTOR_REG(4);
	when "00000101" => DATAO <= PT_VECTOR_REG(5);
	when "00000110" => DATAO <= PT_VECTOR_REG(6);
	when "00000111" => DATAO <= PT_VECTOR_REG(7);
	when "00001000" => DATAO <= PT_VECTOR_REG(8);
	when "00001001" => DATAO <= PT_VECTOR_REG(9);
	when "00001010" => DATAO <= PT_VECTOR_REG(10);
	when "00001011" => DATAO <= PT_VECTOR_REG(11);
	when "00001100" => DATAO <= PT_VECTOR_REG(12);
	when "00001101" => DATAO <= PT_VECTOR_REG(13);
	when "00001110" => DATAO <= PT_VECTOR_REG(14);
	when "00001111" => DATAO <= PT_VECTOR_REG(15);
	when "00010000" => DATAO <= PT_VECTOR_REG(16);
	when "00010001" => DATAO <= PT_VECTOR_REG(17);
	when "00010010" => DATAO <= PT_VECTOR_REG(18);
	when "00010011" => DATAO <= PT_VECTOR_REG(19);
	when "00010100" => DATAO <= PT_VECTOR_REG(20);
	when "00010101" => DATAO <= PT_VECTOR_REG(21);
	when "00010110" => DATAO <= PT_VECTOR_REG(22);
	when "00010111" => DATAO <= PT_VECTOR_REG(23);
	when "00011000" => DATAO <= PT_VECTOR_REG(24);
	when "00011001" => DATAO <= PT_VECTOR_REG(25);
	when "00011010" => DATAO <= PT_VECTOR_REG(26);
	when "00011011" => DATAO <= PT_VECTOR_REG(27);
	when "00011100" => DATAO <= PT_VECTOR_REG(28);
	when "00011101" => DATAO <= PT_VECTOR_REG(29);
	when "00011110" => DATAO <= PT_VECTOR_REG(30);
	when "00011111" => DATAO <= PT_VECTOR_REG(31);

	when "00100000" => DATAO <= PT_XOR_MASK_REG(0);
	when "00100001" => DATAO <= PT_XOR_MASK_REG(1);
	when "00100010" => DATAO <= PT_XOR_MASK_REG(2);
	when "00100011" => DATAO <= PT_XOR_MASK_REG(3);
	when "00100100" => DATAO <= PT_XOR_MASK_REG(4);
	when "00100101" => DATAO <= PT_XOR_MASK_REG(5);
	when "00100110" => DATAO <= PT_XOR_MASK_REG(6);
	when "00100111" => DATAO <= PT_XOR_MASK_REG(7);

	when "00101000" => DATAO <= CIPHER0(0);
	when "00101001" => DATAO <= CIPHER0(1);
	when "00101010" => DATAO <= CIPHER0(2);
	when "00101011" => DATAO <= CIPHER0(3);
	when "00101100" => DATAO <= CIPHER0(4);
	when "00101101" => DATAO <= CIPHER0(5);
	when "00101110" => DATAO <= CIPHER0(6);
	when "00101111" => DATAO <= CIPHER0(7);

	when "00110000" => DATAO <= CIPHER1(0);
	when "00110001" => DATAO <= CIPHER1(1);
	when "00110010" => DATAO <= CIPHER1(2);
	when "00110011" => DATAO <= CIPHER1(3);
	when "00110100" => DATAO <= CIPHER1(4);
	when "00110101" => DATAO <= CIPHER1(5);
	when "00110110" => DATAO <= CIPHER1(6);
	when "00110111" => DATAO <= CIPHER1(7);

	when "00111000" => DATAO <= PT_BYTE_MASK_REG;
	when "00111111" => DATAO <= "000" & BAA_EN & 
		AA_OUT_BAK & ALL_ACTIVE & SEARCH_INFO_REG(1 downto 0) ;

	when "01000111" => DATAO  <= "000000" & SELECT_ONE(0)  & SEARCH_IN(0);
	when "01001111" => DATAO  <= "000000" & SELECT_ONE(1)  & SEARCH_IN(1);
	when "01010111" => DATAO  <= "000000" & SELECT_ONE(2)  & SEARCH_IN(2);
	when "01011111" => DATAO  <= "000000" & SELECT_ONE(3)  & SEARCH_IN(3);
	when "01100111" => DATAO  <= "000000" & SELECT_ONE(4)  & SEARCH_IN(4);
	when "01101111" => DATAO  <= "000000" & SELECT_ONE(5)  & SEARCH_IN(5);
	when "01110111" => DATAO  <= "000000" & SELECT_ONE(6)  & SEARCH_IN(6);
	when "01111111" => DATAO  <= "000000" & SELECT_ONE(7)  & SEARCH_IN(7);
	when "10000111" => DATAO  <= "000000" & SELECT_ONE(8)  & SEARCH_IN(8);
	when "10001111" => DATAO  <= "000000" & SELECT_ONE(9)  & SEARCH_IN(9);
	when "10010111" => DATAO  <= "000000" & SELECT_ONE(10) & SEARCH_IN(10);
	when "10011111" => DATAO  <= "000000" & SELECT_ONE(11) & SEARCH_IN(11);
	when "10100111" => DATAO  <= "000000" & SELECT_ONE(12) & SEARCH_IN(12);
	when "10101111" => DATAO  <= "000000" & SELECT_ONE(13) & SEARCH_IN(13);
	when "10110111" => DATAO  <= "000000" & SELECT_ONE(14) & SEARCH_IN(14);
	when "10111111" => DATAO  <= "000000" & SELECT_ONE(15) & SEARCH_IN(15);
	when "11000111" => DATAO  <= "000000" & SELECT_ONE(16) & SEARCH_IN(16);
	when "11001111" => DATAO  <= "000000" & SELECT_ONE(17) & SEARCH_IN(17);
	when "11010111" => DATAO  <= "000000" & SELECT_ONE(18) & SEARCH_IN(18);
	when "11011111" => DATAO  <= "000000" & SELECT_ONE(19) & SEARCH_IN(19);
	when "11100111" => DATAO  <= "000000" & SELECT_ONE(20) & SEARCH_IN(20);
	when "11101111" => DATAO  <= "000000" & SELECT_ONE(21) & SEARCH_IN(21);
	when "11110111" => DATAO  <= "000000" & SELECT_ONE(22) & SEARCH_IN(22);
	when "11111111" => DATAO  <= "000000" & SELECT_ONE(23) & SEARCH_IN(23);
	when others     => DATAO <= (others => 'Z');

end case;
else
	DATAO <= (others => 'Z');
end if;
end process READ_PR;
--------------------------------------------------------------------------------
PT_VECTOR_PR: process(RST_N, WRB)
--------------------------------------------------------------------------------
begin   
if (RST_N = '0') then
	for i in 0 to 31 loop
	PT_VECTOR_REG(i) <= (others => '0');
	end loop;

	for i in 0 to 7 loop
	PT_XOR_MASK_REG(i) <= (others => '0');
	CIPHER0(i) <= (others => '0');
	CIPHER1(i) <= (others => '0');
	end loop;

	PT_BYTE_MASK_REG <= (others => '0');
	SEARCH_INFO_REG <= (others => '0');

elsif (WRB'event and WRB= '1') then

if ((CHIP_EN_BAK = '1') and (ADDSEL2 = '0')) then
	case ADDR is
	when "00000000" => PT_VECTOR_REG(0) <= DATAI;
	when "00000001" => PT_VECTOR_REG(1) <= DATAI;
	when "00000010" => PT_VECTOR_REG(2) <= DATAI;
	when "00000011" => PT_VECTOR_REG(3) <= DATAI;
	when "00000100" => PT_VECTOR_REG(4) <= DATAI;
	when "00000101" => PT_VECTOR_REG(5) <= DATAI;
	when "00000110" => PT_VECTOR_REG(6) <= DATAI;
	when "00000111" => PT_VECTOR_REG(7) <= DATAI;
	when "00001000" => PT_VECTOR_REG(8) <= DATAI;
	when "00001001" => PT_VECTOR_REG(9) <= DATAI;
	when "00001010" => PT_VECTOR_REG(10) <= DATAI;
	when "00001011" => PT_VECTOR_REG(11) <= DATAI;
	when "00001100" => PT_VECTOR_REG(12) <= DATAI;
	when "00001101" => PT_VECTOR_REG(13) <= DATAI;
	when "00001110" => PT_VECTOR_REG(14) <= DATAI;
	when "00001111" => PT_VECTOR_REG(15) <= DATAI;
	when "00010000" => PT_VECTOR_REG(16) <= DATAI;
	when "00010001" => PT_VECTOR_REG(17) <= DATAI;
	when "00010010" => PT_VECTOR_REG(18) <= DATAI;
	when "00010011" => PT_VECTOR_REG(19) <= DATAI;
	when "00010100" => PT_VECTOR_REG(20) <= DATAI;
	when "00010101" => PT_VECTOR_REG(21) <= DATAI;
	when "00010110" => PT_VECTOR_REG(22) <= DATAI;
	when "00010111" => PT_VECTOR_REG(23) <= DATAI;
	when "00011000" => PT_VECTOR_REG(24) <= DATAI;
	when "00011001" => PT_VECTOR_REG(25) <= DATAI;
	when "00011010" => PT_VECTOR_REG(26) <= DATAI;
	when "00011011" => PT_VECTOR_REG(27) <= DATAI;
	when "00011100" => PT_VECTOR_REG(28) <= DATAI;
	when "00011101" => PT_VECTOR_REG(29) <= DATAI;
	when "00011110" => PT_VECTOR_REG(30) <= DATAI;
	when "00011111" => PT_VECTOR_REG(31) <= DATAI;

	when "00100000" => PT_XOR_MASK_REG(0) <= DATAI;
	when "00100001" => PT_XOR_MASK_REG(1) <= DATAI;
	when "00100010" => PT_XOR_MASK_REG(2) <= DATAI;
	when "00100011" => PT_XOR_MASK_REG(3) <= DATAI;
	when "00100100" => PT_XOR_MASK_REG(4) <= DATAI;
	when "00100101" => PT_XOR_MASK_REG(5) <= DATAI;
	when "00100110" => PT_XOR_MASK_REG(6) <= DATAI;
	when "00100111" => PT_XOR_MASK_REG(7) <= DATAI;

	when "00101000" => CIPHER0(0) <= DATAI;
	when "00101001" => CIPHER0(1) <= DATAI;
	when "00101010" => CIPHER0(2) <= DATAI;
	when "00101011" => CIPHER0(3) <= DATAI;
	when "00101100" => CIPHER0(4) <= DATAI;
	when "00101101" => CIPHER0(5) <= DATAI;
	when "00101110" => CIPHER0(6) <= DATAI;
	when "00101111" => CIPHER0(7) <= DATAI;

	when "00110000" => CIPHER1(0) <= DATAI;
	when "00110001" => CIPHER1(1) <= DATAI;
	when "00110010" => CIPHER1(2) <= DATAI;
	when "00110011" => CIPHER1(3) <= DATAI;
	when "00110100" => CIPHER1(4) <= DATAI;
	when "00110101" => CIPHER1(5) <= DATAI;
	when "00110110" => CIPHER1(6) <= DATAI;
	when "00110111" => CIPHER1(7) <= DATAI;

	when "00111000" => PT_BYTE_MASK_REG <= DATAI;

	when "00111111" => SEARCH_INFO_REG  <= DATAI;



	 when others => null;
        end case;
end if;  
end if;  
 
end process PT_VECTOR_PR;
 
PT_VECTOR <=    PT_VECTOR_REG(31) & PT_VECTOR_REG(30) & PT_VECTOR_REG(29) & PT_VECTOR_REG(28) &
		PT_VECTOR_REG(27) & PT_VECTOR_REG(26) & PT_VECTOR_REG(25) & PT_VECTOR_REG(24) &
		PT_VECTOR_REG(23) & PT_VECTOR_REG(22) & PT_VECTOR_REG(21) & PT_VECTOR_REG(20) &
		PT_VECTOR_REG(19) & PT_VECTOR_REG(18) & PT_VECTOR_REG(17) & PT_VECTOR_REG(16) &
		PT_VECTOR_REG(15) & PT_VECTOR_REG(14) & PT_VECTOR_REG(13) & PT_VECTOR_REG(12) &
		PT_VECTOR_REG(11) & PT_VECTOR_REG(10) & PT_VECTOR_REG(9)  & PT_VECTOR_REG(8) &
		PT_VECTOR_REG(7)  & PT_VECTOR_REG(6)  & PT_VECTOR_REG(5)  & PT_VECTOR_REG(4) &
		PT_VECTOR_REG(3)  & PT_VECTOR_REG(2)  & PT_VECTOR_REG(1)  & PT_VECTOR_REG(0) ;

PT_XOR_MASK  <=  PT_XOR_MASK_REG(7) & PT_XOR_MASK_REG(6) & PT_XOR_MASK_REG(5) & PT_XOR_MASK_REG(4) & 
                 PT_XOR_MASK_REG(3) & PT_XOR_MASK_REG(2) & PT_XOR_MASK_REG(1) & PT_XOR_MASK_REG(0) ;

C1  <=  CIPHER1(7) & CIPHER1(6) & CIPHER1(5) & CIPHER1(4) & 
        CIPHER1(3) & CIPHER1(2) & CIPHER1(1) & CIPHER1(0) ;
C0  <=  CIPHER0(7) & CIPHER0(6) & CIPHER0(5) & CIPHER0(4) & 
        CIPHER0(3) & CIPHER0(2) & CIPHER0(1) & CIPHER0(0) ;

--------------------------------------------------------------------------------
 

--------------------------------------------------------------------------------
PT_BYTE_MASK 	<= PT_BYTE_MASK_REG;
USE_CBC 	<= SEARCH_INFO_REG(0);
EXTRA_XOR 	<= SEARCH_INFO_REG(1);
BAA_EN 		<= SEARCH_INFO_REG(4);
AA_OUT_BAK	<= AA_IN and ALL_ACTIVE when (BAA_EN = '1') else AA_IN;
AA_OUT 		<= AA_OUT_BAK ;

ALL_ACTIVE	<= ( SEARCH_OUT(23) and SEARCH_OUT(22) and SEARCH_OUT(21) and SEARCH_OUT(20) and
                     SEARCH_OUT(19) and SEARCH_OUT(18) and SEARCH_OUT(17) and SEARCH_OUT(16) and
                     SEARCH_OUT(15) and SEARCH_OUT(14) and SEARCH_OUT(13) and SEARCH_OUT(12) and
                     SEARCH_OUT(11) and SEARCH_OUT(10) and SEARCH_OUT(9)  and  SEARCH_OUT(8) and
                     SEARCH_OUT(7)  and SEARCH_OUT(6)  and SEARCH_OUT(5)  and  SEARCH_OUT(4) and
                     SEARCH_OUT(3)  and SEARCH_OUT(2)  and SEARCH_OUT(1)  and  SEARCH_OUT(0));
CHIP_AA_OUT 	<=  ALL_ACTIVE;
CHIP_EN 	<= CHIP_EN_BAK;
--------------------------------------------------------------------------------
end beh;
--------------------------------------------------------------------------------

