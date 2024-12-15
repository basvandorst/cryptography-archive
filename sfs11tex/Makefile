LATEX=latex

all: sfs1.dvi

sfs1.dvi: sfs1.tex sfs2.tex sfs3.tex sfs1.toc
	$(LATEX) sfs1

sfs1.toc: sfs1.tex sfs2.tex sfs3.tex
	$(LATEX) sfs1
	$(LATEX) sfs1

clean:
	rm -f sfs1.dvi sfs1.aux sfs1.toc sfs1.log

tar: sfs_doc.tar

sfs_doc.tar: sfs1.tex sfs2.tex sfs3.tex Makefile
	tar cvf sfs_doc.tar sfs1.tex sfs2.tex sfs3.tex Makefile

