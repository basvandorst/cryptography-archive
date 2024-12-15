all:
	nmake -f gensock.mak

gensck32:
	nmake -f gensck32.mak

# how many junks files can there be ???

clean:
	-del *.obj
	-del *.lib
	-del *.exp
	-del *.res
	-del *.dll
	-del *.bsc
	-del *.map
	-del *.sbr
	-del *.bsc
	-del *.pdb

zip:
	-del \gensock.zip
	zip -r \gensock.zip .

