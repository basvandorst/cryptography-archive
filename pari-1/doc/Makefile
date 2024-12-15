
# type make manual to tex the manual. This will create a users.dvi 

all:	manual

manual:	users.dvi


users.dvi: users.tex titlepage.tex \
        usersch1.tex usersch2.tex usersch3.tex usersch4.tex\
        usersch5.tex tutorial.tex appa.tex appb.tex appc.tex appd.tex 
	rm -f users.std;tex users;\
        sort -f users.idx > users.std;\
	tex users;rm -f users.idx

clean:	
	rm -f users.log users.dvi users.std users.idx users.toc *.ps
