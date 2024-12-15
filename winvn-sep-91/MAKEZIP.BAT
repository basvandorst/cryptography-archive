del d:winvn.zip
pkzip -a -o d:winvn winvn.wri winvn.bsc
pkzip -a -o d:winvn winvn.exe winvn.hpj winvn.hlp
pkzip -a -o d:winvn winvn.h wvglob.h winundoc.h wvdlg.h
pkzip -a -o d:winvn winvn.c wv*.c uucode.c newdes2.c
pkzip -a -o d:winvn find_vec.c vec_srch.c shellsor.c pcemul.c
pkzip -a -o d:winvn winvn.rc winvn.mak winvn.def winvn.lnk
pkzip -a -o d:winvn *.dlg
pkzip -a -o d:winvn winvn.ico wv*.ico *.bmp
pkzip -a -o d:winvn newsrc wvhl*.rtf wvlist.doc makezip.bat wv*.wp5
