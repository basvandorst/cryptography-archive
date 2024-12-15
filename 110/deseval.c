/*
 * Generated S-box files.
 *
 * Produced by Matthew Kwan - April 1997
 */

static void
s1 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86, x87, x88;
	unsigned long	x89, x90, x91, x92, x93, x94, x95;

	x1 = ~a2;
	x2 = ~a6;
	x5 = x2 & x1;
	x6 = a5 ^ x5;
	x4 = a1 & x6;
	x7 = x1 ^ a6;
	x8 = a5 ^ x7;
	x9 = x4 ^ x8;
	x3 = a3 & x9;
	x11 = a5 & a2;
	x12 = x11 ^ x5;
	x10 = a1 & x12;
	x13 = a5 & a6;
	x14 = x10 ^ x13;
	x15 = x3 ^ x14;
	x16 = a4 & x15;
	x20 = x2 | x1;
	x19 = a5 & x20;
	x21 = x19 ^ a2;
	x18 = a1 & x21;
	x22 = x18 ^ x1;
	x17 = a3 & x22;
	x24 = ~x19;
	x23 = a1 & x24;
	x25 = x23 ^ x8;
	x26 = x17 ^ x25;
	x27 = x16 ^ x26;
	*out1 ^= x27;
	x30 = x1 | a6;
	x31 = x13 ^ x30;
	x29 = a1 & x31;
	x32 = x13 ^ a6;
	x33 = x29 ^ x32;
	x28 = a3 & x33;
	x34 = a1 & x19;
	x35 = a5 & x1;
	x36 = a2 | a6;
	x37 = x35 ^ x36;
	x38 = x34 ^ x37;
	x39 = x28 ^ x38;
	x40 = a4 & x39;
	x43 = a5 & x36;
	x44 = x43 ^ x30;
	x42 = a1 & x44;
	x45 = a5 & x2;
	x46 = x45 ^ x20;
	x47 = x42 ^ x46;
	x41 = a3 & x47;
	x48 = a1 & x37;
	x49 = x13 ^ x5;
	x50 = x48 ^ x49;
	x51 = x41 ^ x50;
	x52 = x40 ^ x51;
	*out2 ^= x52;
	x54 = x29 ^ x5;
	x53 = a3 & x54;
	x57 = x2 & a2;
	x56 = a5 & x57;
	x58 = x56 ^ x57;
	x55 = a1 & x58;
	x60 = x2 | a2;
	x59 = a5 & x60;
	x61 = x59 ^ x30;
	x62 = x55 ^ x61;
	x63 = x53 ^ x62;
	x64 = a4 & x63;
	x66 = x55 ^ x37;
	x65 = a3 & x66;
	x67 = a1 & x61;
	x68 = x35 ^ x60;
	x69 = x67 ^ x68;
	x70 = x65 ^ x69;
	x71 = x64 ^ x70;
	*out3 ^= x71;
	x74 = x45 ^ x5;
	x73 = a1 & x74;
	x72 = a3 & x73;
	x76 = x35 ^ x2;
	x75 = a1 & x76;
	x77 = x11 ^ x20;
	x78 = x75 ^ x77;
	x79 = x72 ^ x78;
	x80 = a4 & x79;
	x83 = x19 ^ x1;
	x82 = a1 & x83;
	x84 = a2 & a6;
	x85 = x19 ^ x84;
	x86 = x82 ^ x85;
	x81 = a3 & x86;
	x88 = a5 & x5;
	x89 = x88 ^ a6;
	x87 = a1 & x89;
	x91 = a2 ^ a6;
	x90 = a5 & x91;
	x92 = x90 ^ x57;
	x93 = x87 ^ x92;
	x94 = x81 ^ x93;
	x95 = x80 ^ x94;
	*out4 ^= x95;
}


static void
s2 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84;

	x1 = ~a5;
	x2 = ~a6;
	x5 = x2 & a5;
	x4 = a2 & x5;
	x6 = x4 ^ x5;
	x3 = a1 & x6;
	x7 = a2 & x1;
	x8 = x7 ^ a5;
	x9 = x3 ^ x8;
	x10 = a4 & x9;
	x13 = a2 & x2;
	x14 = a5 & a6;
	x15 = x13 ^ x14;
	x12 = a1 & x15;
	x16 = ~x13;
	x17 = x12 ^ x16;
	x11 = a3 & x17;
	x20 = x1 & a6;
	x19 = a2 & x20;
	x21 = x2 | x1;
	x22 = x19 ^ x21;
	x18 = a1 & x22;
	x23 = a2 & a6;
	x24 = x1 ^ a6;
	x25 = x23 ^ x24;
	x26 = x18 ^ x25;
	x27 = x11 ^ x26;
	x28 = x10 ^ x27;
	*out1 ^= x28;
	x29 = a3 & x14;
	x30 = a1 & x4;
	x31 = x13 ^ x21;
	x32 = x30 ^ x31;
	x33 = x29 ^ x32;
	x34 = a4 & x33;
	x36 = a2 ^ a6;
	x37 = x30 ^ x36;
	x35 = a3 & x37;
	x38 = a2 ^ x24;
	x39 = a1 ^ x38;
	x40 = x35 ^ x39;
	x41 = x34 ^ x40;
	*out2 ^= x41;
	x43 = a1 & x38;
	x44 = x43 ^ x24;
	x42 = a3 & x44;
	x47 = x2 | a5;
	x46 = a2 & x47;
	x48 = x46 ^ a5;
	x45 = a1 & x48;
	x49 = ~x19;
	x50 = x45 ^ x49;
	x51 = x42 ^ x50;
	x52 = a4 & x51;
	x56 = x1 | a6;
	x55 = a2 & x56;
	x57 = x55 ^ x1;
	x54 = a1 & x57;
	x58 = x23 ^ a5;
	x59 = x54 ^ x58;
	x53 = a3 & x59;
	x61 = a2 & x24;
	x62 = x61 ^ x21;
	x60 = a1 & x62;
	x63 = a2 & x21;
	x64 = x63 ^ x1;
	x65 = x60 ^ x64;
	x66 = x53 ^ x65;
	x67 = x52 ^ x66;
	*out3 ^= x67;
	x69 = x23 ^ x14;
	x68 = a1 & x69;
	x70 = x4 ^ x21;
	x71 = x68 ^ x70;
	x72 = a4 & x71;
	x75 = x2 & x1;
	x76 = x19 ^ x75;
	x74 = a1 & x76;
	x77 = x4 ^ x24;
	x78 = x74 ^ x77;
	x73 = a3 & x78;
	x80 = x55 ^ x47;
	x79 = a1 & x80;
	x81 = ~x23;
	x82 = x79 ^ x81;
	x83 = x73 ^ x82;
	x84 = x72 ^ x83;
	*out4 ^= x84;
}


