call wnhide stele.pcx 4 test.txt MYPCXPASSWORD -s:6
call wnunhide wnstego.pcx 4 outfile.txt MYPCXPASSWORD -s:6
fc test.txt outfile.txt
