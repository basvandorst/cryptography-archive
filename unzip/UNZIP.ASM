;
;
; UNZIP v1.1, Pkzip Password Finder
;
; By Dcypher (dcypher@mhv.net)
;
; 
; 
;-------------------------------------------------------
				                    	
	.286                 				
	.model small            			
        .code	                   			
							
	org     100h            			
							
begin:  jmp     start           			
							
;========================================================
sernum          db  ' DCYPHER@MHV.NET / UNZIP v1.1 '	
							
password      	db  41 dup(0)                           
meout   	db  12 dup(0)                           
							
bcheck  	db  0                                   
zipdat  	db  0Ch dup(0)                           
count   	db  0Ch                                 
							
data1   	db  0Eh dup(0)                          
data3   	db  078h, 056h, 034h, 012h, 089h, 067h  
		db  045h, 023h, 090h, 078h, 056h, 034h   
		db  02fh, 00h                           
				 			
wcount		dw  0					
matches 	db  0                                   
finished	db  0					
							
tname1  	db  'table1',0                          
tname2  	db  'table2',0                          
							
word_off	dw  0
word_handle     dw  0
wcount_high     dw  0
wcount_low	dw  0

word_ptr        dw  0
word_loaded     dw  0
word_last       db  0

z_handle	dw  0                                   
zpoint_high     dw  0
zpoint_low      dw  0
z_loaded	dw  0
z_last   	db  0                                           
efiles  	dw  0                                   
zpoint  	dw  0                                   
zipinfo 	db  143 dup(0)                          
							
;------------------------------------------------------------------------------

enlow:  mov     ah, 09h
        mov     dx, offset en_low
        int     21h
        jmp     bye

fprob:  mov     ah, 09h
        mov     dx, offset f_prob
        int     21h
        jmp     bye

prtuse: mov     ah, 09h					
        mov     dx, offset usage
        int     21h

bye:    mov     ah, 4cH                                 
	int     21h                                     

;========================================================
							
start:  mov     ax, 0600h
        mov     bx, 0700h
        mov     cx, 0
        mov     dx, 3250h
        int     10h

        mov     ah, 09h
        mov     dx, offset intro
        int     21h

        mov     bx, 0080h
        cmp     byte ptr [bx], 0
        jz      prtuse 

        call    null
        jc      prtuse
        call    cfiles
        jc      fprob

	call    tables                                  
	call    openz                                   
        cmp     efiles, 3		                 
	jb      enlow

        mov     ah, 09h                                 
	mov     dx, offset ui3                          
	int     21h                                     

 mloop: call    words
        call    reset
        call    main
        call    check
        jmp     mloop

  done: mov     ah, 09h
        mov     dx, offset d_finished
        int     21h
        jmp     bye

;==============================================================================

reset:  mov     count, 0Ch                              

	mov     cx, 0Eh                                 
	mov     ax, ds                                  
	mov     es, ax                                  
	mov     si, offset data3                        
	mov     di, offset data1                        
	repz    movsb                                   
							
	cmp     matches, 0                              
	jnz     reset1                                  
	mov     bx, offset zipinfo                      
	mov     zpoint, bx                              
							
reset1: mov     si, zpoint		                
	mov     al, byte ptr [si]                       
	mov     bcheck, al		                
	inc     si                                      
	mov     cx, 0Ch                                 
	mov     di, offset zipdat                       
	repz    movsb                                   
	add     zpoint, 0Dh		                
	ret
							
;==============================================================================
							 
main:   mov     di, offset meout                        
	mov     si, offset zipdat                       
	call    main1                                   
loop2:  call    main2                                   
	xor     al, byte ptr [si]                       
	mov     byte ptr [di], al                       
	call    crypt1                                  
	inc     si                                      
	inc     di                                      
	dec     count		                        
	jnz     loop2                                   
	ret                                             
							
;--------------------------------------------------------
							
main1:  push    si                                      
	mov     si, offset password                     
loop1:  mov     al, byte ptr [si]                       
	call    crypt1                                  
	inc     si                                      
	cmp     byte ptr [si], 0                        
	jnz     loop1                                   
	pop     si                                      
	ret                                             
							
;--------------------------------------------------------
							