static void
s3 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86, x87, x88;
	unsigned long	x89;

	x1 = ~a1;
	x2 = ~a2;
	x5 = a6 & x1;
	x4 = a5 & x5;
	x6 = x2 & x1;
	x7 = x4 ^ x6;
	x3 = a3 & x7;
	x9 = a6 & x6;
	x10 = x9 ^ x6;
	x8 = a5 & x10;
	x11 = a1 | a2;
	x12 = x5 ^ x11;
	x13 = x8 ^ x12;
	x14 = x3 ^ x13;
	x15 = a4 & x14;
	x19 = a1 ^ a2;
	x18 = a6 & x19;
	x20 = x18 ^ x2;
	x17 = a5 & x20;
	x21 = x1 | a2;
	x22 = x17 ^ x21;
	x16 = a3 & x22;
	x23 = a6 & a1;
	x24 = x2 | a1;
	x25 = x23 ^ x24;
	x26 = a5 ^ x25;
	x27 = x16 ^ x26;
	x28 = x15 ^ x27;
	*out1 ^= x28;
	x30 = a5 & x23;
	x31 = x1 & a2;
	x32 = x30 ^ x31;
	x29 = a3 & x32;
	x34 = ~x5;
	x33 = a5 & x34;
	x35 = a6 & x2;
	x36 = x35 ^ x31;
	x37 = x33 ^ x36;
	x38 = x29 ^ x37;
	x39 = a4 & x38;
	x42 = a6 & x31;
	x43 = x42 ^ x24;
	x41 = a5 & x43;
	x44 = x41 ^ x43;
	x40 = a3 & x44;
	x46 = x42 ^ x31;
	x45 = a5 & x46;
	x47 = a6 & x24;
	x48 = x2 & a1;
	x49 = x47 ^ x48;
	x50 = x45 ^ x49;
	x51 = x40 ^ x50;
	x52 = x39 ^ x51;
	*out2 ^= x52;
	x54 = x9 ^ x24;
	x55 = x33 ^ x54;
	x53 = a3 & x55;
	x57 = a6 & x21;
	x58 = x57 ^ x19;
	x56 = a5 & x58;
	x59 = x57 ^ x6;
	x60 = x56 ^ x59;
	x61 = x53 ^ x60;
	x62 = a4 & x61;
	x65 = x5 ^ x1;
	x64 = a5 & x65;
	x66 = x35 ^ a2;
	x67 = x64 ^ x66;
	x63 = a3 & x67;
	x68 = x1 ^ a2;
	x69 = x57 ^ x68;
	x70 = x41 ^ x69;
	x71 = x63 ^ x70;
	x72 = x62 ^ x71;
	*out3 ^= x72;
	x75 = a1 & a2;
	x74 = a6 & x75;
	x73 = a3 & x74;
	x76 = a5 & x1;
	x77 = ~x23;
	x78 = x76 ^ x77;
	x79 = x73 ^ x78;
	x80 = a4 & x79;
	x82 = a5 & x21;
	x83 = x74 ^ x48;
	x84 = x82 ^ x83;
	x81 = a3 & x84;
	x85 = a5 & x83;
	x86 = x57 ^ x11;
	x87 = x85 ^ x86;
	x88 = x81 ^ x87;
	x89 = x80 ^ x88;
	*out4 ^= x89;
}


static void
s4 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77;

	x1 = ~a3;
	x2 = ~a5;
	x5 = a3 ^ a5;
	x4 = a2 & x5;
	x7 = x1 | a5;
	x6 = a1 & x7;
	x8 = x6 ^ x2;
	x9 = x4 ^ x8;
	x3 = a4 & x9;
	x12 = x1 & a5;
	x11 = a1 & x12;
	x13 = x2 | a3;
	x14 = x11 ^ x13;
	x10 = a2 & x14;
	x15 = x1 ^ a5;
	x16 = x11 ^ x15;
	x17 = x10 ^ x16;
	x18 = x3 ^ x17;
	x19 = a6 & x18;
	x22 = a1 & x15;
	x23 = x22 ^ a5;
	x21 = a2 & x23;
	x24 = ~x6;
	x25 = x21 ^ x24;
	x20 = a4 & x25;
	x27 = a3 | a5;
	x28 = x11 ^ x27;
	x26 = a2 & x28;
	x29 = a1 ^ x12;
	x30 = x26 ^ x29;
	x31 = x20 ^ x30;
	x32 = x19 ^ x31;
	*out1 ^= x32;
	x33 = x11 ^ x5;
	x34 = x10 ^ x33;
	x35 = x3 ^ x34;
	x36 = a6 & x35;
	x39 = x22 ^ a3;
	x38 = a2 & x39;
	x40 = x38 ^ a5;
	x37 = a4 & x40;
	x41 = a2 & x1;
	x42 = a1 & x13;
	x43 = x42 ^ x7;
	x44 = x41 ^ x43;
	x45 = x37 ^ x44;
	x46 = x36 ^ x45;
	*out2 ^= x46;
	x48 = x2 | x1;
	x49 = x42 ^ x48;
	x50 = x4 ^ x49;
	x47 = a4 & x50;
	x53 = x2 & a3;
	x52 = a1 & x53;
	x54 = x52 ^ x15;
	x51 = a2 & x54;
	x55 = x22 ^ x2;
	x56 = x51 ^ x55;
	x57 = x47 ^ x56;
	x58 = a6 & x57;
	x60 = x42 ^ x12;
	x61 = x38 ^ x60;
	x59 = a4 & x61;
	x63 = ~x42;
	x62 = a2 & x63;
	x64 = x62 ^ x16;
	x65 = x59 ^ x64;
	x66 = x58 ^ x65;
	*out3 ^= x66;
	x67 = x51 ^ x23;
	x68 = x47 ^ x67;
	x69 = a6 & x68;
	x71 = x21 ^ x2;
	x70 = a4 & x71;
	x73 = x22 ^ x5;
	x72 = a2 & x73;
	x74 = x6 ^ x1;
	x75 = x72 ^ x74;
	x76 = x70 ^ x75;
	x77 = x69 ^ x76;
	*out4 ^= x77;
}


