cl /c /DDOS /DLOW_MEM gif_comp.c
cl /c /DDOS /DLOW_MEM mandsteg.c
cl /DDOS /DLOW_MEM gifextr.c
link mandsteg.obj+gif_comp.obj;

