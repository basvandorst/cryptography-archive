BOOKROOT=..
TOOLSDIR=$(BOOKROOT)/tools
PSDIR=$(BOOKROOT)/ps
YAPP=$(TOOLSDIR)/yapp
MAKEMANIFEST=$(TOOLSDIR)/makemanifest
PSGEN=BOOKROOT=$(BOOKROOT) $(TOOLSDIR)/psgen
INCLUDERES=(cd $(PSDIR); includeres)

code.ps pagenums: filelist footer.ps MANIFEST books
	$(PSGEN) -P2 -l3 -DfooterFile=footer.ps filelist pagenums 1 \
		| $(INCLUDERES) > code.ps

books:
	ln -s $(BOOKROOT) books

MANIFEST: filelist
	$(MAKEMANIFEST) $< > $@

clean:
	rm -f `cat .cvsignore`

gv%: %.ps
	gv $<