static void
s5 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86, x87, x88;
	unsigned long	x89, x90, x91, x92, x93, x94, x95, x96;

	x1 = ~a3;
	x2 = ~a6;
	x5 = a4 & x2;
	x6 = x5 ^ a6;
	x4 = a1 & x6;
	x7 = a3 & a6;
	x8 = a4 ^ x7;
	x9 = x4 ^ x8;
	x3 = a2 & x9;
	x11 = a4 & a3;
	x12 = x2 | a3;
	x13 = x11 ^ x12;
	x10 = a1 & x13;
	x15 = x1 | a6;
	x14 = a4 & x15;
	x16 = x14 ^ x2;
	x17 = x10 ^ x16;
	x18 = x3 ^ x17;
	x19 = a5 & x18;
	x23 = x1 ^ a6;
	x22 = a4 & x23;
	x24 = x22 ^ x7;
	x21 = a1 & x24;
	x25 = x2 | x1;
	x26 = x5 ^ x25;
	x27 = x21 ^ x26;
	x20 = a2 & x27;
	x29 = a4 & a6;
	x30 = x2 & a3;
	x31 = x29 ^ x30;
	x28 = a1 & x31;
	x33 = a3 | a6;
	x32 = a4 & x33;
	x34 = x1 & a6;
	x35 = x32 ^ x34;
	x36 = x28 ^ x35;
	x37 = x20 ^ x36;
	x38 = x19 ^ x37;
	*out1 ^= x38;
	x40 = x22 ^ a6;
	x39 = a1 & x40;
	x41 = a4 & x7;
	x42 = x41 ^ x25;
	x43 = x39 ^ x42;
	x44 = a5 & x43;
	x47 = x32 ^ x33;
	x46 = a1 & x47;
	x48 = a4 & x25;
	x49 = x48 ^ x7;
	x50 = x46 ^ x49;
	x45 = a2 & x50;
	x51 = x48 ^ x33;
	x52 = a1 ^ x51;
	x53 = x45 ^ x52;
	x54 = x44 ^ x53;
	*out2 ^= x54;
	x57 = x29 ^ x25;
	x56 = a1 & x57;
	x58 = x2 & x1;
	x59 = a4 ^ x58;
	x60 = x56 ^ x59;
	x55 = a2 & x60;
	x62 = a3 ^ a6;
	x63 = x14 ^ x62;
	x61 = a1 & x63;
	x64 = x14 ^ x23;
	x65 = x61 ^ x64;
	x66 = x55 ^ x65;
	x67 = a5 & x66;
	x69 = ~x32;
	x70 = x61 ^ x69;
	x68 = a2 & x70;
	x72 = x14 ^ x58;
	x71 = a1 & x72;
	x73 = a4 & x58;
	x74 = x73 ^ x25;
	x75 = x71 ^ x74;
	x76 = x68 ^ x75;
	x77 = x67 ^ x76;
	*out3 ^= x77;
	x79 = a1 & x59;
	x80 = x29 ^ x23;
	x81 = x79 ^ x80;
	x78 = a2 & x81;
	x82 = a1 & x14;
	x83 = x14 ^ x33;
	x84 = x82 ^ x83;
	x85 = x78 ^ x84;
	x86 = a5 & x85;
	x89 = a4 & x1;
	x90 = x89 ^ x58;
	x88 = a1 & x90;
	x91 = x88 ^ x6;
	x87 = a2 & x91;
	x92 = a1 & x51;
	x93 = x41 ^ x30;
	x94 = x92 ^ x93;
	x95 = x87 ^ x94;
	x96 = x86 ^ x95;
	*out4 ^= x96;
}


static void
s6 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86, x87;

	x1 = ~a2;
	x2 = ~a3;
	x5 = a5 & x1;
	x6 = x1 | a3;
	x7 = x5 ^ x6;
	x4 = a4 & x7;
	x9 = x2 | a2;
	x8 = a5 & x9;
	x10 = x8 ^ x9;
	x11 = x4 ^ x10;
	x3 = a1 & x11;
	x13 = a5 & x2;
	x14 = x13 ^ x9;
	x12 = a4 & x14;
	x15 = x13 ^ a3;
	x16 = x12 ^ x15;
	x17 = x3 ^ x16;
	x18 = a6 & x17;
	x19 = a1 & x15;
	x20 = a4 & x15;
	x21 = a5 ^ x6;
	x22 = x20 ^ x21;
	x23 = x19 ^ x22;
	x24 = x18 ^ x23;
	*out1 ^= x24;
	x27 = a2 & a3;
	x28 = a5 ^ x27;
	x26 = a4 & x28;
	x30 = x1 & a3;
	x29 = a5 & x30;
	x31 = x29 ^ x30;
	x32 = x26 ^ x31;
	x25 = a1 & x32;
	x34 = a5 & a2;
	x33 = a4 & x34;
	x35 = ~x33;
	x36 = x25 ^ x35;
	x37 = a6 & x36;
	x41 = x1 ^ a3;
	x40 = a5 & x41;
	x39 = a4 & x40;
	x42 = a5 & x27;
	x43 = x42 ^ x9;
	x44 = x39 ^ x43;
	x38 = a1 & x44;
	x46 = a5 & a3;
	x47 = x46 ^ x1;
	x45 = a4 & x47;
	x48 = x13 ^ x41;
	x49 = x45 ^ x48;
	x50 = x38 ^ x49;
	x51 = x37 ^ x50;
	*out2 ^= x51;
	x53 = a4 & x5;
	x54 = x29 ^ x2;
	x55 = x53 ^ x54;
	x52 = a1 & x55;
	x56 = a4 & a5;
	x57 = ~x34;
	x58 = x56 ^ x57;
	x59 = x52 ^ x58;
	x60 = a6 & x59;
	x62 = a2 | a3;
	x63 = x13 ^ x62;
	x64 = x33 ^ x63;
	x61 = a1 & x64;
	x65 = a4 & x57;
	x66 = x29 ^ x27;
	x67 = x65 ^ x66;
	x68 = x61 ^ x67;
	x69 = x60 ^ x68;
	*out3 ^= x69;
	x72 = a2 ^ a3;
	x73 = x40 ^ x72;
	x71 = a4 & x73;
	x74 = x71 ^ x6;
	x70 = a1 & x74;
	x76 = x40 ^ x30;
	x75 = a4 & x76;
	x77 = x70 ^ x75;
	x78 = a6 & x77;
	x80 = ~x46;
	x81 = x20 ^ x80;
	x79 = a1 & x81;
	x83 = x46 ^ x62;
	x82 = a4 & x83;
	x84 = a5 ^ x30;
	x85 = x82 ^ x84;
	x86 = x79 ^ x85;
	x87 = x78 ^ x86;
	*out4 ^= x87;
}