main2:  push    si                                      
	mov     si, word ptr [data1+8]                  
	or      si, 2                                   
	mov     ax, si                                  
	xor     al, 1                                   
	mul     si                                      
	mov     al, ah                                  
	pop     si                                      
	ret                                             
							
;--------------------------------------------------------
							
crypt1: push    si                                      
	cld                                             
	xchg    ax, bx                                  
	xor     bh, bh                                  
	mov     ax, cs                                  
	add     ax, 0020h                               
	mov     es, ax                                  
	mov     si, offset data1                        
	call    crypt2                                  
	mov     si, offset [data1+4]                    
	mov     cx, word ptr [si]                       
	mov     bx, word ptr [si+2]                     
	xor     ah, ah                                  
	add     cx, ax                                  
	adc     bx, 00                                  
	mov     dx, 8405h                               
	mov     ax, cx                                  
	mul     dx                                      
	shl     cx, 1                                   
	shl     cx, 1                                   
	shl     cx, 1                                   
	add     ch, cl                                  
	add     dx, cx                                  
	add     dx, bx                                  
	shl     bx, 1                                   
	shl     bx, 1                                   
	add     dx, bx                                  
	add     dh, bl                                  
	mov     cl, 05                                  
	shl     bx, cl                                  
	add     dh, bl                                  
	add     ax, 1                                   
	adc     dx, 00                                  
	mov     word ptr [si], ax                       
	mov     word ptr [si+2], dx                     
	mov     bl, dh                                  
	xor     bh, bh                                  
	mov     si, offset [data1+8]                    
	call    crypt2                                  
	pop     si                                      
	ret                                             
							
;--------------------------------------------------------
							
crypt2: lodsw                                           
	mov     dx, word ptr [si]                       
	xor     bl, al                                  
	mov     al, ah                                  
	mov     ah, dl                                  
	mov     dl, dh                                  
	xor     dh, dh                                  
	shl     bx, 1                                   
							
	push    bx                                      
	push    bx                                      
	add     bx, offset bufA                         
	xor     ax, word ptr cs:[bx]                     
	pop     bx                                      
	add     bx, offset bufB                         
	xor     dx, word ptr cs:[bx]                    
	pop     bx                                      
							
	mov     word ptr [si-2], ax                     
	mov     word ptr [si], dx                       
	ret                                             
							
;==============================================================================
							
check:  mov     si, offset meout                        
	mov     al, bcheck                              
	cmp     byte ptr [si+11], al                     
	jnz     check2                                  

	inc     matches                                 
	mov     ax, efiles	                        
	cmp     matches, al                             
	jnz     check3                                  
							
	call    print                                   
check2: mov     matches, 0                              
        call    working					
check3: ret		                                
							
;--------------------------------------------------------
							
print:  mov     bx, offset password                     
  pt1:  inc     bx
        cmp     byte ptr [bx], 0
        jnz     pt1
        mov     byte ptr [bx], 24h

        mov     ah, 09h
        mov     dx, offset ipw
        int     21h

        mov     ah, 09
        mov     dx, offset password
        int     21h

        mov     ah, 09h
        mov     dx, offset ipw
        int     21h
	ret                                             
							
;--------------------------------------------------------

working:inc     wcount
        cmp     wcount, 50000
        jnz     wk1
        mov     wcount, 0

        mov     ah, 2
        mov     dl, '.'
        int     21h
   wk1: ret

;----------------------------------------------------------
 words: cmp     matches, 0
        jnz     wo5

        cmp     word_last, 0
        jz      wo8
        cmp     word_loaded, 20 
        ja      wo1
        cmp     word_last, 2
        jz      wo1

   wo8: call    mwords
        mov     bx, offset word_dat
        mov     word_ptr, bx

   wo1: mov     bx, offset password        
        mov     di, word_ptr
   wo2: mov     al, byte ptr [di]
        cmp     al, 0Ah
        jz      wo4
        mov     byte ptr [bx], al
        inc     bx
        inc     di
        dec     word_loaded
        jz      wo6
        jmp     wo2

   wo4: inc     di
        dec     word_loaded
        jz      wo6

        mov     word_ptr, di
        mov     byte ptr [bx], 0
   wo5: ret

   wo6: jmp     done
							
