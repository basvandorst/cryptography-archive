-----BEGIN PGP SIGNED MESSAGE-----

***************************************************************   
* Este documento nao tem acentos para que seja correctamente  *   
* legivel em qualquer computador.                             *   
***************************************************************   
                                                                  
NOTAS SOBRE A TRADUCAO PORTUGUESA DO PGP:                         
                                                                  
Ao   descomprimir  o  ficheiro  principal  deverao  ter  os       
seguintes ficheiros:                                              
                                                                  
  -README.TXT     ;Estas breves notas que estao a ver             
  -MARCOKEY.ASC   ;A minha chave publica de 2048 bits             
  -TR_AMIGA.LHA   ;A traducao do PGP para o AMIGA                 
  -TR_AMIGA.SIG                                                   
  -TR_PC.ZIP      ;A traducao do PGP para o PC                    
  -TR_PC.SIG                                                      
                                                                  
                                                                  
Descomprimam o  ficheiro 'TR_AMIGA.LHA' ou 'TR_PC.ZIP' para       
a  directoria  onde esta  o programa PGP e facam um edit do       
ficheiro 'CONFIG.TXT'. Procurem a expressao 'Language = En'       
e alterem-na para 'Language = Pt'.                                
                                                                  
                                                                  
UTILIZADORES PC:                                                  
Caso estejam  a  correr  o PGP num PC, tanto o 'CONFIG.SYS'       
como  o 'AUTOEXEC.BAT'  deverao  seleccionar  a  pagina  de       
codigo (code page) 860 (Portugal). Ex:                            
                                                                  
                   ---- CONFIG.SYS ----                           
Country=351,860,C:\WINDOWS\COMMAND\country.sys                    
devicehigh=C:\WINDOWS\COMMAND\display.sys con=(ega,860,1)         
                                                                  
                  ---- AUTOEXEC.BAT ----                          
lh mode con cp prepare=((860) C:\WINDOWS\COMMAND\ega.cpi)         
lh mode con cp select=860                                         
lh keyb po,,C:\WINDOWS\COMMAND\keyboard.sys                       
                                                                  

-----BEGIN PGP SIGNATURE-----
Version: 2.6.3i
Charset: noconv

iQEVAwUBM2DzWXUEZURL1FB9AQHIlgf+J33CwPqyR7RKfS7Fo+PVfzheJR14b/r7
2r38JiP6Lh3qR3ZlZ1eDTLCHPSLhR7YJbUeaM2yPNbb87BB+X1TFBvk1IaLSUTKM
ni4P5TJWnNBzGKGn8lma806/N4UDnamjEwCXCLbykWiRRWi05ryGrYg1hrhSh+3Q
GQ6ucZ2nMpZXzm1H1DDukzxd6UuXbiRqVGDC66a4lswGIefOx6dcZlLT7bfRpfDY
S/y33QDSZD3upFd/0Kmpl+1Aa2iD/c1QNyL9ZS9c80Q0n0A0XyYpFKqSN8yZbmph
wH5PKDkRJISjpOCe47EQEm9//EgjKALLrqkCo9W+4SMW37rAMVFVzw==
=xf53
-----END PGP SIGNATURE-----
