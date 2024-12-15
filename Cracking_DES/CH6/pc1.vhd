---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu
-- Date            :  10/02/97                                  
-- Description	   :  Generate Permutation Choice #1
-- Function	   :  Array has the table which tells the mapping
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;


-- -----------------------------------------------------------------------------
entity PC1 is

port(   
	KEY_IN     : in     std_logic_vector(55 downto 0);
       	KEY_OUT    : out    std_logic_vector(55 downto 0)
    );
end PC1;
-- -----------------------------------------------------------------------------
architecture beh of PC1 is
-- -----------------------------------------------------------------------------
subtype small_integer is INTEGER range 0 to 55;
type PC1_TYPE is array(0 to 55) of small_integer;
signal PC1_TABLE : PC1_TYPE;

begin

PC1_TABLE <= 	(27,19,11,31,39,47,55,
		 26,18,10,30,38,46,54,
		 25,17, 9,29,37,45,53,
		 24,16, 8,28,36,44,52,
		 23,15, 7, 3,35,43,51,
		 22,14, 6, 2,34,42,50,
		 21,13, 5, 1,33,41,49,
		 20,12, 4, 0,32,40,48);	
-- -----------------------------------------------------------------------------
Permutation_choice_1: process(KEY_IN,PC1_TABLE)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 55 loop
	    KEY_OUT(PC1_TABLE(i)) <= KEY_IN(i);
	end loop;
end process;
-- -----------------------------------------------------------------------------
end beh;
-- -----------------------------------------------------------------------------