static void
s7 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86;

	x1 = ~a1;
	x2 = ~a6;
	x4 = a2 & a1;
	x6 = x2 & x1;
	x5 = a3 & x6;
	x7 = a1 & a6;
	x8 = x5 ^ x7;
	x9 = x4 ^ x8;
	x3 = a5 & x9;
	x11 = x5 ^ x1;
	x10 = a2 & x11;
	x12 = x10 ^ a1;
	x13 = x3 ^ x12;
	x14 = a4 & x13;
	x17 = a3 & a1;
	x16 = a2 & x17;
	x18 = x1 | a6;
	x19 = x17 ^ x18;
	x20 = x16 ^ x19;
	x15 = a5 & x20;
	x22 = x5 ^ a1;
	x21 = a2 & x22;
	x24 = x2 | x1;
	x23 = a3 & x24;
	x25 = x1 & a6;
	x26 = x23 ^ x25;
	x27 = x21 ^ x26;
	x28 = x15 ^ x27;
	x29 = x14 ^ x28;
	*out1 ^= x29;
	x31 = a2 & x25;
	x33 = a1 ^ a6;
	x32 = a3 & x33;
	x34 = x31 ^ x32;
	x30 = a5 & x34;
	x36 = x17 ^ x24;
	x35 = a2 & x36;
	x37 = x35 ^ x1;
	x38 = x30 ^ x37;
	x39 = a4 & x38;
	x41 = x1 ^ a6;
	x42 = x23 ^ x41;
	x40 = a2 & x42;
	x43 = x40 ^ x19;
	x44 = a5 ^ x43;
	x45 = x39 ^ x44;
	*out2 ^= x45;
	x47 = a2 & x41;
	x48 = a3 & x25;
	x49 = x48 ^ x2;
	x50 = x47 ^ x49;
	x46 = a5 & x50;
	x52 = x48 ^ a1;
	x51 = a2 & x52;
	x53 = ~x48;
	x54 = x51 ^ x53;
	x55 = x46 ^ x54;
	x56 = a4 & x55;
	x60 = x2 & a1;
	x59 = a3 & x60;
	x58 = a2 & x59;
	x61 = x59 ^ x6;
	x62 = x58 ^ x61;
	x57 = a5 & x62;
	x64 = ~x59;
	x63 = a2 & x64;
	x65 = a3 & x41;
	x66 = x65 ^ x7;
	x67 = x63 ^ x66;
	x68 = x57 ^ x67;
	x69 = x56 ^ x68;
	*out3 ^= x69;
	x71 = a2 & a6;
	x72 = x23 ^ x24;
	x73 = x71 ^ x72;
	x70 = a5 & x73;
	x74 = x23 ^ x7;
	x75 = x31 ^ x74;
	x76 = x70 ^ x75;
	x77 = a4 & x76;
	x79 = a2 & x7;
	x80 = ~x79;
	x78 = a5 & x80;
	x82 = ~x23;
	x81 = a2 & x82;
	x83 = a3 ^ x33;
	x84 = x81 ^ x83;
	x85 = x78 ^ x84;
	x86 = x77 ^ x85;
	*out4 ^= x86;
}


static void
s8 (
	unsigned long	a1,
	unsigned long	a2,
	unsigned long	a3,
	unsigned long	a4,
	unsigned long	a5,
	unsigned long	a6,
	unsigned long	*out1,
	unsigned long	*out2,
	unsigned long	*out3,
	unsigned long	*out4
) {
	unsigned long	x1, x2, x3, x4, x5, x6, x7, x8;
	unsigned long	x9, x10, x11, x12, x13, x14, x15, x16;
	unsigned long	x17, x18, x19, x20, x21, x22, x23, x24;
	unsigned long	x25, x26, x27, x28, x29, x30, x31, x32;
	unsigned long	x33, x34, x35, x36, x37, x38, x39, x40;
	unsigned long	x41, x42, x43, x44, x45, x46, x47, x48;
	unsigned long	x49, x50, x51, x52, x53, x54, x55, x56;
	unsigned long	x57, x58, x59, x60, x61, x62, x63, x64;
	unsigned long	x65, x66, x67, x68, x69, x70, x71, x72;
	unsigned long	x73, x74, x75, x76, x77, x78, x79, x80;
	unsigned long	x81, x82, x83, x84, x85, x86, x87, x88;

	x1 = ~a3;
	x2 = ~a5;
	x6 = x1 ^ a5;
	x5 = a4 & x6;
	x7 = x2 & a3;
	x8 = x5 ^ x7;
	x4 = a2 & x8;
	x10 = a3 ^ a5;
	x9 = a4 & x10;
	x11 = x9 ^ x6;
	x12 = x4 ^ x11;
	x3 = a6 & x12;
	x14 = a4 & a3;
	x13 = a2 & x14;
	x15 = x2 | x1;
	x16 = x9 ^ x15;
	x17 = x13 ^ x16;
	x18 = x3 ^ x17;
	x19 = a1 & x18;
	x22 = a4 & x1;
	x23 = x22 ^ x2;
	x21 = a2 & x23;
	x24 = ~x5;
	x25 = x21 ^ x24;
	x20 = a6 & x25;
	x27 = x5 ^ a5;
	x26 = a2 & x27;
	x28 = x14 ^ x6;
	x29 = x26 ^ x28;
	x30 = x20 ^ x29;
	x31 = x19 ^ x30;
	*out1 ^= x31;
	x33 = x1 & a5;
	x34 = x22 ^ x33;
	x35 = x13 ^ x34;
	x32 = a6 & x35;
	x37 = x5 ^ x10;
	x36 = a2 & x37;
	x38 = x22 ^ x7;
	x39 = x36 ^ x38;
	x40 = x32 ^ x39;
	x41 = a1 & x40;
	x43 = a4 & x2;
	x44 = x43 ^ x6;
	x42 = a2 & x44;
	x45 = x2 | a3;
	x46 = a4 ^ x45;
	x47 = x42 ^ x46;
	x48 = a6 ^ x47;
	x49 = x41 ^ x48;
	*out2 ^= x49;
	x52 = a4 & a5;
	x53 = a3 & a5;
	x54 = x52 ^ x53;
	x51 = a2 & x54;
	x55 = x43 ^ a5;
	x56 = x51 ^ x55;
	x50 = a6 & x56;
	x57 = a2 & x33;
	x58 = x43 ^ x45;
	x59 = x57 ^ x58;
	x60 = x50 ^ x59;
	x61 = a1 & x60;
	x64 = x5 ^ x6;
	x63 = a2 & x64;
	x62 = a6 & x63;
	x65 = a3 | a5;
	x66 = x52 ^ x65;
	x67 = a2 ^ x66;
	x68 = x62 ^ x67;
	x69 = x61 ^ x68;
	*out3 ^= x69;
	x71 = a2 & x34;
	x72 = x71 ^ x16;
	x70 = a6 & x72;
	x74 = x43 ^ x7;
	x73 = a2 & x74;
	x75 = x73 ^ x65;
	x76 = x70 ^ x75;
	x77 = a1 & x76;
	x79 = a2 & x10;
	x81 = x1 | a5;
	x80 = a4 & x81;
	x82 = x80 ^ x33;
	x83 = x79 ^ x82;
	x78 = a6 & x83;
	x85 = ~x52;
	x84 = a2 & x85;
	x86 = x84 ^ x44;
	x87 = x78 ^ x86;
	x88 = x77 ^ x87;
	*out4 ^= x88;
}


/*
 * Bitslice implementation of DES.
 *
 * Checks that the plaintext bits p[0] .. p[63]
 * encrypt to the ciphertext bits c[0] .. c[63]
 * given the key bits k[0] .. k[55]
 */

