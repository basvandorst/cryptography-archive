/* __mp_bases -- Structure for conversion between internal binary
   format and strings in base 2..36.  The fields are explained in
   gmp-impl.h.

   ***** THIS FILE WAS CREATED BY A PROGRAM.  DON'T EDIT IT! *****

Copyright (C) 1991 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2, or
(at your option) any later version.

The GNU MP Library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU MP Library; see the file COPYING.  If not, write
to the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
USA.  */

#include "gmp.h"
#include "gmp-impl.h"

const struct bases __mp_bases[256] =
{
  /*  0 */ {0, 0, 0, 0.0},
  /*  1 */ {0, 0, 0, 0.0},
  /*  2 */ {32, 0x1, 0x0, 1.00000000},
  /*  3 */ {20, 0xCFD41B91, 0x3B563C24, 0.63092975},
  /*  4 */ {16, 0x2, 0x0, 0.50000000},
  /*  5 */ {13, 0x48C27395, 0xC25C2684, 0.43067656},
  /*  6 */ {12, 0x81BF1000, 0xF91BD1B6, 0.38685281},
  /*  7 */ {11, 0x75DB9C97, 0x1607A2CB, 0.35620719},
  /*  8 */ {10, 0x3, 0x0, 0.33333333},
  /*  9 */ {10, 0xCFD41B91, 0x3B563C24, 0.31546488},
  /* 10 */ {9, 0x3B9ACA00, 0x12E0BE82, 0.30103000},
  /* 11 */ {9, 0x8C8B6D2B, 0xD24CDE04, 0.28906483},
  /* 12 */ {8, 0x19A10000, 0x3FA39AB5, 0.27894295},
  /* 13 */ {8, 0x309F1021, 0x50F8AC5F, 0.27023815},
  /* 14 */ {8, 0x57F6C100, 0x74843B1E, 0.26264954},
  /* 15 */ {8, 0x98C29B81, 0xAD0326C2, 0.25595802},
  /* 16 */ {8, 0x4, 0x0, 0.25000000},
  /* 17 */ {7, 0x18754571, 0x4EF0B6BD, 0.24465054},
  /* 18 */ {7, 0x247DBC80, 0xC0FC48A1, 0.23981247},
  /* 19 */ {7, 0x3547667B, 0x33838942, 0.23540891},
  /* 20 */ {7, 0x4C4B4000, 0xAD7F29AB, 0.23137821},
  /* 21 */ {7, 0x6B5A6E1D, 0x313C3D15, 0.22767025},
  /* 22 */ {7, 0x94ACE180, 0xB8CCA9E0, 0.22424382},
  /* 23 */ {7, 0xCAF18367, 0x42ED6DE9, 0.22106473},
  /* 24 */ {6, 0xB640000, 0x67980E0B, 0.21810429},
  /* 25 */ {6, 0xE8D4A51, 0x19799812, 0.21533828},
  /* 26 */ {6, 0x1269AE40, 0xBCE85396, 0.21274605},
  /* 27 */ {6, 0x17179149, 0x62C103A9, 0.21030992},
  /* 28 */ {6, 0x1CB91000, 0x1D353D43, 0.20801460},
  /* 29 */ {6, 0x23744899, 0xCE1DECEA, 0.20584683},
  /* 30 */ {6, 0x2B73A840, 0x790FC511, 0.20379505},
  /* 31 */ {6, 0x34E63B41, 0x35B865A0, 0.20184909},
  /* 32 */ {6, 0x5, 0x0, 0.20000000},
  /* 33 */ {6, 0x4CFA3CC1, 0xA9AED1B3, 0.19823986},
  /* 34 */ {6, 0x5C13D840, 0x63DFC229, 0.19656163},
  /* 35 */ {6, 0x6D91B519, 0x2B0FEE30, 0.19495902},
  /* 36 */ {6, 0x81BF1000, 0xF91BD1B6, 0.19342640},
  /* 37 */ {6, 0x98EDE0C9, 0xAC89C3A9, 0.19195872},
  /* 38 */ {6, 0xB3773E40, 0x6D2C32FE, 0.19055141},
  /* 39 */ {6, 0xD1BBC4D1, 0x387907C9, 0.18920036},
  /* 40 */ {6, 0xF4240000, 0xC6F7A0B, 0.18790182},
  /* 41 */ {5, 0x6E7D349, 0x28928154, 0.18665241},
  /* 42 */ {5, 0x7CA30A0, 0x6E8629D, 0.18544902},
  /* 43 */ {5, 0x8C32BBB, 0xD373DCA0, 0.18428883},
  /* 44 */ {5, 0x9D46C00, 0xA0B17895, 0.18316925},
  /* 45 */ {5, 0xAFFACFD, 0x746811A5, 0.18208790},
  /* 46 */ {5, 0xC46BEE0, 0x4DA6500F, 0.18104260},
  /* 47 */ {5, 0xDAB86EF, 0x2BA23582, 0.18003133},
  /* 48 */ {5, 0xF300000, 0xDB20A88, 0.17905223},
  /* 49 */ {5, 0x10D63AF1, 0xE68D5CE4, 0.17810359},
  /* 50 */ {5, 0x12A05F20, 0xB7CDFD9D, 0.17718382},
  /* 51 */ {5, 0x1490AAE3, 0x8E583933, 0.17629143},
  /* 52 */ {5, 0x16A97400, 0x697CC3EA, 0.17542506},
  /* 53 */ {5, 0x18ED2825, 0x48A5CA6C, 0.17458343},
  /* 54 */ {5, 0x1B5E4D60, 0x2B52DB16, 0.17376534},
  /* 55 */ {5, 0x1DFF8297, 0x111586A6, 0.17296969},
  /* 56 */ {5, 0x20D38000, 0xF31D2B36, 0.17219543},
  /* 57 */ {5, 0x23DD1799, 0xC8D76D19, 0.17144160},
  /* 58 */ {5, 0x271F35A0, 0xA2CB1EB4, 0.17070728},
  /* 59 */ {5, 0x2A9CE10B, 0x807C3EC3, 0.16999162},
  /* 60 */ {5, 0x2E593C00, 0x617EC8BF, 0.16929381},
  /* 61 */ {5, 0x3257844D, 0x45746CBE, 0.16861310},
  /* 62 */ {5, 0x369B13E0, 0x2C0AA273, 0.16794878},
  /* 63 */ {5, 0x3B27613F, 0x14F90805, 0.16730018},
  /* 64 */ {5, 0x6, 0x0, 0.16666667},
  /* 65 */ {5, 0x4528A141, 0xD9CF0829, 0.16604765},
  /* 66 */ {5, 0x4AA51420, 0xB6FC4841, 0.16544255},
  /* 67 */ {5, 0x50794633, 0x973054CB, 0.16485086},
  /* 68 */ {5, 0x56A94400, 0x7A1DBE4B, 0.16427205},
  /* 69 */ {5, 0x5D393975, 0x5F7FCD7F, 0.16370566},
  /* 70 */ {5, 0x642D7260, 0x47196C84, 0.16315122},
  /* 71 */ {5, 0x6B8A5AE7, 0x30B43635, 0.16260831},
  /* 72 */ {5, 0x73548000, 0x1C1FA5F6, 0.16207652},
  /* 73 */ {5, 0x7B908FE9, 0x930634A, 0.16155547},
  /* 74 */ {5, 0x84435AA0, 0xEF7F4A3C, 0.16104477},
  /* 75 */ {5, 0x8D71D25B, 0xCF5552D2, 0.16054409},
  /* 76 */ {5, 0x97210C00, 0xB1A47C8E, 0.16005307},
  /* 77 */ {5, 0xA1563F9D, 0x9634B43E, 0.15957142},
  /* 78 */ {5, 0xAC16C8E0, 0x7CD3817D, 0.15909881},
  /* 79 */ {5, 0xB768278F, 0x65536761, 0.15863496},
  /* 80 */ {5, 0xC3500000, 0x4F8B588E, 0.15817959},
  /* 81 */ {5, 0xCFD41B91, 0x3B563C24, 0.15773244},
  /* 82 */ {5, 0xDCFA6920, 0x28928154, 0.15729325},
  /* 83 */ {5, 0xEAC8FD83, 0x1721BFB0, 0.15686177},
  /* 84 */ {5, 0xF9461400, 0x6E8629D, 0.15643779},
  /* 85 */ {4, 0x31C84B1, 0x491CC17C, 0.15602107},
  /* 86 */ {4, 0x342AB10, 0x3A11D83B, 0.15561139},
  /* 87 */ {4, 0x36A2C21, 0x2BE074CD, 0.15520856},
  /* 88 */ {4, 0x3931000, 0x1E7A02E7, 0.15481238},
  /* 89 */ {4, 0x3BD5EE1, 0x11D10EDD, 0.15442266},
  /* 90 */ {4, 0x3E92110, 0x5D92C68, 0.15403922},
  /* 91 */ {4, 0x4165EF1, 0xF50DBFB2, 0.15366189},
  /* 92 */ {4, 0x4452100, 0xDF9F1316, 0.15329049},
  /* 93 */ {4, 0x4756FD1, 0xCB52A684, 0.15292487},
  /* 94 */ {4, 0x4A75410, 0xB8163E97, 0.15256487},
  /* 95 */ {4, 0x4DAD681, 0xA5D8F269, 0.15221035},
  /* 96 */ {4, 0x5100000, 0x948B0FCD, 0.15186115},
  /* 97 */ {4, 0x546D981, 0x841E0215, 0.15151715},
  /* 98 */ {4, 0x57F6C10, 0x74843B1E, 0.15117821},
  /* 99 */ {4, 0x5B9C0D1, 0x65B11E6E, 0.15084420},
  /* 100 */ {4, 0x5F5E100, 0x5798EE23, 0.15051500},
  /* 101 */ {4, 0x633D5F1, 0x4A30B99B, 0.15019048},
  /* 102 */ {4, 0x673A910, 0x3D6E4D94, 0.14987054},
  /* 103 */ {4, 0x6B563E1, 0x314825B0, 0.14955506},
  /* 104 */ {4, 0x6F91000, 0x25B55F2E, 0.14924394},
  /* 105 */ {4, 0x73EB721, 0x1AADACCB, 0.14893706},
  /* 106 */ {4, 0x7866310, 0x10294BA2, 0.14863434},
  /* 107 */ {4, 0x7D01DB1, 0x620F8F6, 0.14833567},
  /* 108 */ {4, 0x81BF100, 0xF91BD1B6, 0.14804096},
  /* 109 */ {4, 0x869E711, 0xE6D37B2A, 0.14775011},
  /* 110 */ {4, 0x8BA0A10, 0xD55CFF6E, 0.14746305},
  /* 111 */ {4, 0x90C6441, 0xC4AD2DB2, 0.14717969},
  /* 112 */ {4, 0x9610000, 0xB4B985CF, 0.14689994},
  /* 113 */ {4, 0x9B7E7C1, 0xA5782BEF, 0.14662372},
  /* 114 */ {4, 0xA112610, 0x96DFDD2A, 0.14635096},
  /* 115 */ {4, 0xA6CC591, 0x88E7E509, 0.14608158},
  /* 116 */ {4, 0xACAD100, 0x7B8813D3, 0.14581551},
  /* 117 */ {4, 0xB2B5331, 0x6EB8B595, 0.14555268},
  /* 118 */ {4, 0xB8E5710, 0x627289DB, 0.14529302},
  /* 119 */ {4, 0xBF3E7A1, 0x56AEBC07, 0.14503647},
  /* 120 */ {4, 0xC5C1000, 0x4B66DC33, 0.14478295},
  /* 121 */ {4, 0xCC6DB61, 0x4094D8A3, 0.14453241},
  /* 122 */ {4, 0xD345510, 0x3632F7A5, 0.14428479},
  /* 123 */ {4, 0xDA48871, 0x2C3BD1F0, 0.14404003},
  /* 124 */ {4, 0xE178100, 0x22AA4D5F, 0.14379807},
  /* 125 */ {4, 0xE8D4A51, 0x19799812, 0.14355885},
  /* 126 */ {4, 0xF05F010, 0x10A523E5, 0.14332233},
  /* 127 */ {4, 0xF817E01, 0x828A237, 0.14308844},
  /* 128 */ {4, 0x7, 0x0, 0.14285714},
  /* 129 */ {4, 0x10818201, 0xF04EC452, 0.14262838},
  /* 130 */ {4, 0x11061010, 0xE136444A, 0.14240211},
  /* 131 */ {4, 0x118DB651, 0xD2AF9589, 0.14217828},
  /* 132 */ {4, 0x12188100, 0xC4B42A83, 0.14195685},
  /* 133 */ {4, 0x12A67C71, 0xB73DCCF5, 0.14173777},
  /* 134 */ {4, 0x1337B510, 0xAA4698C5, 0.14152100},
  /* 135 */ {4, 0x13CC3761, 0x9DC8F729, 0.14130649},
  /* 136 */ {4, 0x14641000, 0x91BF9A30, 0.14109421},
  /* 137 */ {4, 0x14FF4BA1, 0x86257887, 0.14088412},
  /* 138 */ {4, 0x159DF710, 0x7AF5C98C, 0.14067617},
  /* 139 */ {4, 0x16401F31, 0x702C01A0, 0.14047033},
  /* 140 */ {4, 0x16E5D100, 0x65C3CEB1, 0.14026656},
  /* 141 */ {4, 0x178F1991, 0x5BB91502, 0.14006482},
  /* 142 */ {4, 0x183C0610, 0x5207EC23, 0.13986509},
  /* 143 */ {4, 0x18ECA3C1, 0x48AC9C19, 0.13966731},
  /* 144 */ {4, 0x19A10000, 0x3FA39AB5, 0.13947147},
  /* 145 */ {4, 0x1A592841, 0x36E98912, 0.13927753},
  /* 146 */ {4, 0x1B152A10, 0x2E7B3140, 0.13908545},
  /* 147 */ {4, 0x1BD51311, 0x2655840B, 0.13889521},
  /* 148 */ {4, 0x1C98F100, 0x1E7596EA, 0.13870677},
  /* 149 */ {4, 0x1D60D1B1, 0x16D8A20D, 0.13852011},
  /* 150 */ {4, 0x1E2CC310, 0xF7BFE87, 0.13833519},
  /* 151 */ {4, 0x1EFCD321, 0x85D2492, 0.13815199},
  /* 152 */ {4, 0x1FD11000, 0x179A9F4, 0.13797047},
  /* 153 */ {4, 0x20A987E1, 0xF59E80EB, 0.13779062},
  /* 154 */ {4, 0x21864910, 0xE8B768DB, 0.13761241},
  /* 155 */ {4, 0x226761F1, 0xDC39D6D5, 0.13743580},
  /* 156 */ {4, 0x234CE100, 0xD021C5D1, 0.13726078},
  /* 157 */ {4, 0x2436D4D1, 0xC46B5E37, 0.13708732},
  /* 158 */ {4, 0x25254C10, 0xB912F39C, 0.13691539},
  /* 159 */ {4, 0x26185581, 0xAE150294, 0.13674498},
  /* 160 */ {4, 0x27100000, 0xA36E2EB1, 0.13657605},
  /* 161 */ {4, 0x280C5A81, 0x991B4094, 0.13640859},
  /* 162 */ {4, 0x290D7410, 0x8F19241E, 0.13624257},
  /* 163 */ {4, 0x2A135BD1, 0x8564E6B7, 0.13607797},
  /* 164 */ {4, 0x2B1E2100, 0x7BFBB5B4, 0.13591477},
  /* 165 */ {4, 0x2C2DD2F1, 0x72DADCC8, 0.13575295},
  /* 166 */ {4, 0x2D428110, 0x69FFC498, 0.13559250},
  /* 167 */ {4, 0x2E5C3AE1, 0x6167F154, 0.13543338},
  /* 168 */ {4, 0x2F7B1000, 0x5911016E, 0.13527558},
  /* 169 */ {4, 0x309F1021, 0x50F8AC5F, 0.13511908},
  /* 170 */ {4, 0x31C84B10, 0x491CC17C, 0.13496386},
  /* 171 */ {4, 0x32F6D0B1, 0x417B26D8, 0.13480991},
  /* 172 */ {4, 0x342AB100, 0x3A11D83B, 0.13465720},
  /* 173 */ {4, 0x3563FC11, 0x32DEE622, 0.13450572},
  /* 174 */ {4, 0x36A2C210, 0x2BE074CD, 0.13435545},
  /* 175 */ {4, 0x37E71341, 0x2514BB58, 0.13420637},
  /* 176 */ {4, 0x39310000, 0x1E7A02E7, 0.13405847},
  /* 177 */ {4, 0x3A8098C1, 0x180EA5D0, 0.13391173},
  /* 178 */ {4, 0x3BD5EE10, 0x11D10EDD, 0.13376614},
  /* 179 */ {4, 0x3D311091, 0xBBFB88E, 0.13362168},
  /* 180 */ {4, 0x3E921100, 0x5D92C68, 0.13347832},
  /* 181 */ {4, 0x3FF90031, 0x1C024C, 0.13333607},
  /* 182 */ {4, 0x4165EF10, 0xF50DBFB2, 0.13319491},
  /* 183 */ {4, 0x42D8EEA1, 0xEA30EFA3, 0.13305481},
  /* 184 */ {4, 0x44521000, 0xDF9F1316, 0.13291577},
  /* 185 */ {4, 0x45D16461, 0xD555C0C9, 0.13277777},
  /* 186 */ {4, 0x4756FD10, 0xCB52A684, 0.13264079},
  /* 187 */ {4, 0x48E2EB71, 0xC193881F, 0.13250483},
  /* 188 */ {4, 0x4A754100, 0xB8163E97, 0.13236988},
  /* 189 */ {4, 0x4C0E0F51, 0xAED8B724, 0.13223591},
  /* 190 */ {4, 0x4DAD6810, 0xA5D8F269, 0.13210292},
  /* 191 */ {4, 0x4F535D01, 0x9D15039D, 0.13197089},
  /* 192 */ {4, 0x51000000, 0x948B0FCD, 0.13183981},
  /* 193 */ {4, 0x52B36301, 0x8C394D1D, 0.13170967},
  /* 194 */ {4, 0x546D9810, 0x841E0215, 0.13158046},
  /* 195 */ {4, 0x562EB151, 0x7C3784F8, 0.13145216},
  /* 196 */ {4, 0x57F6C100, 0x74843B1E, 0.13132477},
  /* 197 */ {4, 0x59C5D971, 0x6D02985D, 0.13119827},
  /* 198 */ {4, 0x5B9C0D10, 0x65B11E6E, 0.13107265},
  /* 199 */ {4, 0x5D796E61, 0x5E8E5C64, 0.13094791},
  /* 200 */ {4, 0x5F5E1000, 0x5798EE23, 0.13082402},
  /* 201 */ {4, 0x614A04A1, 0x50CF7BDE, 0.13070099},
  /* 202 */ {4, 0x633D5F10, 0x4A30B99B, 0.13057879},
  /* 203 */ {4, 0x65383231, 0x43BB66BD, 0.13045743},
  /* 204 */ {4, 0x673A9100, 0x3D6E4D94, 0.13033688},
  /* 205 */ {4, 0x69448E91, 0x374842EE, 0.13021715},
  /* 206 */ {4, 0x6B563E10, 0x314825B0, 0.13009822},
  /* 207 */ {4, 0x6D6FB2C1, 0x2B6CDE75, 0.12998007},
  /* 208 */ {4, 0x6F910000, 0x25B55F2E, 0.12986271},
  /* 209 */ {4, 0x71BA3941, 0x2020A2C5, 0.12974613},
  /* 210 */ {4, 0x73EB7210, 0x1AADACCB, 0.12963031},
  /* 211 */ {4, 0x7624BE11, 0x155B891F, 0.12951524},
  /* 212 */ {4, 0x78663100, 0x10294BA2, 0.12940092},
  /* 213 */ {4, 0x7AAFDEB1, 0xB160FE9, 0.12928734},
  /* 214 */ {4, 0x7D01DB10, 0x620F8F6, 0.12917448},
  /* 215 */ {4, 0x7F5C3A21, 0x14930EF, 0.12906235},
  /* 216 */ {4, 0x81BF1000, 0xF91BD1B6, 0.12895094},
  /* 217 */ {4, 0x842A70E1, 0xEFDCB0C7, 0.12884022},
  /* 218 */ {4, 0x869E7110, 0xE6D37B2A, 0.12873021},
  /* 219 */ {4, 0x891B24F1, 0xDDFEB94A, 0.12862089},
  /* 220 */ {4, 0x8BA0A100, 0xD55CFF6E, 0.12851224},
  /* 221 */ {4, 0x8E2EF9D1, 0xCCECED50, 0.12840428},
  /* 222 */ {4, 0x90C64410, 0xC4AD2DB2, 0.12829698},
  /* 223 */ {4, 0x93669481, 0xBC9C75F9, 0.12819034},
  /* 224 */ {4, 0x96100000, 0xB4B985CF, 0.12808435},
  /* 225 */ {4, 0x98C29B81, 0xAD0326C2, 0.12797901},
  /* 226 */ {4, 0x9B7E7C10, 0xA5782BEF, 0.12787431},
  /* 227 */ {4, 0x9E43B6D1, 0x9E1771A9, 0.12777024},
  /* 228 */ {4, 0xA1126100, 0x96DFDD2A, 0.12766680},
  /* 229 */ {4, 0xA3EA8FF1, 0x8FD05C41, 0.12756398},
  /* 230 */ {4, 0xA6CC5910, 0x88E7E509, 0.12746176},
  /* 231 */ {4, 0xA9B7D1E1, 0x8225759D, 0.12736016},
  /* 232 */ {4, 0xACAD1000, 0x7B8813D3, 0.12725915},
  /* 233 */ {4, 0xAFAC2921, 0x750ECCF9, 0.12715874},
  /* 234 */ {4, 0xB2B53310, 0x6EB8B595, 0.12705891},
  /* 235 */ {4, 0xB5C843B1, 0x6884E923, 0.12695967},
  /* 236 */ {4, 0xB8E57100, 0x627289DB, 0.12686100},
  /* 237 */ {4, 0xBC0CD111, 0x5C80C07B, 0.12676290},
  /* 238 */ {4, 0xBF3E7A10, 0x56AEBC07, 0.12666537},
  /* 239 */ {4, 0xC27A8241, 0x50FBB19B, 0.12656839},
  /* 240 */ {4, 0xC5C10000, 0x4B66DC33, 0.12647197},
  /* 241 */ {4, 0xC91209C1, 0x45EF7C7C, 0.12637609},
  /* 242 */ {4, 0xCC6DB610, 0x4094D8A3, 0.12628075},
  /* 243 */ {4, 0xCFD41B91, 0x3B563C24, 0.12618595},
  /* 244 */ {4, 0xD3455100, 0x3632F7A5, 0.12609168},
  /* 245 */ {4, 0xD6C16D31, 0x312A60C3, 0.12599794},
  /* 246 */ {4, 0xDA488710, 0x2C3BD1F0, 0.12590471},
  /* 247 */ {4, 0xDDDAB5A1, 0x2766AA45, 0.12581200},
  /* 248 */ {4, 0xE1781000, 0x22AA4D5F, 0.12571980},
  /* 249 */ {4, 0xE520AD61, 0x1E06233C, 0.12562811},
  /* 250 */ {4, 0xE8D4A510, 0x19799812, 0.12553692},
  /* 251 */ {4, 0xEC940E71, 0x15041C33, 0.12544622},
  /* 252 */ {4, 0xF05F0100, 0x10A523E5, 0.12535601},
  /* 253 */ {4, 0xF4359451, 0xC5C2749, 0.12526629},
  /* 254 */ {4, 0xF817E010, 0x828A237, 0.12517705},
  /* 255 */ {4, 0xFC05FC01, 0x40A1423, 0.12508829},
};