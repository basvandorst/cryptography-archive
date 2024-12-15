---------|---------|---------|---------|---------|---------|---------|---------|
-- Author          :  Tom Vu                                     
-- Date            :  09/07/97                                  
-- Description	   :  Search Unit, 24 search units per ASIC
--------------------------------------------------------------------------------
library ieee;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;
--------------------------------------------------------------------------------
entity MUX256 is

port(   

        SHIFT_OUT      	: in  	 std_logic_vector(7 downto 0);
        PT_VECTOR       : in     std_logic_vector(255 downto 0);
        BIT_MUX 	: out     std_logic
    );


end MUX256;

--------------------------------------------------------------------------------
architecture beh of MUX256 is
--------------------------------------------------------------------------------


begin
--------------------------------------------------------------------------------
DECODER_PR: process(SHIFT_OUT,PT_VECTOR)
--------------------------------------------------------------------------------
variable ii 	: integer;
begin
ii := conv_integer(SHIFT_OUT);
case ii is 
	when   0 => BIT_MUX <= PT_VECTOR(0);
	when   1 => BIT_MUX <= PT_VECTOR(1);
	when   2 => BIT_MUX <= PT_VECTOR(2);
	when   3 => BIT_MUX <= PT_VECTOR(3);

	when   4 => BIT_MUX <= PT_VECTOR(4);
	when   5 => BIT_MUX <= PT_VECTOR(5);
	when   6 => BIT_MUX <= PT_VECTOR(6);
	when   7 => BIT_MUX <= PT_VECTOR(7);

	when   8 => BIT_MUX <= PT_VECTOR(8);
	when   9 => BIT_MUX <= PT_VECTOR(9);
	when  10 => BIT_MUX <= PT_VECTOR(10);
	when  11 => BIT_MUX <= PT_VECTOR(11);

	when  12 => BIT_MUX <= PT_VECTOR(12);
	when  13 => BIT_MUX <= PT_VECTOR(13);
	when  14 => BIT_MUX <= PT_VECTOR(14);
	when  15 => BIT_MUX <= PT_VECTOR(15);

	when  16 => BIT_MUX <= PT_VECTOR(16);
	when  17 => BIT_MUX <= PT_VECTOR(17);
	when  18 => BIT_MUX <= PT_VECTOR(18);
	when  19 => BIT_MUX <= PT_VECTOR(19);

	when  20 => BIT_MUX <= PT_VECTOR(20);
	when  21 => BIT_MUX <= PT_VECTOR(21);
	when  22 => BIT_MUX <= PT_VECTOR(22);
	when  23 => BIT_MUX <= PT_VECTOR(23);

	when  24 => BIT_MUX <= PT_VECTOR(24);
	when  25 => BIT_MUX <= PT_VECTOR(25);
	when  26 => BIT_MUX <= PT_VECTOR(26);
	when  27 => BIT_MUX <= PT_VECTOR(27);

	when  28 => BIT_MUX <= PT_VECTOR(28);
	when  29 => BIT_MUX <= PT_VECTOR(29);
	when  30 => BIT_MUX <= PT_VECTOR(30);
	when  31 => BIT_MUX <= PT_VECTOR(31);
	when  32 => BIT_MUX <= PT_VECTOR(32);
	when  33 => BIT_MUX <= PT_VECTOR(33);
	when  34 => BIT_MUX <= PT_VECTOR(34);
	when  35 => BIT_MUX <= PT_VECTOR(35);
	when  36 => BIT_MUX <= PT_VECTOR(36);
	when  37 => BIT_MUX <= PT_VECTOR(37);
	when  38 => BIT_MUX <= PT_VECTOR(38);
	when  39 => BIT_MUX <= PT_VECTOR(39);
	when  40 => BIT_MUX <= PT_VECTOR(40);
	when  41 => BIT_MUX <= PT_VECTOR(41);
	when  42 => BIT_MUX <= PT_VECTOR(42);
	when  43 => BIT_MUX <= PT_VECTOR(43);
	when  44 => BIT_MUX <= PT_VECTOR(44);
	when  45 => BIT_MUX <= PT_VECTOR(45);
	when  46 => BIT_MUX <= PT_VECTOR(46);
	when  47 => BIT_MUX <= PT_VECTOR(47);
	when  48 => BIT_MUX <= PT_VECTOR(48);
	when  49 => BIT_MUX <= PT_VECTOR(49);
	when  50 => BIT_MUX <= PT_VECTOR(50);
	when  51 => BIT_MUX <= PT_VECTOR(51);
	when  52 => BIT_MUX <= PT_VECTOR(52);
	when  53 => BIT_MUX <= PT_VECTOR(53);
	when  54 => BIT_MUX <= PT_VECTOR(54);
	when  55 => BIT_MUX <= PT_VECTOR(55);
	when  56 => BIT_MUX <= PT_VECTOR(56);
	when  57 => BIT_MUX <= PT_VECTOR(57);
	when  58 => BIT_MUX <= PT_VECTOR(58);
	when  59 => BIT_MUX <= PT_VECTOR(59);
	when  60 => BIT_MUX <= PT_VECTOR(60);
	when  61 => BIT_MUX <= PT_VECTOR(61);
	when  62 => BIT_MUX <= PT_VECTOR(62);
	when  63 => BIT_MUX <= PT_VECTOR(63);
	when  64 => BIT_MUX <= PT_VECTOR(64);
	when  65 => BIT_MUX <= PT_VECTOR(65);
	when  66 => BIT_MUX <= PT_VECTOR(66);
	when  67 => BIT_MUX <= PT_VECTOR(67);
	when  68 => BIT_MUX <= PT_VECTOR(68);
	when  69 => BIT_MUX <= PT_VECTOR(69);
	when  70 => BIT_MUX <= PT_VECTOR(70);
	when  71 => BIT_MUX <= PT_VECTOR(71);
	when  72 => BIT_MUX <= PT_VECTOR(72);
	when  73 => BIT_MUX <= PT_VECTOR(73);
	when  74 => BIT_MUX <= PT_VECTOR(74);
	when  75 => BIT_MUX <= PT_VECTOR(75);
	when  76 => BIT_MUX <= PT_VECTOR(76);
	when  77 => BIT_MUX <= PT_VECTOR(77);
	when  78 => BIT_MUX <= PT_VECTOR(78);
	when  79 => BIT_MUX <= PT_VECTOR(79);
	when  80 => BIT_MUX <= PT_VECTOR(80);
	when  81 => BIT_MUX <= PT_VECTOR(81);
	when  82 => BIT_MUX <= PT_VECTOR(82);
	when  83 => BIT_MUX <= PT_VECTOR(83);
	when  84 => BIT_MUX <= PT_VECTOR(84);
	when  85 => BIT_MUX <= PT_VECTOR(85);
	when  86 => BIT_MUX <= PT_VECTOR(86);
	when  87 => BIT_MUX <= PT_VECTOR(87);
	when  88 => BIT_MUX <= PT_VECTOR(88);
	when  89 => BIT_MUX <= PT_VECTOR(89);
	when  90 => BIT_MUX <= PT_VECTOR(90);
	when  91 => BIT_MUX <= PT_VECTOR(91);
	when  92 => BIT_MUX <= PT_VECTOR(92);
	when  93 => BIT_MUX <= PT_VECTOR(93);
	when  94 => BIT_MUX <= PT_VECTOR(94);
	when  95 => BIT_MUX <= PT_VECTOR(95);
	when  96 => BIT_MUX <= PT_VECTOR(96);
	when  97 => BIT_MUX <= PT_VECTOR(97);
	when  98 => BIT_MUX <= PT_VECTOR(98);
	when  99 => BIT_MUX <= PT_VECTOR(99);

	when 100 => BIT_MUX <= PT_VECTOR(100);
	when 101 => BIT_MUX <= PT_VECTOR(101);
	when 102 => BIT_MUX <= PT_VECTOR(102);
	when 103 => BIT_MUX <= PT_VECTOR(103);
	when 104 => BIT_MUX <= PT_VECTOR(104);
	when 105 => BIT_MUX <= PT_VECTOR(105);
	when 106 => BIT_MUX <= PT_VECTOR(106);
	when 107 => BIT_MUX <= PT_VECTOR(107);
	when 108 => BIT_MUX <= PT_VECTOR(108);
	when 109 => BIT_MUX <= PT_VECTOR(109);
	when 110 => BIT_MUX <= PT_VECTOR(110);
	when 111 => BIT_MUX <= PT_VECTOR(111);
	when 112 => BIT_MUX <= PT_VECTOR(112);
	when 113 => BIT_MUX <= PT_VECTOR(113);
	when 114 => BIT_MUX <= PT_VECTOR(114);
	when 115 => BIT_MUX <= PT_VECTOR(115);
	when 116 => BIT_MUX <= PT_VECTOR(116);
	when 117 => BIT_MUX <= PT_VECTOR(117);
	when 118 => BIT_MUX <= PT_VECTOR(118);
	when 119 => BIT_MUX <= PT_VECTOR(119);
	when 120 => BIT_MUX <= PT_VECTOR(120);
	when 121 => BIT_MUX <= PT_VECTOR(121);
	when 122 => BIT_MUX <= PT_VECTOR(122);
	when 123 => BIT_MUX <= PT_VECTOR(123);
	when 124 => BIT_MUX <= PT_VECTOR(124);
	when 125 => BIT_MUX <= PT_VECTOR(125);
	when 126 => BIT_MUX <= PT_VECTOR(126);
	when 127 => BIT_MUX <= PT_VECTOR(127);
	when 128 => BIT_MUX <= PT_VECTOR(128);
	when 129 => BIT_MUX <= PT_VECTOR(129);
	when 130 => BIT_MUX <= PT_VECTOR(130);
	when 131 => BIT_MUX <= PT_VECTOR(131);
	when 132 => BIT_MUX <= PT_VECTOR(132);
	when 133 => BIT_MUX <= PT_VECTOR(133);
	when 134 => BIT_MUX <= PT_VECTOR(134);
	when 135 => BIT_MUX <= PT_VECTOR(135);
	when 136 => BIT_MUX <= PT_VECTOR(136);
	when 137 => BIT_MUX <= PT_VECTOR(137);
	when 138 => BIT_MUX <= PT_VECTOR(138);
	when 139 => BIT_MUX <= PT_VECTOR(139);
	when 140 => BIT_MUX <= PT_VECTOR(140);
	when 141 => BIT_MUX <= PT_VECTOR(141);
	when 142 => BIT_MUX <= PT_VECTOR(142);
	when 143 => BIT_MUX <= PT_VECTOR(143);
	when 144 => BIT_MUX <= PT_VECTOR(144);
	when 145 => BIT_MUX <= PT_VECTOR(145);
	when 146 => BIT_MUX <= PT_VECTOR(146);
	when 147 => BIT_MUX <= PT_VECTOR(147);
	when 148 => BIT_MUX <= PT_VECTOR(148);
	when 149 => BIT_MUX <= PT_VECTOR(149);
	when 150 => BIT_MUX <= PT_VECTOR(150);
	when 151 => BIT_MUX <= PT_VECTOR(151);
	when 152 => BIT_MUX <= PT_VECTOR(152);
	when 153 => BIT_MUX <= PT_VECTOR(153);
	when 154 => BIT_MUX <= PT_VECTOR(154);
	when 155 => BIT_MUX <= PT_VECTOR(155);
	when 156 => BIT_MUX <= PT_VECTOR(156);
	when 157 => BIT_MUX <= PT_VECTOR(157);
	when 158 => BIT_MUX <= PT_VECTOR(158);
	when 159 => BIT_MUX <= PT_VECTOR(159);
	when 160 => BIT_MUX <= PT_VECTOR(160);
	when 161 => BIT_MUX <= PT_VECTOR(161);
	when 162 => BIT_MUX <= PT_VECTOR(162);
	when 163 => BIT_MUX <= PT_VECTOR(163);
	when 164 => BIT_MUX <= PT_VECTOR(164);
	when 165 => BIT_MUX <= PT_VECTOR(165);
	when 166 => BIT_MUX <= PT_VECTOR(166);
	when 167 => BIT_MUX <= PT_VECTOR(167);
	when 168 => BIT_MUX <= PT_VECTOR(168);
	when 169 => BIT_MUX <= PT_VECTOR(169);
	when 170 => BIT_MUX <= PT_VECTOR(170);
	when 171 => BIT_MUX <= PT_VECTOR(171);
	when 172 => BIT_MUX <= PT_VECTOR(172);
	when 173 => BIT_MUX <= PT_VECTOR(173);
	when 174 => BIT_MUX <= PT_VECTOR(174);
	when 175 => BIT_MUX <= PT_VECTOR(175);
	when 176 => BIT_MUX <= PT_VECTOR(176);
	when 177 => BIT_MUX <= PT_VECTOR(177);
	when 178 => BIT_MUX <= PT_VECTOR(178);
	when 179 => BIT_MUX <= PT_VECTOR(179);
	when 180 => BIT_MUX <= PT_VECTOR(180);
	when 181 => BIT_MUX <= PT_VECTOR(181);
	when 182 => BIT_MUX <= PT_VECTOR(182);
	when 183 => BIT_MUX <= PT_VECTOR(183);
	when 184 => BIT_MUX <= PT_VECTOR(184);
	when 185 => BIT_MUX <= PT_VECTOR(185);
	when 186 => BIT_MUX <= PT_VECTOR(186);
	when 187 => BIT_MUX <= PT_VECTOR(187);
	when 188 => BIT_MUX <= PT_VECTOR(188);
	when 189 => BIT_MUX <= PT_VECTOR(189);
	when 190 => BIT_MUX <= PT_VECTOR(190);
	when 191 => BIT_MUX <= PT_VECTOR(191);
	when 192 => BIT_MUX <= PT_VECTOR(192);
	when 193 => BIT_MUX <= PT_VECTOR(193);
	when 194 => BIT_MUX <= PT_VECTOR(194);
	when 195 => BIT_MUX <= PT_VECTOR(195);
	when 196 => BIT_MUX <= PT_VECTOR(196);
	when 197 => BIT_MUX <= PT_VECTOR(197);
	when 198 => BIT_MUX <= PT_VECTOR(198);
	when 199 => BIT_MUX <= PT_VECTOR(199);

	when 200 => BIT_MUX <= PT_VECTOR(200);
	when 201 => BIT_MUX <= PT_VECTOR(201);
	when 202 => BIT_MUX <= PT_VECTOR(202);
	when 203 => BIT_MUX <= PT_VECTOR(203);
	when 204 => BIT_MUX <= PT_VECTOR(204);
	when 205 => BIT_MUX <= PT_VECTOR(205);
	when 206 => BIT_MUX <= PT_VECTOR(206);
	when 207 => BIT_MUX <= PT_VECTOR(207);
	when 208 => BIT_MUX <= PT_VECTOR(208);
	when 209 => BIT_MUX <= PT_VECTOR(209);
	when 210 => BIT_MUX <= PT_VECTOR(210);
	when 211 => BIT_MUX <= PT_VECTOR(211);
	when 212 => BIT_MUX <= PT_VECTOR(212);
	when 213 => BIT_MUX <= PT_VECTOR(213);
	when 214 => BIT_MUX <= PT_VECTOR(214);
	when 215 => BIT_MUX <= PT_VECTOR(215);
	when 216 => BIT_MUX <= PT_VECTOR(216);
	when 217 => BIT_MUX <= PT_VECTOR(217);
	when 218 => BIT_MUX <= PT_VECTOR(218);
	when 219 => BIT_MUX <= PT_VECTOR(219);
	when 220 => BIT_MUX <= PT_VECTOR(220);
	when 221 => BIT_MUX <= PT_VECTOR(221);
	when 222 => BIT_MUX <= PT_VECTOR(222);
	when 223 => BIT_MUX <= PT_VECTOR(223);
	when 224 => BIT_MUX <= PT_VECTOR(224);
	when 225 => BIT_MUX <= PT_VECTOR(225);
	when 226 => BIT_MUX <= PT_VECTOR(226);
	when 227 => BIT_MUX <= PT_VECTOR(227);
	when 228 => BIT_MUX <= PT_VECTOR(228);
	when 229 => BIT_MUX <= PT_VECTOR(229);
	when 230 => BIT_MUX <= PT_VECTOR(230);
	when 231 => BIT_MUX <= PT_VECTOR(231);
	when 232 => BIT_MUX <= PT_VECTOR(232);
	when 233 => BIT_MUX <= PT_VECTOR(233);
	when 234 => BIT_MUX <= PT_VECTOR(234);
	when 235 => BIT_MUX <= PT_VECTOR(235);
	when 236 => BIT_MUX <= PT_VECTOR(236);
	when 237 => BIT_MUX <= PT_VECTOR(237);
	when 238 => BIT_MUX <= PT_VECTOR(238);
	when 239 => BIT_MUX <= PT_VECTOR(239);
	when 240 => BIT_MUX <= PT_VECTOR(240);
	when 241 => BIT_MUX <= PT_VECTOR(241);
	when 242 => BIT_MUX <= PT_VECTOR(242);
	when 243 => BIT_MUX <= PT_VECTOR(243);
	when 244 => BIT_MUX <= PT_VECTOR(244);
	when 245 => BIT_MUX <= PT_VECTOR(245);
	when 246 => BIT_MUX <= PT_VECTOR(246);
	when 247 => BIT_MUX <= PT_VECTOR(247);
	when 248 => BIT_MUX <= PT_VECTOR(248);
	when 249 => BIT_MUX <= PT_VECTOR(249);
	when 250 => BIT_MUX <= PT_VECTOR(250);
	when 251 => BIT_MUX <= PT_VECTOR(251);
	when 252 => BIT_MUX <= PT_VECTOR(252);
	when 253 => BIT_MUX <= PT_VECTOR(253);
	when 254 => BIT_MUX <= PT_VECTOR(254);
	when 255 => BIT_MUX <= PT_VECTOR(255);

	when others => BIT_MUX <= '0';
 end case;

end process DECODER_PR;

--------------------------------------------------------------------------------
end beh;
--------------------------------------------------------------------------------

