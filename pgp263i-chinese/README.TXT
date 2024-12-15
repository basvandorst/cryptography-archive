 Chinese Language Modules v1.00 for PGP 2.6.3i
 ===============================================
  There are two Chinese language modules included in this package.  They
are BIG-5 Chinese language module, which is the most widely used code scheme
in Chinese computer processing.  As well as GB Chinese language module,
which is a code scheme used mainly in Mainland China.

SETUP
=====
1) Extract all the files into a directory.  If you are using DOS pkunzip,
   don't forget to use the -d parameter to restore all the files in their
   default directories.
   ex. pkunzip -d pgp263i-chinese.zip

2) for BIG-5:
   Copy the two files LANGUAGE.TXT and ZH.HLP in BIG5 directory to
   replace the files in your PGP directory.

   for GB:
   Copy the two files LANGUAGE.TXT and ZH.HLP in GB directory to
   replace the files in your PGP directory.

3) Edit the CONFIG.TXT file in your PGP directory; change
   Language = en     ----- to -----> Language = zh
   CharSet = cp850   ----- to -----> CharSet = latin1

Others
======
  These two Chinese language modules are translated by Charles Tsai.
  If you have any further questions or suggestions, please feel free to
e-mail me at cxt4@po.cwru.edu.

