---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu 
-- Date            :  10/02/97                                  
-- Description	   :  Generate Permutation Choice #2
-- Function	   :  Array has the table which tells the mapping
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;


-- -----------------------------------------------------------------------------
entity PC2 is

port(   
	KEY_IN     : in     std_logic_vector(55 downto 0);
       	KEY_OUT    : out    std_logic_vector(47 downto 0)
    );

end PC2;

-- -----------------------------------------------------------------------------
architecture beh of PC2 is
-- -----------------------------------------------------------------------------
subtype small_integer is INTEGER range 0 to 55;
type PC2_TYPE is array(0 to 47) of small_integer;
signal PC2_TABLE : PC2_TYPE;

begin

PC2_TABLE<= 	(24,27,20, 6,14,10, 3,22,
		  0,17, 7,12, 8,23,11, 5,
		 16,26, 1, 9,19,25, 4,15,
		 54,43,36,29,49,40,48,30,
	 	 52,44,37,33,46,35,50,41,
		 28,53,51,55,32,45,39,42);
-- -----------------------------------------------------------------------------
Permutation_choice_2: process(KEY_IN,PC2_TABLE)
-- -----------------------------------------------------------------------------
begin
	for i in 0 to 47 loop
	    KEY_OUT(i) <= KEY_IN(PC2_TABLE(i));
	end loop;
end process;
-- -----------------------------------------------------------------------------
end beh;
-- -----------------------------------------------------------------------------
