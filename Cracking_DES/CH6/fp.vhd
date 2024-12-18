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

-- -----------------------------------------------------------------------------
entity FP is

port(
        FP_IN           : in     std_logic_vector(63 downto 0);
        FP_OUT          : out    std_logic_vector(63 downto 0)
    );

end FP;

-- -----------------------------------------------------------------------------
architecture beh of FP is
-- -----------------------------------------------------------------------------
subtype small_integer is INTEGER range 0 to 63;
type FP_TYPE is array(0 to 63) of small_integer;

signal FP_TABLE : FP_TYPE;

begin

FP_TABLE <=     (57,49,41,33,25,17, 9, 1,
                 59,51,43,35,27,19,11, 3,
                 61,53,45,37,29,21,13, 5,
                 63,55,47,39,31,23,15, 7,
                 56,48,40,32,24,16, 8, 0,
                 58,50,42,34,26,18,10, 2,
                 60,52,44,36,28,20,12, 4,
                 62,54,46,38,30,22,14, 6);


-- -----------------------------------------------------------------------------
FP_PR: process(FP_TABLE,FP_IN)
-- -----------------------------------------------------------------------------
begin
        for i in 0 to 63 loop
            FP_OUT(FP_TABLE(i)) <= FP_IN(i);
        end loop;
end process FP_PR;
-- -----------------------------------------------------------------------------
end beh;

