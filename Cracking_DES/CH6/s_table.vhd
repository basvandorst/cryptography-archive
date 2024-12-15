---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu
-- Date            :  10/02/97                                  
-- Description	   :  Create table for lookup values of S function
-- Function	   :  6 inputs are used to lookup in the table and produce 
--		      4 ouputs.  There are a total of 8 tables
-- -----------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
use ieee.std_logic_arith.conv_std_logic_vector; 


-- -----------------------------------------------------------------------------
entity S_TABLE is

port(  KEY        : in     std_logic_vector(47 downto 0);
       S_OUT      : out    std_logic_vector(31 downto 0)
    );

end S_TABLE;

-- -----------------------------------------------------------------------------
architecture beh of S_TABLE is
-- -----------------------------------------------------------------------------
subtype small_integer is INTEGER range 0 to 15;
type TABLE_TYPE is array(0 to 63) of small_integer;

signal S1 : TABLE_TYPE;
signal S2 : TABLE_TYPE;
signal S3 : TABLE_TYPE;
signal S4 : TABLE_TYPE;
signal S5 : TABLE_TYPE;
signal S6 : TABLE_TYPE;
signal S7 : TABLE_TYPE;
signal S8 : TABLE_TYPE;

-- -----------------------------------------------------------------------------
function lookup(signal table: in TABLE_TYPE;
		signal key: in std_logic_vector(5 downto 0)) 

	 return std_logic_vector is

variable row    : std_logic_vector(3 downto 0);
variable col    : std_logic_vector(1 downto 0);
variable addr   : std_logic_vector(5 downto 0);
variable index  : integer;
variable result : std_logic_vector(3 downto 0);

begin

    col:= key(5) & key(0);
    row:= key(4 downto 1);
    addr:= col & row;
    index:= CONV_INTEGER(key);

    result:= CONV_STD_LOGIC_VECTOR(table(index),4);

    return result;

end  lookup;
-- -----------------------------------------------------------------------------
begin
-- -----------------------------------------------------------------------------

S1 	<=	(13, 1, 2,15, 8,13, 4, 8, 6,10,15, 3,11, 7, 1, 4,
		 10,12, 9, 5, 3, 6,14,11, 5, 0, 0,14,12, 9, 7, 2,
		  7, 2,11, 1, 4,14, 1, 7, 9, 4,12,10,14, 8, 2,13,
		  0,15, 6,12,10, 9,13, 0,15, 3, 3, 5, 5, 6, 8,11);

S2 	<=	( 4,13,11, 0, 2,11,14, 7,15, 4, 0, 9, 8, 1,13,10,
		  3,14,12, 3, 9, 5, 7,12, 5, 2,10,15, 6, 8, 1, 6,
		  1, 6, 4,11,11,13,13, 8,12, 1, 3, 4, 7,10,14, 7,
		 10, 9,15, 5, 6, 0, 8,15, 0,14, 5, 2, 9, 3, 2,12);

S3 	<=	(12,10, 1,15,10, 4,15, 2, 9, 7, 2,12, 6, 9, 8, 5,
		  0, 6,13, 1, 3,13, 4,14,14, 0, 7,11, 5, 3,11, 8,
		  9, 4,14, 3,15, 2, 5,12, 2, 9, 8, 5,12,15, 3,10,
		  7,11, 0,14, 4, 1,10, 7, 1, 6,13, 0,11, 8, 6,13);

S4 	<=	( 2,14,12,11, 4, 2, 1,12, 7, 4,10, 7,11,13, 6, 1,
		  8, 5, 5, 0, 3,15,15,10,13, 3, 0, 9,14, 8, 9, 6,
		  4,11, 2, 8, 1,12,11, 7,10, 1,13,14, 7, 2, 8,13,
		 15, 6, 9,15,12, 0, 5, 9, 6,10, 3, 4, 0, 5,14, 3);

S5 	<=	( 7,13,13, 8,14,11, 3, 5, 0, 6, 6,15, 9, 0,10, 3,
		  1, 4, 2, 7, 8, 2, 5,12,11, 1,12,10, 4,14,15, 9,
		 10, 3, 6,15, 9, 0, 0, 6,12,10,11, 1, 7,13,13, 8,
		 15, 9, 1, 4, 3, 5,14,11, 5,12, 2, 7, 8, 2, 4,14);

S6 	<=	(10,13, 0, 7, 9, 0,14, 9, 6, 3, 3, 4,15, 6, 5,10,
		  1, 2,13, 8,12, 5, 7,14,11,12, 4,11, 2,15, 8, 1,
		 13, 1, 6,10, 4,13, 9, 0, 8, 6,15, 9, 3, 8, 0, 7,
		 11, 4, 1,15, 2,14,12, 3, 5,11,10, 5,14, 2, 7,12);

S7 	<=	(15, 3, 1,13, 8, 4,14, 7, 6,15,11, 2, 3, 8, 4,14,
		  9,12, 7, 0, 2, 1,13,10,12, 6, 0, 9, 5,11,10, 5,
		  0,13,14, 8, 7,10,11, 1,10, 3, 4,15,13, 4, 1, 2,
		  5,11, 8, 6,12, 7, 6,12, 9, 0, 3, 5, 2,14,15, 9);

S8 	<=	(14, 0, 4,15,13, 7, 1, 4, 2,14,15, 2,11,13, 8, 1,
		  3,10,10, 6, 6,12,12,11, 5, 9, 9, 5, 0, 3, 7, 8,
		  4,15, 1,12,14, 8, 8, 2,13, 4, 6, 9, 2, 1,11, 7,
		 15, 5,12,11, 9, 3, 7,14, 3,10,10, 0, 5, 6, 0,13);

S_OUT 	<= 	lookup(S8,KEY(47 downto 42)) &
         	lookup(S7,KEY(41 downto 36)) &
         	lookup(S6,KEY(35 downto 30)) &
         	lookup(S5,KEY(29 downto 24)) &
         	lookup(S4,KEY(23 downto 18)) &
         	lookup(S3,KEY(17 downto 12)) &
         	lookup(S2,KEY(11 downto  6)) &
         	lookup(S1,KEY( 5 downto  0)) ;

-- -----------------------------------------------------------------------------
end beh;
-- -----------------------------------------------------------------------------
