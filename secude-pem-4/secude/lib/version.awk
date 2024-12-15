{ major = $1; minor = $2; local = $3 + 1; sfw = $4; 
	  if (NF >= 5) note = $5; else note = ""; }
END	{ printf "char *%sversion = \"%s%s %d.%d #%d ", sfw, sfw, note, major, minor, local; 
	  printf "%d\n", local > "version.local"; }
