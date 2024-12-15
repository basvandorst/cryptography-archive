---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/27/9&                                  
-- Description	   :  Left and Right 32-bit registers               
-- FILE	NAME       :  p.vhd                                                          
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;


-- -----------------------------------------------------------------------------
entity P is

port(
	P_IN     	: in     std_logic_vector(31 downto 0);
       	P_OUT   	: out    std_logic_vector(31 downto 0)
    );

end P;

-- -----------------------------------------------------------------------------
architecture beh of P is
-- -----------------------------------------------------------------------------
subtype small_integer is INTEGER range 0 to 31;
type P_TYPE is array(0 to 31) of small_integer;

signal P_TABLE : P_TYPE;

begin

P_TABLE <= 	(11,17, 5,27,25,10,20, 0,
		 13,21, 3,28,29, 7,18,24,
		 31,22,12, 6,26, 2,16, 8,
		 14,30, 4,19, 1, 9,15,23);	

-- -----------------------------------------------------------------------------
P_PR: process(P_TABLE,P_IN)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 31 loop
	    P_OUT(P_TABLE(i)) <= P_IN(i);
	end loop;
end process P_PR;
-- -----------------------------------------------------------------------------
end beh;
-- -----------------------------------------------------------------------------
