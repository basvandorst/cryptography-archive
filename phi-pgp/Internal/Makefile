#
# Makefile pour la documentation
#
# Le point d'entree par defaut construit et imprime le mode d'emploi de 
# crmodel.
#
# Les autres points d'entree possibles sont : 
#  - specification.ps pour imprimer le dossier de specification, 
#  - preliminaire.ps pour imprimer le dossier de conception preliminaire, 
#  - detaillee.ps pour imprimer le dossier de conception preliminaire, 
#  - clean ou veryclean pour revenir aux fichiers de distribution seuls,
#  - all pour imprimer tous les documents.

#
# nom du programme LaTeX
#
LATEX=latex
#
# nom du programme de conversion du fichier dvi en fichier PostScript
#
DVIPS=dvips
DVIFLAGS=-t a4 -o
#
# nom de l'imprimante PostScript
#
IMPRIMANTE=hp

OBJ= decoCons.tex gereIden.tex alloueIde.tex coheDecl.tex chargCond.tex \
		chargArb.tex veriType.tex attaArb.tex constBin.tex \
		coheExpr.tex erreAnal.tex analyse.tex geneSubr.tex \
		prepDer.tex deriv.tex bouclArb.tex crmodel.tex evalArb.tex \
		analDer.tex decorArb.tex prepEval.tex

default : modeemploi.ps

all :	modeemploi.ps specification.ps detaillee.ps preliminaire.ps

.SUFFIXES : .tex .ps

detaillee.ps : $(OBJ)

$(OBJ) :
	prepdetail $@

.tex.ps :
	$(LATEX) $*.tex
	$(LATEX) $*.tex
	$(LATEX) $*.tex
	$(DVIPS) $(DVIFLAGS) $*.ps $*.dvi
	lpr -P$(IMPRIMANTE) -r $*.ps

veryclean clean : 
	$(RM) *.log *.aux *.toc *.dvi *.ps $(OBJ)
