main(argc, argv)
     int argc;
     char **argv;
{
  int x;

  if (argc != 2)
    abort();

  x = strtol (argv[1], 0, 0);

  puts("; HP-PA-1.1 mpn_addmul_1 -- Multiply a limb vector with a limb and");
  puts("; add the result to a second limb vector.");
  puts("");
  puts("; Copyright (C) 1992 Free Software Foundation, Inc.");
  puts("");
  puts("; This file is part of the GNU MP Library.");
  puts("");
  puts("; The GNU MP Library is free software; you can redistribute it and/or modify");
  puts("; it under the terms of the GNU General Public License as published by");
  puts("; the Free Software Foundation; either version 2, or (at your option)");
  puts("; any later version.");
  puts("");
  puts("; The GNU MP Library is distributed in the hope that it will be useful,");
  puts("; but WITHOUT ANY WARRANTY; without even the implied warranty of");
  puts("; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
  puts("; GNU General Public License for more details.");
  puts("");
  puts("; You should have received a copy of the GNU General Public License");
  puts("; along with the GNU MP Library; see the file COPYING.  If not, write to");
  puts("; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.");
  puts("");
  puts("");
  puts("; INPUT PARAMETERS");
  puts("; res_ptr	r26");
  puts("; s1_ptr	r25");
  puts("; size		r24");
  puts("; s2_limb	r23");
  puts("");
  puts("; This code runs in 11 cycles/limb.  The inner loop is optimal on the 7000 CPU.");
  puts("; For the 7100 CPU we should rewrite this quite much due to superscalar");
  puts("; instruction issue and shorter pipelines...");
  puts("");
  puts("	.SPACE $TEXT$");
  puts("	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY");
  puts("	.SPACE $TEXT$");
  puts("	.SUBSPA $CODE$");
  puts("");
  puts("	.align 4");
  puts("	.EXPORT mpn_submul_1");
  puts("	.EXPORT mpn_submul_1,PRIV_LEV=3,ARGW0=GR,ARGW1=GR,ARGW2=GR,ARGW3=GR,RTNVAL=GR");
  puts("mpn_submul_1");
  puts("	.PROC");
  puts("	.CALLINFO FRAME=48,NO_CALLS");
  puts("	.ENTRY");
  puts("");
  puts("	ldo	 64(%r30),%r30");
  puts("	fldws,ma 4(%r25),%fr5");
  if ((x & 16) != 0)
    puts("	andcm	 %r0,%r23,%r23");
  if ((x & 32) != 0)
    puts("	sub	 %r0,%r23,%r23");
  puts("	stw	 %r23,-16(%r30)		; move s2_limb ...");
  puts("	addib,=	 -1,%r24,just_one_limb");
  puts("	fldws	 -16(%r30),%fr4		; ... into fr4");
  puts("	add	 %r0,%r0,%r0		; clear carry");
  puts("	xmpyu	 %fr4,%fr5,%fr5");
  puts("	fldws,ma 4(%r25),%fr6");
  puts("	fstds	 %fr5,-16(%r30)");
  puts("	xmpyu	 %fr4,%fr6,%fr6");
  puts("	ldw	 -12(%r30),%r19		; least significant limb in product");
  puts("	ldw	 -16(%r30),%r28");
  puts("");
  puts("	fstds	 %fr6,-16(%r30)");
  puts("	addib,=	 -1,%r24,Lend2");
  puts("	ldw	 -12(%r30),%r1");
  puts("");
  puts("; Main loop");

  puts("Loop");
  puts("	ldws	 0(%r26),%r29");
  puts("	fldws,ma 4(%r25),%fr5");
  if ((x & 1) != 0)
    puts("	add	 %r29,%r19,%r19");
  else
    puts("	sub	 %r29,%r19,%r19");
  puts("	stws,ma	 %r19,4(%r26)");
  puts("	xmpyu	 %fr4,%fr5,%fr5");
  if ((x & 2) != 0)
    if ((x & 4) != 0)
      puts("	addc	 %r28,%r1,%r19");
    else
      puts("	addc	 %r28,%r19,%r1");
  else
    if ((x & 4) != 0)
      puts("	subb	 %r28,%r1,%r19");
    else
      puts("	subb	 %r28,%r19,%r1");
  puts("	ldw	 -16(%r30),%r28");
  puts("	fstds	 %fr5,-16(%r30)");
  if ((x & 8) != 0)
    puts("	subb	 %r0,%r28,%r28");
  else
    puts("	subb	 %r28,%r0,%r28");
  puts("	addib,<> -1,%r24,Loop");
  puts("	 ldw	 -12(%r30),%r1");
  puts("");

  puts("Lend2	ldw	 0(%r26),%r29");
  if ((x & 1) != 0)
    puts("	add	 %r29,%r19,%r19");
  else
    puts("	sub	 %r29,%r19,%r19");
  puts("	stws,ma	 %r19,4(%r26)");
  if ((x & 2) != 0)
    if ((x & 4) != 0)
      puts("	addc	 %r28,%r1,%r19");
    else
      puts("	addc	 %r28,%r19,%r1");
  else
    if ((x & 4) != 0)
      puts("	subb	 %r28,%r1,%r19");
    else
      puts("	subb	 %r28,%r19,%r1");
  puts("	ldw	 -16(%r30),%r28");
  puts("	ldws	 0(%r26),%r29");
  if ((x & 8) != 0)
    puts("	subb	 %r0,%r28,%r28");
  else
    puts("	subb	 %r28,%r0,%r28");
  if ((x & 1) != 0)
    puts("	add	 %r29,%r19,%r19");
  else
    puts("	sub	 %r29,%r19,%r19");
  puts("	stws,ma	 %r19,4(%r26)");
  puts("	addc	 %r0,%r28,%r28");
  puts("	bv	 0(%r2)");
  puts("	ldo	 -64(%r30),%r30");
  puts("");

  puts("just_one_limb");
  puts("	xmpyu	 %fr4,%fr5,%fr5");
  puts("	ldw	 0(%r26),%r29");
  puts("	fstds	 %fr5,-16(%r30)");
  puts("	ldw	 -12(%r30),%r1");
  puts("	ldw	 -16(%r30),%r28");
  puts("	add	 %r29,%r1,%r19");
  puts("	stw	 %r19,0(%r26)");
  puts("	addc	 %r0,%r28,%r28");
  puts("	bv	 0(%r2)");
  puts("	ldo	 -64(%r30),%r30");
  puts("");
  puts("	.EXIT");
  puts("	.PROCEND");
}
