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
entity ADDR_KEY is

port(   
        ADDSEL2        	: in     std_logic;
        CHIP_EN        	: in     std_logic;
        ADDR       	: in     std_logic_vector(7 downto 0);

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
        DATAI       	: in     std_logic_vector(7 downto 0)
    );


end ADDR_KEY;

--------------------------------------------------------------------------------
architecture beh of ADDR_KEY is
--------------------------------------------------------------------------------

begin
 
ADDR_KEY0(0) <=  '1' when ((ADDR =  "01000000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(1) <=  '1' when ((ADDR =  "01000001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(2) <=  '1' when ((ADDR =  "01000010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(3) <=  '1' when ((ADDR =  "01000011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(4) <=  '1' when ((ADDR =  "01000100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(5) <=  '1' when ((ADDR =  "01000101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY0(6) <=  '1' when ((ADDR =  "01000110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY1(0) <=  '1' when ((ADDR =  "01001000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(1) <=  '1' when ((ADDR =  "01001001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(2) <=  '1' when ((ADDR =  "01001010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(3) <=  '1' when ((ADDR =  "01001011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(4) <=  '1' when ((ADDR =  "01001100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(5) <=  '1' when ((ADDR =  "01001101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY1(6) <=  '1' when ((ADDR =  "01001110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY2(0) <=  '1' when ((ADDR =  "01010000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(1) <=  '1' when ((ADDR =  "01010001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(2) <=  '1' when ((ADDR =  "01010010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(3) <=  '1' when ((ADDR =  "01010011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(4) <=  '1' when ((ADDR =  "01010100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(5) <=  '1' when ((ADDR =  "01010101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY2(6) <=  '1' when ((ADDR =  "01010110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY3(0) <=  '1' when ((ADDR =  "01011000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(1) <=  '1' when ((ADDR =  "01011001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(2) <=  '1' when ((ADDR =  "01011010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(3) <=  '1' when ((ADDR =  "01011011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(4) <=  '1' when ((ADDR =  "01011100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(5) <=  '1' when ((ADDR =  "01011101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY3(6) <=  '1' when ((ADDR =  "01011110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY4(0) <=  '1' when ((ADDR =  "01100000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(1) <=  '1' when ((ADDR =  "01100001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(2) <=  '1' when ((ADDR =  "01100010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(3) <=  '1' when ((ADDR =  "01100011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(4) <=  '1' when ((ADDR =  "01100100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(5) <=  '1' when ((ADDR =  "01100101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY4(6) <=  '1' when ((ADDR =  "01100110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY5(0) <=	 '1' when ((ADDR =  "01101000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(1) <=	 '1' when ((ADDR =  "01101001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(2) <=	 '1' when ((ADDR =  "01101010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(3) <=	 '1' when ((ADDR =  "01101011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(4) <=	 '1' when ((ADDR =  "01101100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(5) <=	 '1' when ((ADDR =  "01101101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY5(6) <=	 '1' when ((ADDR =  "01101110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY6(0) <=	 '1' when ((ADDR =  "01110000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(1) <=	 '1' when ((ADDR =  "01110001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(2) <=	 '1' when ((ADDR =  "01110010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(3) <=	 '1' when ((ADDR =  "01110011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(4) <=	 '1' when ((ADDR =  "01110100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(5) <=	 '1' when ((ADDR =  "01110101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY6(6) <=	 '1' when ((ADDR =  "01110110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY7(0) <=	 '1' when ((ADDR =  "01111000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(1) <=	 '1' when ((ADDR =  "01111001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(2) <=	 '1' when ((ADDR =  "01111010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(3) <=	 '1' when ((ADDR =  "01111011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(4) <=	 '1' when ((ADDR =  "01111100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(5) <=	 '1' when ((ADDR =  "01111101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY7(6) <=	 '1' when ((ADDR =  "01111110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY8(0) <=	 '1' when ((ADDR =  "10000000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(1) <=	 '1' when ((ADDR =  "10000001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(2) <=	 '1' when ((ADDR =  "10000010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(3) <=	 '1' when ((ADDR =  "10000011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(4) <=	 '1' when ((ADDR =  "10000100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(5) <=	 '1' when ((ADDR =  "10000101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY8(6) <=	 '1' when ((ADDR =  "10000110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY9(0) <=	 '1' when ((ADDR =  "10001000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(1) <=	 '1' when ((ADDR =  "10001001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(2) <=	 '1' when ((ADDR =  "10001010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(3) <=	 '1' when ((ADDR =  "10001011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(4) <=	 '1' when ((ADDR =  "10001100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(5) <=	 '1' when ((ADDR =  "10001101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY9(6) <=	 '1' when ((ADDR =  "10001110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY10(0) <= '1' when ((ADDR =  "10010000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(1) <= '1' when ((ADDR =  "10010001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(2) <= '1' when ((ADDR =  "10010010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(3) <= '1' when ((ADDR =  "10010011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(4) <= '1' when ((ADDR =  "10010100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(5) <= '1' when ((ADDR =  "10010101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY10(6) <= '1' when ((ADDR =  "10010110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY11(0) <= '1' when ((ADDR =  "10011000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(1) <= '1' when ((ADDR =  "10011001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(2) <= '1' when ((ADDR =  "10011010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(3) <= '1' when ((ADDR =  "10011011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(4) <= '1' when ((ADDR =  "10011100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(5) <= '1' when ((ADDR =  "10011101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY11(6) <= '1' when ((ADDR =  "10011110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY12(0) <= '1' when ((ADDR =  "10100000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(1) <= '1' when ((ADDR =  "10100001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(2) <= '1' when ((ADDR =  "10100010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(3) <= '1' when ((ADDR =  "10100011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(4) <= '1' when ((ADDR =  "10100100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(5) <= '1' when ((ADDR =  "10100101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY12(6) <= '1' when ((ADDR =  "10100110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY13(0) <= '1' when ((ADDR =  "10101000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(1) <= '1' when ((ADDR =  "10101001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(2) <= '1' when ((ADDR =  "10101010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(3) <= '1' when ((ADDR =  "10101011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(4) <= '1' when ((ADDR =  "10101100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(5) <= '1' when ((ADDR =  "10101101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY13(6) <= '1' when ((ADDR =  "10101110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY14(0) <= '1' when ((ADDR =  "10110000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(1) <= '1' when ((ADDR =  "10110001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(2) <= '1' when ((ADDR =  "10110010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(3) <= '1' when ((ADDR =  "10110011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(4) <= '1' when ((ADDR =  "10110100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(5) <= '1' when ((ADDR =  "10110101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY14(6) <= '1' when ((ADDR =  "10110110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY15(0) <= '1' when ((ADDR =  "10111000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(1) <= '1' when ((ADDR =  "10111001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(2) <= '1' when ((ADDR =  "10111010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(3) <= '1' when ((ADDR =  "10111011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(4) <= '1' when ((ADDR =  "10111100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(5) <= '1' when ((ADDR =  "10111101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY15(6) <= '1' when ((ADDR =  "10111110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY16(0) <= '1' when ((ADDR =  "11000000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(1) <= '1' when ((ADDR =  "11000001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(2) <= '1' when ((ADDR =  "11000010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(3) <= '1' when ((ADDR =  "11000011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(4) <= '1' when ((ADDR =  "11000100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(5) <= '1' when ((ADDR =  "11000101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY16(6) <= '1' when ((ADDR =  "11000110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY17(0) <= '1' when ((ADDR =  "11001000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(1) <= '1' when ((ADDR =  "11001001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(2) <= '1' when ((ADDR =  "11001010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(3) <= '1' when ((ADDR =  "11001011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(4) <= '1' when ((ADDR =  "11001100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(5) <= '1' when ((ADDR =  "11001101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY17(6) <= '1' when ((ADDR =  "11001110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY18(0) <= '1' when ((ADDR =  "11010000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(1) <= '1' when ((ADDR =  "11010001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(2) <= '1' when ((ADDR =  "11010010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(3) <= '1' when ((ADDR =  "11010011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(4) <= '1' when ((ADDR =  "11010100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(5) <= '1' when ((ADDR =  "11010101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY18(6) <= '1' when ((ADDR =  "11010110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY19(0) <= '1' when ((ADDR =  "11011000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(1) <= '1' when ((ADDR =  "11011001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(2) <= '1' when ((ADDR =  "11011010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(3) <= '1' when ((ADDR =  "11011011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(4) <= '1' when ((ADDR =  "11011100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(5) <= '1' when ((ADDR =  "11011101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY19(6) <= '1' when ((ADDR =  "11011110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY20(0) <= '1' when ((ADDR =  "11100000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(1) <= '1' when ((ADDR =  "11100001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(2) <= '1' when ((ADDR =  "11100010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(3) <= '1' when ((ADDR =  "11100011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(4) <= '1' when ((ADDR =  "11100100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(5) <= '1' when ((ADDR =  "11100101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY20(6) <= '1' when ((ADDR =  "11100110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY21(0) <= '1' when ((ADDR =  "11101000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(1) <= '1' when ((ADDR =  "11101001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(2) <= '1' when ((ADDR =  "11101010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(3) <= '1' when ((ADDR =  "11101011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(4) <= '1' when ((ADDR =  "11101100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(5) <= '1' when ((ADDR =  "11101101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY21(6) <= '1' when ((ADDR =  "11101110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY22(0) <= '1' when ((ADDR =  "11110000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(1) <= '1' when ((ADDR =  "11110001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(2) <= '1' when ((ADDR =  "11110010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(3) <= '1' when ((ADDR =  "11110011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(4) <= '1' when ((ADDR =  "11110100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(5) <= '1' when ((ADDR =  "11110101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY22(6) <= '1' when ((ADDR =  "11110110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

ADDR_KEY23(0) <= '1' when ((ADDR =  "11111000") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(1) <= '1' when ((ADDR =  "11111001") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(2) <= '1' when ((ADDR =  "11111010") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(3) <= '1' when ((ADDR =  "11111011") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(4) <= '1' when ((ADDR =  "11111100") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(5) <= '1' when ((ADDR =  "11111101") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';
ADDR_KEY23(6) <= '1' when ((ADDR =  "11111110") and (CHIP_EN = '1') and (ADDSEL2 = '0')) else '0';

--------------------------------------------------------------------------------
end beh;
--------------------------------------------------------------------------------