;--------------------------------------------------------				
							
mwords: cmp     word_last, 0
        jz      mw1

        mov     cx, word_loaded
  mwp1: cmp     wcount_low, 0
        jz      mwp2
        dec     wcount_low
        dec     cx
        jz      mw1
        jmp     mwp1
  mwp2: dec     wcount_high
        dec     cx
        jnz     mwp1

   mw1: mov     ax, 3D00h
        mov     dx, word_off
        int     21h
        mov     word_handle, ax

        mov     ax, 4200h
        mov     bx, word_handle
        mov     cx, wcount_high
        mov     dx, wcount_low
        int     21h

        mov  	ah, 3Fh
        mov     bx, word_handle
        mov     cx, 60000
        mov     dx, offset word_dat
        int     21h
        mov     word_loaded, ax

        mov     ax, 4201h
        mov     bx, word_handle
        xor     cx, cx
        xor     dx, dx
        int     21h
        mov     wcount_high, dx
        mov     wcount_low, ax

        mov     ah, 3Eh
        mov     bx, word_handle
        int     21h

        cmp     word_loaded, 60000
        jnz     mw3
        mov     word_last, 1
        ret

   mw3: mov     word_last, 2
        ret        

;==============================================================================

  null:	mov    	bx, 81h
    n1: inc     bx
        cmp     byte ptr [bx], 20h
        jnz     n2
        mov     byte ptr [bx], 0
        mov     word_off, bx
        inc     word_off
        jmp     n1
    n2: cmp     byte ptr [bx], 0Dh
        jnz     n1
        mov     byte ptr [bx], 0

        cmp     word_off, 0
        jnz     n3
        stc
    n3: ret

;--------------------------------------------------------

cfiles:	mov  	ax, 3D00h
        mov     dx, offset tname1
        int     21h
        jc      cf6
        mov     bx, ax
        mov     ah, 3Eh
        int     21h
        mov     ax, 3D00h
        mov     dx, offset tname2
        int     21h
        jc      cf6
        mov     bx, ax
        mov     ah, 3Eh
        int     21h

        mov     ax, 3D00h
        mov     dx, 82h
        int     21h
        jc      cf6
        mov     bx, ax
        mov     ah, 3Eh
        int     21h

        mov     ax, 3D00h
        mov     dx, word_off
        int     21h
        jc      cf6
        mov     bx, ax
        mov     ah, 3Eh
        int     21h
        clc
   cf6: ret
;--------------------------------------------------------
tables: mov     ax, 3D00h
        mov     dx, offset tname1                       
	int     21h                                     
	mov     bx, ax                                  
	mov     ah, 3fh                                 
	mov     cx, 512                                  
	mov     dx, offset bufA                         
	int     21h                                     
	mov     ah, 3eh                                 
	int     21h                                     
							
	mov     ax, 3D00h                               
	mov     dx, offset tname2                       
	int     21h                                     
	mov     bx, ax                                  
	mov     ah, 3fh                                 
	mov     cx, 512                                 
	mov     dx, offset bufB                         
	int     21h                                     
	mov     ah, 3eh                                 
	int     21h                                     
  	ret                                             
							
;--------------------------------------------------------

 openz:	mov     bx, offset zipinfo                      
	mov     zpoint, bx                              

   sz1: call    morez
        sub     z_loaded, 50

   	mov     bx, offset word_dat                     
   sz3: cmp     word ptr [bx], 4b50h                    
	jnz     sz4                                     
	cmp     word ptr [bx+2], 0403h                  
	jnz     sz4                                     

	call    pinfo                                   
	call    dataz                                   

   sz4:	dec     z_loaded                                
	jz      sz5
        inc     bx
        jmp     sz3

   sz5: cmp     z_last, 1
        jz      sz1

        ret

;--------------------------------------------------------
							
pinfo:  mov     si, bx                                  
	mov     cx, word ptr [bx+26]                    
	add     si, 30                                  
	mov     ah, 02h                                 
pinfo1: mov     dl, byte ptr [si]                       
	int     21h                                     
	inc     si                                      
	dec     cx                                      
	jnz     pinfo1                                  
							
	mov     ax, word ptr [bx+26]                    
	mov     cx, 14                                  
	sub     cx, ax                                  
	mov     ah, 02h                                 
	mov     dl, 20h                                 