unsigned long
deseval (
	const unsigned long	*p,
	const unsigned long	*c,
	const unsigned long	*k
) {
	unsigned long	result = ~0UL;
	unsigned long	l0 = p[6];
	unsigned long	l1 = p[14];
	unsigned long	l2 = p[22];
	unsigned long	l3 = p[30];
	unsigned long	l4 = p[38];
	unsigned long	l5 = p[46];
	unsigned long	l6 = p[54];
	unsigned long	l7 = p[62];
	unsigned long	l8 = p[4];
	unsigned long	l9 = p[12];
	unsigned long	l10 = p[20];
	unsigned long	l11 = p[28];
	unsigned long	l12 = p[36];
	unsigned long	l13 = p[44];
	unsigned long	l14 = p[52];
	unsigned long	l15 = p[60];
	unsigned long	l16 = p[2];
	unsigned long	l17 = p[10];
	unsigned long	l18 = p[18];
	unsigned long	l19 = p[26];
	unsigned long	l20 = p[34];
	unsigned long	l21 = p[42];
	unsigned long	l22 = p[50];
	unsigned long	l23 = p[58];
	unsigned long	l24 = p[0];
	unsigned long	l25 = p[8];
	unsigned long	l26 = p[16];
	unsigned long	l27 = p[24];
	unsigned long	l28 = p[32];
	unsigned long	l29 = p[40];
	unsigned long	l30 = p[48];
	unsigned long	l31 = p[56];
	unsigned long	r0 = p[7];
	unsigned long	r1 = p[15];
	unsigned long	r2 = p[23];
	unsigned long	r3 = p[31];
	unsigned long	r4 = p[39];
	unsigned long	r5 = p[47];
	unsigned long	r6 = p[55];
	unsigned long	r7 = p[63];
	unsigned long	r8 = p[5];
	unsigned long	r9 = p[13];
	unsigned long	r10 = p[21];
	unsigned long	r11 = p[29];
	unsigned long	r12 = p[37];
	unsigned long	r13 = p[45];
	unsigned long	r14 = p[53];
	unsigned long	r15 = p[61];
	unsigned long	r16 = p[3];
	unsigned long	r17 = p[11];
	unsigned long	r18 = p[19];
	unsigned long	r19 = p[27];
	unsigned long	r20 = p[35];
	unsigned long	r21 = p[43];
	unsigned long	r22 = p[51];
	unsigned long	r23 = p[59];
	unsigned long	r24 = p[1];
	unsigned long	r25 = p[9];
	unsigned long	r26 = p[17];
	unsigned long	r27 = p[25];
	unsigned long	r28 = p[33];
	unsigned long	r29 = p[41];
	unsigned long	r30 = p[49];
	unsigned long	r31 = p[57];

	/* Round 1 */
	s1 (r31 ^ k[47], r0 ^ k[11], r1 ^ k[26], r2 ^ k[3], r3 ^ k[13],
		r4 ^ k[41], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[27], r4 ^ k[6], r5 ^ k[54], r6 ^ k[48], r7 ^ k[39],
		r8 ^ k[19], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[53], r8 ^ k[25], r9 ^ k[33], r10 ^ k[34], r11 ^ k[17],
		r12 ^ k[5], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[4], r12 ^ k[55], r13 ^ k[24], r14 ^ k[32], r15 ^ k[40],
		r16 ^ k[20], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[36], r16 ^ k[31], r17 ^ k[21], r18 ^ k[8], r19 ^ k[23],
		r20 ^ k[52], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[14], r20 ^ k[29], r21 ^ k[51], r22 ^ k[9], r23 ^ k[35],
		r24 ^ k[30], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[2], r24 ^ k[37], r25 ^ k[22], r26 ^ k[0], r27 ^ k[42],
		r28 ^ k[38], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[16], r28 ^ k[43], r29 ^ k[44], r30 ^ k[1], r31 ^ k[7],
		r0 ^ k[28], &l4, &l26, &l14, &l20);

	/* Round 2 */
	s1 (l31 ^ k[54], l0 ^ k[18], l1 ^ k[33], l2 ^ k[10], l3 ^ k[20],
		l4 ^ k[48], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[34], l4 ^ k[13], l5 ^ k[4], l6 ^ k[55], l7 ^ k[46],
		l8 ^ k[26], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[3], l8 ^ k[32], l9 ^ k[40], l10 ^ k[41], l11 ^ k[24],
		l12 ^ k[12], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[11], l12 ^ k[5], l13 ^ k[6], l14 ^ k[39], l15 ^ k[47],
		l16 ^ k[27], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[43], l16 ^ k[38], l17 ^ k[28], l18 ^ k[15], l19 ^ k[30],
		l20 ^ k[0], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[21], l20 ^ k[36], l21 ^ k[31], l22 ^ k[16], l23 ^ k[42],
		l24 ^ k[37], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[9], l24 ^ k[44], l25 ^ k[29], l26 ^ k[7], l27 ^ k[49],
		l28 ^ k[45], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[23], l28 ^ k[50], l29 ^ k[51], l30 ^ k[8], l31 ^ k[14],
		l0 ^ k[35], &r4, &r26, &r14, &r20);

	/* Round 3 */
	s1 (r31 ^ k[11], r0 ^ k[32], r1 ^ k[47], r2 ^ k[24], r3 ^ k[34],
		r4 ^ k[5], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[48], r4 ^ k[27], r5 ^ k[18], r6 ^ k[12], r7 ^ k[3],
		r8 ^ k[40], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[17], r8 ^ k[46], r9 ^ k[54], r10 ^ k[55], r11 ^ k[13],
		r12 ^ k[26], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[25], r12 ^ k[19], r13 ^ k[20], r14 ^ k[53], r15 ^ k[4],
		r16 ^ k[41], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[2], r16 ^ k[52], r17 ^ k[42], r18 ^ k[29], r19 ^ k[44],
		r20 ^ k[14], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[35], r20 ^ k[50], r21 ^ k[45], r22 ^ k[30], r23 ^ k[1],
		r24 ^ k[51], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[23], r24 ^ k[31], r25 ^ k[43], r26 ^ k[21], r27 ^ k[8],
		r28 ^ k[0], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[37], r28 ^ k[9], r29 ^ k[38], r30 ^ k[22], r31 ^ k[28],
		r0 ^ k[49], &l4, &l26, &l14, &l20);

	/* Round 4 */
	s1 (l31 ^ k[25], l0 ^ k[46], l1 ^ k[4], l2 ^ k[13], l3 ^ k[48],
		l4 ^ k[19], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[5], l4 ^ k[41], l5 ^ k[32], l6 ^ k[26], l7 ^ k[17],
		l8 ^ k[54], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[6], l8 ^ k[3], l9 ^ k[11], l10 ^ k[12], l11 ^ k[27],
		l12 ^ k[40], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[39], l12 ^ k[33], l13 ^ k[34], l14 ^ k[10], l15 ^ k[18],
		l16 ^ k[55], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[16], l16 ^ k[7], l17 ^ k[1], l18 ^ k[43], l19 ^ k[31],
		l20 ^ k[28], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[49], l20 ^ k[9], l21 ^ k[0], l22 ^ k[44], l23 ^ k[15],
		l24 ^ k[38], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[37], l24 ^ k[45], l25 ^ k[2], l26 ^ k[35], l27 ^ k[22],
		l28 ^ k[14], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[51], l28 ^ k[23], l29 ^ k[52], l30 ^ k[36], l31 ^ k[42],
		l0 ^ k[8], &r4, &r26, &r14, &r20);

	/* Round 5 */
	s1 (r31 ^ k[39], r0 ^ k[3], r1 ^ k[18], r2 ^ k[27], r3 ^ k[5],
		r4 ^ k[33], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[19], r4 ^ k[55], r5 ^ k[46], r6 ^ k[40], r7 ^ k[6],
		r8 ^ k[11], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[20], r8 ^ k[17], r9 ^ k[25], r10 ^ k[26], r11 ^ k[41],
		r12 ^ k[54], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[53], r12 ^ k[47], r13 ^ k[48], r14 ^ k[24], r15 ^ k[32],
		r16 ^ k[12], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[30], r16 ^ k[21], r17 ^ k[15], r18 ^ k[2], r19 ^ k[45],
		r20 ^ k[42], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[8], r20 ^ k[23], r21 ^ k[14], r22 ^ k[31], r23 ^ k[29],
		r24 ^ k[52], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[51], r24 ^ k[0], r25 ^ k[16], r26 ^ k[49], r27 ^ k[36],
		r28 ^ k[28], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[38], r28 ^ k[37], r29 ^ k[7], r30 ^ k[50], r31 ^ k[1],
		r0 ^ k[22], &l4, &l26, &l14, &l20);

	/* Round 6 */
	s1 (l31 ^ k[53], l0 ^ k[17], l1 ^ k[32], l2 ^ k[41], l3 ^ k[19],
		l4 ^ k[47], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[33], l4 ^ k[12], l5 ^ k[3], l6 ^ k[54], l7 ^ k[20],
		l8 ^ k[25], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[34], l8 ^ k[6], l9 ^ k[39], l10 ^ k[40], l11 ^ k[55],
		l12 ^ k[11], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[10], l12 ^ k[4], l13 ^ k[5], l14 ^ k[13], l15 ^ k[46],
		l16 ^ k[26], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[44], l16 ^ k[35], l17 ^ k[29], l18 ^ k[16], l19 ^ k[0],
		l20 ^ k[1], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[22], l20 ^ k[37], l21 ^ k[28], l22 ^ k[45], l23 ^ k[43],
		l24 ^ k[7], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[38], l24 ^ k[14], l25 ^ k[30], l26 ^ k[8], l27 ^ k[50],
		l28 ^ k[42], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[52], l28 ^ k[51], l29 ^ k[21], l30 ^ k[9], l31 ^ k[15],
		l0 ^ k[36], &r4, &r26, &r14, &r20);
	
	/* Round 7 */
	s1 (r31 ^ k[10], r0 ^ k[6], r1 ^ k[46], r2 ^ k[55], r3 ^ k[33],
		r4 ^ k[4], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[47], r4 ^ k[26], r5 ^ k[17], r6 ^ k[11], r7 ^ k[34],
		r8 ^ k[39], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[48], r8 ^ k[20], r9 ^ k[53], r10 ^ k[54], r11 ^ k[12],
		r12 ^ k[25], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[24], r12 ^ k[18], r13 ^ k[19], r14 ^ k[27], r15 ^ k[3],
		r16 ^ k[40], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[31], r16 ^ k[49], r17 ^ k[43], r18 ^ k[30], r19 ^ k[14],
		r20 ^ k[15], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[36], r20 ^ k[51], r21 ^ k[42], r22 ^ k[0], r23 ^ k[2],
		r24 ^ k[21], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[52], r24 ^ k[28], r25 ^ k[44], r26 ^ k[22], r27 ^ k[9],
		r28 ^ k[1], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[7], r28 ^ k[38], r29 ^ k[35], r30 ^ k[23], r31 ^ k[29],
		r0 ^ k[50], &l4, &l26, &l14, &l20);
	
	/* Round 8 */
	s1 (l31 ^ k[24], l0 ^ k[20], l1 ^ k[3], l2 ^ k[12], l3 ^ k[47],
		l4 ^ k[18], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[4], l4 ^ k[40], l5 ^ k[6], l6 ^ k[25], l7 ^ k[48],
		l8 ^ k[53], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[5], l8 ^ k[34], l9 ^ k[10], l10 ^ k[11], l11 ^ k[26],
		l12 ^ k[39], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[13], l12 ^ k[32], l13 ^ k[33], l14 ^ k[41], l15 ^ k[17],
		l16 ^ k[54], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[45], l16 ^ k[8], l17 ^ k[2], l18 ^ k[44], l19 ^ k[28],
		l20 ^ k[29], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[50], l20 ^ k[38], l21 ^ k[1], l22 ^ k[14], l23 ^ k[16],
		l24 ^ k[35], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[7], l24 ^ k[42], l25 ^ k[31], l26 ^ k[36], l27 ^ k[23],
		l28 ^ k[15], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[21], l28 ^ k[52], l29 ^ k[49], l30 ^ k[37], l31 ^ k[43],
		l0 ^ k[9], &r4, &r26, &r14, &r20);
	
	/* Round 9 */
	s1 (r31 ^ k[6], r0 ^ k[27], r1 ^ k[10], r2 ^ k[19], r3 ^ k[54],
		r4 ^ k[25], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[11], r4 ^ k[47], r5 ^ k[13], r6 ^ k[32], r7 ^ k[55],
		r8 ^ k[3], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[12], r8 ^ k[41], r9 ^ k[17], r10 ^ k[18], r11 ^ k[33],
		r12 ^ k[46], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[20], r12 ^ k[39], r13 ^ k[40], r14 ^ k[48], r15 ^ k[24],
		r16 ^ k[4], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[52], r16 ^ k[15], r17 ^ k[9], r18 ^ k[51], r19 ^ k[35],
		r20 ^ k[36], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[2], r20 ^ k[45], r21 ^ k[8], r22 ^ k[21], r23 ^ k[23],
		r24 ^ k[42], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[14], r24 ^ k[49], r25 ^ k[38], r26 ^ k[43], r27 ^ k[30],
		r28 ^ k[22], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[28], r28 ^ k[0], r29 ^ k[1], r30 ^ k[44], r31 ^ k[50],
		r0 ^ k[16], &l4, &l26, &l14, &l20);
	
	/* Round 10 */
	s1 (l31 ^ k[20], l0 ^ k[41], l1 ^ k[24], l2 ^ k[33], l3 ^ k[11],
		l4 ^ k[39], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[25], l4 ^ k[4], l5 ^ k[27], l6 ^ k[46], l7 ^ k[12],
		l8 ^ k[17], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[26], l8 ^ k[55], l9 ^ k[6], l10 ^ k[32], l11 ^ k[47],
		l12 ^ k[3], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[34], l12 ^ k[53], l13 ^ k[54], l14 ^ k[5], l15 ^ k[13],
		l16 ^ k[18], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[7], l16 ^ k[29], l17 ^ k[23], l18 ^ k[38], l19 ^ k[49],
		l20 ^ k[50], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[16], l20 ^ k[0], l21 ^ k[22], l22 ^ k[35], l23 ^ k[37],
		l24 ^ k[1], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[28], l24 ^ k[8], l25 ^ k[52], l26 ^ k[2], l27 ^ k[44],
		l28 ^ k[36], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[42], l28 ^ k[14], l29 ^ k[15], l30 ^ k[31], l31 ^ k[9],
		l0 ^ k[30], &r4, &r26, &r14, &r20);
	
	/* Round 11 */
	s1 (r31 ^ k[34], r0 ^ k[55], r1 ^ k[13], r2 ^ k[47], r3 ^ k[25],
		r4 ^ k[53], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[39], r4 ^ k[18], r5 ^ k[41], r6 ^ k[3], r7 ^ k[26],
		r8 ^ k[6], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[40], r8 ^ k[12], r9 ^ k[20], r10 ^ k[46], r11 ^ k[4],
		r12 ^ k[17], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[48], r12 ^ k[10], r13 ^ k[11], r14 ^ k[19], r15 ^ k[27],
		r16 ^ k[32], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[21], r16 ^ k[43], r17 ^ k[37], r18 ^ k[52], r19 ^ k[8],
		r20 ^ k[9], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[30], r20 ^ k[14], r21 ^ k[36], r22 ^ k[49], r23 ^ k[51],
		r24 ^ k[15], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[42], r24 ^ k[22], r25 ^ k[7], r26 ^ k[16], r27 ^ k[31],
		r28 ^ k[50], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[1], r28 ^ k[28], r29 ^ k[29], r30 ^ k[45], r31 ^ k[23],
		r0 ^ k[44], &l4, &l26, &l14, &l20);
	
	/* Round 12 */
	s1 (l31 ^ k[48], l0 ^ k[12], l1 ^ k[27], l2 ^ k[4], l3 ^ k[39],
		l4 ^ k[10], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[53], l4 ^ k[32], l5 ^ k[55], l6 ^ k[17], l7 ^ k[40],
		l8 ^ k[20], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[54], l8 ^ k[26], l9 ^ k[34], l10 ^ k[3], l11 ^ k[18],
		l12 ^ k[6], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[5], l12 ^ k[24], l13 ^ k[25], l14 ^ k[33], l15 ^ k[41],
		l16 ^ k[46], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[35], l16 ^ k[2], l17 ^ k[51], l18 ^ k[7], l19 ^ k[22],
		l20 ^ k[23], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[44], l20 ^ k[28], l21 ^ k[50], l22 ^ k[8], l23 ^ k[38],
		l24 ^ k[29], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[1], l24 ^ k[36], l25 ^ k[21], l26 ^ k[30], l27 ^ k[45],
		l28 ^ k[9], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[15], l28 ^ k[42], l29 ^ k[43], l30 ^ k[0], l31 ^ k[37],
		l0 ^ k[31], &r4, &r26, &r14, &r20);
	
	/* Round 13 */
	s1 (r31 ^ k[5], r0 ^ k[26], r1 ^ k[41], r2 ^ k[18], r3 ^ k[53],
		r4 ^ k[24], &l8, &l16, &l22, &l30);
	s2 (r3 ^ k[10], r4 ^ k[46], r5 ^ k[12], r6 ^ k[6], r7 ^ k[54],
		r8 ^ k[34], &l12, &l27, &l1, &l17);
	s3 (r7 ^ k[11], r8 ^ k[40], r9 ^ k[48], r10 ^ k[17], r11 ^ k[32],
		r12 ^ k[20], &l23, &l15, &l29, &l5);
	s4 (r11 ^ k[19], r12 ^ k[13], r13 ^ k[39], r14 ^ k[47], r15 ^ k[55],
		r16 ^ k[3], &l25, &l19, &l9, &l0);
	s5 (r15 ^ k[49], r16 ^ k[16], r17 ^ k[38], r18 ^ k[21], r19 ^ k[36],
		r20 ^ k[37], &l7, &l13, &l24, &l2);
	s6 (r19 ^ k[31], r20 ^ k[42], r21 ^ k[9], r22 ^ k[22], r23 ^ k[52],
		r24 ^ k[43], &l3, &l28, &l10, &l18);
	s7 (r23 ^ k[15], r24 ^ k[50], r25 ^ k[35], r26 ^ k[44], r27 ^ k[0],
		r28 ^ k[23], &l31, &l11, &l21, &l6);
	s8 (r27 ^ k[29], r28 ^ k[1], r29 ^ k[2], r30 ^ k[14], r31 ^ k[51],
		r0 ^ k[45], &l4, &l26, &l14, &l20);
	
	/* Round 14 */
	s1 (l31 ^ k[19], l0 ^ k[40], l1 ^ k[55], l2 ^ k[32], l3 ^ k[10],
		l4 ^ k[13], &r8, &r16, &r22, &r30);
	s2 (l3 ^ k[24], l4 ^ k[3], l5 ^ k[26], l6 ^ k[20], l7 ^ k[11],
		l8 ^ k[48], &r12, &r27, &r1, &r17);
	s3 (l7 ^ k[25], l8 ^ k[54], l9 ^ k[5], l10 ^ k[6], l11 ^ k[46],
		l12 ^ k[34], &r23, &r15, &r29, &r5);
	s4 (l11 ^ k[33], l12 ^ k[27], l13 ^ k[53], l14 ^ k[4], l15 ^ k[12],
		l16 ^ k[17], &r25, &r19, &r9, &r0);
	s5 (l15 ^ k[8], l16 ^ k[30], l17 ^ k[52], l18 ^ k[35], l19 ^ k[50],
		l20 ^ k[51], &r7, &r13, &r24, &r2);
	s6 (l19 ^ k[45], l20 ^ k[1], l21 ^ k[23], l22 ^ k[36], l23 ^ k[7],
		l24 ^ k[2], &r3, &r28, &r10, &r18);
	s7 (l23 ^ k[29], l24 ^ k[9], l25 ^ k[49], l26 ^ k[31], l27 ^ k[14],
		l28 ^ k[37], &r31, &r11, &r21, &r6);
	s8 (l27 ^ k[43], l28 ^ k[15], l29 ^ k[16], l30 ^ k[28], l31 ^ k[38],
		l0 ^ k[0], &r4, &r26, &r14, &r20);
	
	/* Round 15 */
	s1 (r31 ^ k[33], r0 ^ k[54], r1 ^ k[12], r2 ^ k[46], r3 ^ k[24],
		r4 ^ k[27], &l8, &l16, &l22, &l30);
	result &= ~(l8 ^ c[5]);
	result &= ~(l16 ^ c[3]);
	result &= ~(l22 ^ c[51]);
	result &= ~(l30 ^ c[49]);
	if (result == 0)
	    return (0);
	s2 (r3 ^ k[13], r4 ^ k[17], r5 ^ k[40], r6 ^ k[34], r7 ^ k[25],
		r8 ^ k[5], &l12, &l27, &l1, &l17);
	result &= ~(l12 ^ c[37]);
	result &= ~(l27 ^ c[25]);
	result &= ~(l1 ^ c[15]);
	result &= ~(l17 ^ c[11]);
	if (result == 0)
	    return (0);
	s3 (r7 ^ k[39], r8 ^ k[11], r9 ^ k[19], r10 ^ k[20], r11 ^ k[3],
		r12 ^ k[48], &l23, &l15, &l29, &l5);
	result &= ~(l23 ^ c[59]);
	result &= ~(l15 ^ c[61]);
	result &= ~(l29 ^ c[41]);
	result &= ~(l5 ^ c[47]);
	if (result == 0)
	    return (0);
	s4 (r11 ^ k[47], r12 ^ k[41], r13 ^ k[10], r14 ^ k[18], r15 ^ k[26],
		r16 ^ k[6], &l25, &l19, &l9, &l0);
	result &= ~(l25 ^ c[9]);
	result &= ~(l19 ^ c[27]);
	result &= ~(l9 ^ c[13]);
	result &= ~(l0 ^ c[7]);
	if (result == 0)
	    return (0);
	s5 (r15 ^ k[22], r16 ^ k[44], r17 ^ k[7], r18 ^ k[49], r19 ^ k[9],
		r20 ^ k[38], &l7, &l13, &l24, &l2);
	result &= ~(l7 ^ c[63]);
	result &= ~(l13 ^ c[45]);
	result &= ~(l24 ^ c[1]);
	result &= ~(l2 ^ c[23]);
	if (result == 0)
	    return (0);
	s6 (r19 ^ k[0], r20 ^ k[15], r21 ^ k[37], r22 ^ k[50], r23 ^ k[21],
		r24 ^ k[16], &l3, &l28, &l10, &l18);
	result &= ~(l3 ^ c[31]);
	result &= ~(l28 ^ c[33]);
	result &= ~(l10 ^ c[21]);
	result &= ~(l18 ^ c[19]);
	if (result == 0)
	    return (0);
	s7 (r23 ^ k[43], r24 ^ k[23], r25 ^ k[8], r26 ^ k[45], r27 ^ k[28],
		r28 ^ k[51], &l31, &l11, &l21, &l6);
	result &= ~(l31 ^ c[57]);
	result &= ~(l11 ^ c[29]);
	result &= ~(l21 ^ c[43]);
	result &= ~(l6 ^ c[55]);
	if (result == 0)
	    return (0);
	s8 (r27 ^ k[2], r28 ^ k[29], r29 ^ k[30], r30 ^ k[42], r31 ^ k[52],
		r0 ^ k[14], &l4, &l26, &l14, &l20);
	result &= ~(l4 ^ c[39]);
	result &= ~(l26 ^ c[17]);
	result &= ~(l14 ^ c[53]);
	result &= ~(l20 ^ c[35]);
	if (result == 0)
	    return (0);
	
	/* Round 16 */
	s1 (l31 ^ k[40], l0 ^ k[4], l1 ^ k[19], l2 ^ k[53], l3 ^ k[6],
		l4 ^ k[34], &r8, &r16, &r22, &r30);
	result &= ~(r8 ^ c[4]);
	result &= ~(r16 ^ c[2]);
	result &= ~(r22 ^ c[50]);
	result &= ~(r30 ^ c[48]);
	if (result == 0)
	    return (0);
	s2 (l3 ^ k[20], l4 ^ k[24], l5 ^ k[47], l6 ^ k[41], l7 ^ k[32],
		l8 ^ k[12], &r12, &r27, &r1, &r17);
	result &= ~(r12 ^ c[36]);
	result &= ~(r27 ^ c[24]);
	result &= ~(r1 ^ c[14]);
	result &= ~(r17 ^ c[10]);
	if (result == 0)
	    return (0);
	s3 (l7 ^ k[46], l8 ^ k[18], l9 ^ k[26], l10 ^ k[27], l11 ^ k[10],
		l12 ^ k[55], &r23, &r15, &r29, &r5);
	result &= ~(r23 ^ c[58]);
	result &= ~(r15 ^ c[60]);
	result &= ~(r29 ^ c[40]);
	result &= ~(r5 ^ c[46]);
	if (result == 0)
	    return (0);
	s4 (l11 ^ k[54], l12 ^ k[48], l13 ^ k[17], l14 ^ k[25], l15 ^ k[33],
		l16 ^ k[13], &r25, &r19, &r9, &r0);
	result &= ~(r25 ^ c[8]);
	result &= ~(r19 ^ c[26]);
	result &= ~(r9 ^ c[12]);
	result &= ~(r0 ^ c[6]);
	if (result == 0)
	    return (0);
	s5 (l15 ^ k[29], l16 ^ k[51], l17 ^ k[14], l18 ^ k[1], l19 ^ k[16],
		l20 ^ k[45], &r7, &r13, &r24, &r2);
	result &= ~(r7 ^ c[62]);
	result &= ~(r13 ^ c[44]);
	result &= ~(r24 ^ c[0]);
	result &= ~(r2 ^ c[22]);
	if (result == 0)
	    return (0);
	s6 (l19 ^ k[7], l20 ^ k[22], l21 ^ k[44], l22 ^ k[2], l23 ^ k[28],
		l24 ^ k[23], &r3, &r28, &r10, &r18);
	result &= ~(r3 ^ c[30]);
	result &= ~(r28 ^ c[32]);
	result &= ~(r10 ^ c[20]);
	result &= ~(r18 ^ c[18]);
	if (result == 0)
	    return (0);
	s7 (l23 ^ k[50], l24 ^ k[30], l25 ^ k[15], l26 ^ k[52], l27 ^ k[35],
		l28 ^ k[31], &r31, &r11, &r21, &r6);
	result &= ~(r31 ^ c[56]);
	result &= ~(r11 ^ c[28]);
	result &= ~(r21 ^ c[42]);
	result &= ~(r6 ^ c[54]);
	if (result == 0)
	    return (0);
	s8 (l27 ^ k[9], l28 ^ k[36], l29 ^ k[37], l30 ^ k[49], l31 ^ k[0],
		l0 ^ k[21], &r4, &r26, &r14, &r20);
	result &= ~(r4 ^ c[38]);
	result &= ~(r26 ^ c[16]);
	result &= ~(r14 ^ c[52]);
	result &= ~(r20 ^ c[34]);
	if (result == 0)
	    return (0);
	return (result);
}