pinfo4: int     21h                                     
	dec     cx                                      
	jnz     pinfo4                                  
							
	mov     ax, word ptr [bx+6]                     
	test    ax, 1                                   
	jz      pinfo2                                  
	mov     ah, 09h                                 
	mov     dx, offset ui4                          
	int     21h                                     
	jmp     pinfo3                                  

pinfo2: mov     ah, 09h                                 
	mov     dx, offset ui5                          
	int     21h                                     
							
pinfo3:	ret                                             
							
;--------------------------------------------------------
							
dataz:	mov     ax, word ptr [bx+6]                      
	test    ax, 1                                   
	jz      dataz2                                  

        mov     si, offset zipinfo			
        add     si, 130
        cmp     zpoint, si
        jae     dataz2
							
	inc     efiles		                        
	mov     al, byte ptr [bx+17]                    
	mov     si, zpoint	                        
	mov     byte ptr [si], al                       
	inc     zpoint		                        
							
	mov     ax, word ptr [bx+26]                    
	add     bx, ax                                  
	add     bx, 30                                  
							
	mov     cx, 0ch                                 
	mov     si, bx                                  
	mov     di, zpoint		                
	repz    movsb                                   
	add     zpoint, 0Ch		                
							
dataz2:	ret                                             
							
;--------------------------------------------------------                                
							
morez:  cmp     z_last, 0
        jz      mz1

        mov     cx, 50
  mzp1: cmp     zpoint_low, 0
        jz      mzp2
        dec     zpoint_low
        dec     cx
        jz      mz1
        jmp     mzp1
  mzp2: dec     zpoint_high
        dec     cx
        jnz     mzp1

   mz1: mov     ax, 3D00h
        mov     dx, 82h
        int     21h
        mov     z_handle, ax

        mov     ax, 4200h                               
	mov     bx, z_handle	                        
	mov     cx, zpoint_high                         
	mov     dx, zpoint_low                          
	int     21h                                     

        mov     ah, 3Fh
        mov     bx, z_handle
        mov     cx, 60000
        mov     dx, offset word_dat
        int     21h
        mov     z_loaded, ax
        
        mov     ax, 4201h
        mov     bx, z_handle
        mov     cx, 0
        mov     dx, 0
        int     21h
        mov     zpoint_high, dx
        mov     zpoint_low, ax

        mov     ah, 3Eh
        mov     bx, z_handle
        int     21h
    
        cmp     z_loaded, 60000
        jnz     mz4
        mov     z_last, 1
        ret

   mz4: mov     z_last, 2
        ret                                

;==============================================================================

f_prob	db      10,13 
        db      'WARNING: There is a problem with one of the files.',10,13
        db      '         Please check the files.',10,13
        db      10,13,'$'

en_low  db      10,13
        db      'WARNING: Three or more protected files are needed',10,13
	db      '         to search for the password. Aborting.',10,13,'$'

ipw     db      10,13,'$'

;------------------------------------------------------------------------------

intro   	db      10,13
        	db      'UNZIP v1.1',10,13 
        	db      'Pkzip password finder.',10,13
        	db      'By Dcypher (dcypher@mhv.net)',10,13
        	db      10,13,'$'

usage   	db      'Usage: UNZIP <zipfile> <wordslist>',10,13
		db      10,13
                db      'Notes: The zipfile must contain atleast 3 password protected files.',10,13
                db      '       The wordslist must use only a <LF> between each word.',10,13
                db      '       A "." will be displayed every 50,000 words checked.',10,13
        	db      10,13, '$'

ui4     	db      '-  encrypted.',10,13,'$'
ui5     	db      '-  not encrypted.',10,13,'$'

ui3             db      10,13
         	db      'Valid Passwords',10,13
		db      '~~~~~~~~~~~~~~~',10,13,'$'

d_finished	db      10,10,13
         	db      'Finished !',10,13
                db      '~~~~~~~~~~',10,13
                db      10,13,'$'

bufA    	db      512 dup(0)
bufB    	db      512 dup(0)
word_dat  	db      0
;==============================================================================

end     begin
