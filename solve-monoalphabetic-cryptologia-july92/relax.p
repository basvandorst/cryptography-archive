(*****************************************************************************) 
(*                         PROGRAM RELAXATION                                *) 
(*****************************************************************************) 
(* A Pascal implementation of probabilistic relaxation to solve mono-        *) 
(* alphabetic substitution ciphers.  A binomial initialization formula is    *) 
(* used with a product rule updating formula.                                *) 
(*      This implementation is described in J. King and D. Bahler, "An Imple-*) 
(* mentation of Probabilistic Relaxation in the Cryptanalysis of Simple Sub- *) 
(* stitution Ciphers", Cryptologia 16(3):215-225 (July 1992).                *) 
(*      A C implementation of this algorithm with the extensions to solve    *) 
(* periodic polyalphabetic substitution ciphers will be available after      *) 
(* publication in Cryptologia.                                               *) 
(*      Language statistics (based on English text without word divisions)   *) 
(* are provided in file INFILER).  The derivation of these statistics is     *) 
(* also described in the paper.                                              *) 
(*      Porting hint: The functions SAFE_MULT, SAFE_DIV, and EXPON handle    *) 
(* underflow and overflow.  These take up approximately 60% of the execution *) 
(* time.  If your machine handles underflow and overflow you can get by      *) 
(* without them.                                                             *) 
(*****************************************************************************) 
(*  (c) Copyright John C. King, 1990   kingj@corp.hp.com                     *) 
(*****************************************************************************) 
(*  INPUT: tri-gram statistics -----| These two files provided with this     *) 
(*         single-gram statistics --| software in file INFILER.              *) 
(*         ciphertext                                                        *) 
(*         plaintext (for testing purposes)                                  *) 
(*  OUTPUT: cipher symbol frequencies                                        *) 
(*         initial estimate matrix P0                                        *) 
(*         during each iteration: mappings and metrics                       *) 
(*****************************************************************************) 
                                                                                
PROGRAM RELAXATION(infiler,outfiler);                                           
  CONST                                                                         
    DATA_DELIM = 9999;               (* delimiter in input data file  *)        
    MAX_LENGTH =  800;               (* max length of cipher          *)        
    MAX_DISTINCT = 26;               (* max distinct cipher symbols   *)        
    SCALE_CONST1 = 1.0L250;          (* used to prevent underflow     *)        
    LOW_LIMIT_EXP = -250;            (*           "  "                *)        
    MAX_TRI_INDEX = 9452;              (* number of trigram statistics  *)      
    MAX_ITERATION= 2;             (* number of iterations *)                    
                                                                                
  TYPE                                                                          
    index_vector = array[65..90] of longreal;                                   
    solution_state  = record              (* a state                       *)   
               c: array[1..MAX_DISTINCT] of char;    (* solution mappings *)    
               freq: array[1..MAX_DISTINCT] of longreal;                        
                         (* freq is prob of solution mapping *)                 
             end;                                                               
    lkvector = record                  (* a likelihood vector         *)        
                 symbol: integer;      (* middle symbol of triple     *)        
                 vector: index_vector; (* likelihood values           *)        
               end;                                                             
    symbol = record      (* cipher symbol and its frequency in cipher *)        
               sym: integer;                                                    
               freq: integer;                                                   
             end;                                                               
    trigram = record     (* trigram statistics *)                               
                ident: integer;   (* a hashing function value *)                
                count: integer;                                                 
              end;                                                              
                                                                                
  VAR                                                                           
    tri_cnt: array[65..90,65..90,65..90] of integer;  (* trigram stats *)       
    num_dist: integer;        (* number of distinct symbols in cipher text *)   
    num_sym: integer;         (* number of symbols in cipher text          *)   
    tri_cnt_sum: real;         (* sum of trigram counts                     *)  
    ctext: packed array[1..MAX_LENGTH] of integer;  (* cipher              *)   
    ptext: array[1..MAX_LENGTH] of char;  (* for testing *)                     
    cdist: array[1..MAX_DISTINCT] of symbol; (* distinct cipher symbols    *)   
                                             (* sorted in descending order *)   
    sing_prob: index_vector;                 (* singe-gram probabilities   *)   
    infiler,outfiler: text;                    (* input,output files         *) 
    view_state: solution_state;         (* used to view state after   *)        
                                             (* each iteration             *)   
    P: array[1..MAX_DISTINCT] of index_vector; (* P matrix                 *)   
    P_Prime: array[1..MAX_DISTINCT] of index_vector; (* "working" P matrix *)   
    key: array[1..MAX_DISTINCT] of char;  (* actual mappings *)                 
                                                                                
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE CHECK_INIT                              *) 
(*****************************************************************************) 
PROCEDURE CHECK_INIT;                                                           
 (* Output results from PROCEDURE INPUT_PLAIN_TEXT *)                           
 VAR                                                                            
    i: integer;                                                                 
 BEGIN                                                                          
   writeln(outfiler);  writeln(outfiler,'     INPUT CIPHER:');                  
   writeln(outfiler,'     ------------');                                       
   writeln(outfiler,'     num_sym=',num_sym:5);                                 
   writeln(outfiler,'     num_dist=',num_dist:5);                               
   writeln(outfiler,'     rank  sym freq  r-freq key');                         
   writeln(outfiler,'     ---- ---- ---- ------- ---');                         
   FOR i:=1 to num_dist DO                                                      
     BEGIN                                                                      
       write(outfiler,i:8,cdist[i].sym:6, cdist[i].freq:5);                     
       writeln(outfiler,(cdist[i].freq/num_sym):8:5,key[i]:4);                  
     END;                                                                       
  END; (* PROCEDURE CHECK_INIT *)                                               
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE CHECK_P                                 *) 
(*****************************************************************************) 
PROCEDURE CHECK_P;                                                              
 (* Output P matrix *)                                                          
  VAR done : boolean;                                                           
      index1,index2,i,j: integer;                                               
  BEGIN                                                                         
    writeln(outfiler);                                                          
    writeln(outfiler,'          P MATRIX:');                                    
    write(outfiler,'         ------------------------------------------------');
    writeln(outfiler,'-------------');                                          
    index1:=1; index2:=9; done:=false;                                          
    REPEAT                                                                      
      write(outfiler,'            ');                                           
      FOR i:=index1 to index2 DO                                                
        write(outfiler,cdist[i].sym:5,'=',key[i]:1);                            
      writeln(outfiler);                                                        
      FOR i:=65 to 90 DO                                                        
        BEGIN                                                                   
          write(outfiler,'          ',chr(i):1,'  ');                           
          FOR j:=index1 to index2 DO write(outfiler,P[j,i]:7:4);                
          writeln(outfiler);                                                    
        END;                                                                    
      writeln(outfiler); writeln(outfiler);                                     
      IF (index2 = num_dist) THEN done:=true                                    
                             ELSE BEGIN                                         
                                    index1:=index1 + 9;                         
                                    index2:=index2 + 9;                         
                                    IF (index2 > num_dist)                      
                                      THEN index2:= num_dist;                   
                                  END;                                          
    UNTIL(done);                                                                
  END; (* PROCEDURE CHECK_P *)                                                  
                                                                                
(*****************************************************************************) 
(*                             FUNCTION VOWEL                                *) 
(*****************************************************************************) 
FUNCTION VOWEL(symbol: char):boolean;                                           
(* Return true if symbol is a vowel *)                                          
BEGIN                                                                           
  IF ((symbol='A') OR (symbol='E') OR (symbol='I') OR (symbol='O') OR           
      (symbol='U') OR (symbol='Y'))                                             
    THEN VOWEL:=true                                                            
    ELSE VOWEL:=false;                                                          
END; (* FUNCTION VOWEL *)                                                       
                                                                                
(*****************************************************************************) 
(*                           FUNCTION CONSONANT                              *) 
(*****************************************************************************) 
FUNCTION CONSONANT(symbol: char): boolean;                                      
(* Return true if symbolis a consonant *)                                       
BEGIN                                                                           
  IF (not VOWEL(symbol))                                                        
    THEN CONSONANT:=true                                                        
    ELSE CONSONANT:=false;                                                      
END; (* FUNCTION CONSONANT *)                                                   
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE CHECK_STATE                             *) 
(*****************************************************************************) 
  PROCEDURE CHECK_STATE(state: solution_state);                                 
   (* Output a state *)                                                         
    VAR i,correct: integer;                                                     
        correct_vc_freq,correct_freq,sum_prob: longreal;                        
    BEGIN                                                                       
      writeln(outfiler,'----------------------------------------------');       
      writeln(outfiler,'rank  sym key prob map');                               
      writeln(outfiler,'---- ---- --- ---- ---');                               
      correct:=0;                                                               
      correct_freq:=0.0;                                                        
      correct_vc_freq:=0.0;                                                     
      sum_prob:=0.0;                                                            
      FOR i:=1 to num_dist DO                                                   
        BEGIN                                                                   
          sum_prob:=sum_prob+state.freq[i];                                     
          write(outfiler,i:3,cdist[i].sym:5,key[i]:3);                          
          write(outfiler,state.freq[i]:6:2,state.c[i]:3);                       
          IF (state.c[i]=key[i])                                                
            THEN BEGIN                                                          
                   correct:=correct+1;                                          
                   correct_freq:=correct_freq+cdist[i].freq;                    
                   correct_vc_freq:=correct_vc_freq+cdist[i].freq;              
                   writeln(outfiler);                                           
                 END                                                            
            ELSE IF (state.c[i]<>key[i])                                        
                   THEN BEGIN                                                   
                          IF ((VOWEL(state.c[i]) AND VOWEL(key[i])) OR          
                             (CONSONANT(state.c[i]) AND CONSONANT(key[i])))     
                            THEN correct_vc_freq:=correct_vc_freq+cdist[i].freq;
                          writeln(outfiler,' WRONG');                           
                         END;                                                   
         END;                                                                   
      writeln(outfiler);                                                        
      writeln(outfiler,'CORRECT MAPPINGS=',correct:4);                          
      writeln(outfiler,'WRONG MAPPINGS=',(num_dist-correct):6);                 
      writeln(outfiler,'AVG. PROB=',(sum_prob/num_dist):7:4);                   
      writeln(outfiler,'CORRECT REL. FREQ=  ',(correct_freq/num_sym):7:2);      
(*    writeln(outfiler,'INCORECT REL. FREQ=',(1-(correct_freq/num_sym)):5:2); *)
      writeln(outfiler,'CORRECT V/C RES.=  ',(correct_vc_freq/num_sym):7:2);    
(* writeln(outfiler,'INCORRECT V/C RES.=',(1-(correct_vc_freq/num_sym)):5:2); *)
    END; (* PROCEDURE CHECK_STATE *)                                            
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE INPUT_CIPHER                            *) 
(*****************************************************************************) 
PROCEDURE INPUT_CIPHER;                                                         
  (* Input and process cipher *)                                                
VAR                                                                             
    symbol,index,i,j,save1,save2,max,maxindex: integer;                         
    found: boolean;                                                             
  BEGIN                                                                         
  (* input cipher and count distinct symbols *)                                 
    num_dist:=0;  num_sym:=0;                                                   
    REPEAT                                                                      
      readln(infiler,symbol);                                                   
      IF (symbol<>DATA_DELIM) THEN BEGIN                                        
                                     num_sym:=num_sym + 1;                      
                                     ctext[num_sym]:=symbol;                    
                                   END;                                         
      IF (num_dist=0)                                                           
        THEN BEGIN (* add first symbol *)                                       
               num_dist:=num_dist + 1;                                          
               cdist[num_dist].sym:=symbol; cdist[num_dist].freq:=1;            
             END                                                                
        ELSE BEGIN (* find symbol and update count *)                           
               index:=0; found:=false;                                          
               REPEAT                                                           
                 index:=index + 1;                                              
                 IF (symbol=cdist[index].sym)                                   
                   THEN BEGIN                                                   
                          found:=true; cdist[index].freq:=cdist[index].freq + 1;
                        END;                                                    
               UNTIL (found or (index=num_dist));                               
               IF ((not found) AND (symbol<>DATA_DELIM))                        
                 THEN BEGIN  (* add symbol *)                                   
                        num_dist:=num_dist + 1;                                 
                        cdist[num_dist].sym:=symbol; cdist[num_dist].freq:=1;   
                      END;                                                      
             END; (* ELSE *)                                                    
    UNTIL (symbol=DATA_DELIM);                                                  
  (* sort distinct symbols *)                                                   
    FOR i:=1 to (num_dist-1) DO                                                 
      BEGIN                                                                     
        save1:=cdist[i].sym; save2:=cdist[i].freq; max:=save2; maxindex:=i;     
        FOR j:= (i+1) to num_dist DO                                            
          IF (cdist[j].freq>max) THEN BEGIN                                     
                                     max:=cdist[j].freq; maxindex:=j;           
                                   END;                                         
        cdist[i].sym:=cdist[maxindex].sym; cdist[i].freq:=max;                  
        cdist[maxindex].sym:=save1; cdist[maxindex].freq:=save2;                
      END;                                                                      
  END; (* PROCEDURE INPUT_CIPHER *)                                             
                                                                                
(*****************************************************************************) 
(*                         FUNCTION EXPON                                    *) 
(*****************************************************************************) 
FUNCTION EXPON(x: longreal): integer;                                           
 (* Return exponent of x *)                                                     
  VAR count: integer;                                                           
  BEGIN                                                                         
    count:=0;                                                                   
    IF ((x<1.0) AND (x>0.0))                                                    
      THEN REPEAT                                                               
             count:=count-1;                                                    
             x:=x*10.0;                                                         
           UNTIL (x>=1.0)                                                       
      ELSE IF (x>=10.0)                                                         
             THEN REPEAT                                                        
                    count:=count+1;                                             
                    x:=x/10.0;                                                  
                  UNTIL (x<10.0);                                               
    EXPON:=count;                                                               
  END; (* FUNCTION EXPON *)                                                     
                                                                                
(*****************************************************************************) 
(*                         FUNCTION SAFE_MULT                                *) 
(*****************************************************************************) 
FUNCTION SAFE_MULT(x,y: longreal): longreal;                                    
 (* Return x*y or 0.0 if underflow *)                                           
  BEGIN                                                                         
    IF ((EXPON(x)+EXPON(y)) < LOW_LIMIT_EXP)                                    
      THEN SAFE_MULT:=0.0                                                       
      ELSE SAFE_MULT:=x*y;                                                      
  END; (* FUNCTION SAFE_MULT *)                                                 
                                                                                
(*****************************************************************************) 
(*                         FUNCTION SAFE_DIV                                 *) 
(*****************************************************************************) 
FUNCTION SAFE_DIV(x,y: longreal): longreal;                                     
 (* Return x/y or 0.0 if underflow *)                                           
  BEGIN                                                                         
    IF ((EXPON(x)-EXPON(y)) < LOW_LIMIT_EXP)                                    
      THEN SAFE_DIV:=0.0                                                        
      ELSE SAFE_DIV:=x/y;                                                       
  END; (* FUNCTION SAFE_DIV *)                                                  
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE INPUT_LANGUAGE_STATS                    *) 
(*****************************************************************************) 
PROCEDURE INPUT_LANGUAGE_STATS;                                                 
  (* Input and process language statistics *)                                   
  VAR                                                                           
    i,a,b,c,count,sing_cnt_sum: integer;                                        
    l1,l2,l3: char;                                                             
  BEGIN                                                                         
    (* input and process trigram counts *)                                      
    reset(infiler); rewrite(outfiler);                                          
    FOR a:=65 to 90 DO                                                          
      FOR b:=65 to 90 DO                                                        
        FOR c:=65 to 90 DO                                                      
          tri_cnt[a,b,c]:=0;                                                    
    tri_cnt_sum:=0.0;                                                           
    FOR i:=1 to MAX_TRI_INDEX DO                                                
      BEGIN                                                                     
        readln(infiler,l1,l2,l3,count);                                         
        tri_cnt_sum:=tri_cnt_sum + count;                                       
        tri_cnt[ord(l1),ord(l2),ord(l3)]:=count;                                
      END;                                                                      
    (* input and process monogram counts *)                                     
    sing_cnt_sum:=0;                                                            
    FOR i:=65 to 90 DO                                                          
      BEGIN                                                                     
        readln(infiler,count);                                                  
        sing_cnt_sum:= sing_cnt_sum + count;                                    
        sing_prob[i]:=count;                                                    
      END;                                                                      
    FOR i:=65 to 90 DO sing_prob[i]:=sing_prob[i]/sing_cnt_sum;                 
END; (* PROCEDURE INPUT_LANGUAGE_STATS *)                                       
                                                                                
                                                                                
(*****************************************************************************) 
(*               PROCEDURE INPUT_PLAIN_TEXT                       *)            
(*****************************************************************************) 
PROCEDURE INPUT_PLAIN_TEXT;                                                     
  (* Initialize solution data structures *)                                     
  VAR                                                                           
    i,index: integer;                                                           
  BEGIN                                                                         
  (* compute correct mappings for testing *)                                    
    FOR i:=1 to num_sym DO readln(infiler,ptext[i]);                            
    FOR i:=1 to num_dist DO                                                     
      BEGIN                                                                     
        index:=0;                                                               
        REPEAT                                                                  
          index:=index+1;                                                       
        UNTIL(ctext[index]=cdist[i].sym);                                       
        key[i]:=ptext[index];                                                   
      END;                                                                      
  END; (* PROCEDURE INPUT_PLAIN_TEXT *)                                         
                                                                                
(*****************************************************************************) 
(*                   PROCEDURE INIT_P_MATRIX                                  *)
(*****************************************************************************) 
PROCEDURE INIT_P_MATRIX;                                                        
 (* Set P to initial estimates using binomial formula *)                        
  VAR pr,sum,prod: longreal;                                                    
      i,j,k,y: integer;                                                         
  BEGIN                                                                         
    FOR i:=1 to num_dist DO                                                     
      BEGIN                                                                     
        y:=cdist[i].freq;                                                       
        sum:=0.0;                                                               
        FOR j:=65 to 90 DO                                                      
          BEGIN                                                                 
            pr:=sing_prob[j];                                                   
            prod:=SCALE_CONST1;                                                 
            FOR k:=1 to (num_sym-y) DO                                          
                prod:=prod*(1-pr);                                              
            FOR k:=1 to y DO                                                    
                prod:=SAFE_MULT(prod,pr);                                       
            P[i,j]:=prod;                                                       
            sum:=sum+P[i,j];                                                    
          END;                                                                  
       (* normalize column *)                                                   
        IF (sum <> 0)                                                           
          THEN FOR j:=65 to 90 DO                                               
                 P[i,j]:=SAFE_DIV(P[i,j],sum)                                   
          ELSE BEGIN                                                            
                 writeln(outfiler,'PROCEDURE INIT_P_MATRIX: COLUMN SUM=0');     
                 halt;                                                          
               END;                                                             
      END; (* column *)                                                         
  END; (* PROCEDURE INIT_P_MATRIX *)                                            
                                                                                
(*****************************************************************************) 
(*                   PROCEDURE INIT_P_PRIME_MATRIX                            *)
(*****************************************************************************) 
PROCEDURE INIT_P_PRIME_MATRIX;                                                  
(* Initialize P_Prime matrix before relaxation with product rule updating *)    
  VAR                                                                           
    i,j: integer;                                                               
  BEGIN                                                                         
    FOR i:=1 to num_dist DO                                                     
      FOR j:=65 to 90 DO                                                        
        P_Prime[i,j]:=P[i,j];                                                   
  END; (* PROCEDURE INIT_P_PRIME_MATRIX *)                                      
                                                                                
(*****************************************************************************) 
(*                         FUNCTION SYMBOL_INDEX                             *) 
(*****************************************************************************) 
FUNCTION SYMBOL_INDEX(symbol: integer): integer;                                
 (* return rank number of distinct symbol *)                                    
 VAR                                                                            
   i: integer;                                                                  
   BEGIN                                                                        
     i:=0;                                                                      
     REPEAT i:=i+1; UNTIL (cdist[i].sym=symbol);                                
     SYMBOL_INDEX:=i;                                                           
   END; (* FUNCTION SYMBOL_INDEX *)                                             
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE COMPUTE_LV                              *) 
(*****************************************************************************) 
PROCEDURE COMPUTE_LV(var lv: lkvector; sym_index: integer);                     
  (* compute likelihood vector for trigram number 'index' *)                    
  VAR                                                                           
    U_index,W_index,a,b,c: integer;                                             
    t_cnt,suma,sumc,r1,r2: longreal;                                            
  BEGIN                                                                         
    (* find P column that represents U & W *)                                   
    U_index:=SYMBOL_INDEX(ctext[sym_index]);                                    
    W_index:=SYMBOL_INDEX(ctext[sym_index+2]);                                  
    (* triple loop to calculate likelihood vector *)                            
    FOR b:=65 to 90 DO                                                          
      BEGIN (* b loop *)                                                        
        suma:=0.0;                                                              
        FOR a:=65 to 90 DO                                                      
          BEGIN (* a loop *)                                                    
            sumc:=0.0;                                                          
            FOR c:=65 to 90 DO                                                  
              BEGIN (* c loop *)                                                
                t_cnt:=tri_cnt[a,b,c];                                          
                IF (t_cnt<>0)                                                   
                  THEN BEGIN                                                    
                         r1:=1.0/(sing_prob[a]*sing_prob[b]*sing_prob[c]);      
                         r2:=t_cnt/tri_cnt_sum;                                 
                         sumc:=sumc+SAFE_MULT(SAFE_MULT(P[W_index,c],r1),r2);   
                       END;                                                     
              END; (* c loop *)                                                 
            suma:=suma+SAFE_MULT(P[U_index,a],sumc);                            
          END; (* a loop*)                                                      
        lv.vector[b]:=suma;                                                     
      END; (* b loop *)                                                         
    lv.symbol:=ctext[sym_index+1];                                              
  END; (* PROCEDURE COMPUTE_LV *)                                               
                                                                                
                                                                                
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE UPDATE                                  *) 
(*****************************************************************************) 
PROCEDURE UPDATE(lk: lkvector);                                                 
(* update P_Prime using the newly computed likelihood vector lk *)              
  VAR                                                                           
    i,j: integer;                                                               
    sum: longreal;                                                              
  BEGIN                                                                         
    i:=SYMBOL_INDEX(lk.symbol);                                                 
    sum:=0.0;                                                                   
    FOR j:=65 to 90 DO                                                          
      BEGIN                                                                     
        P_Prime[i,j]:=SAFE_MULT(P_Prime[i,j],lk.vector[j]);                     
        sum:=sum+P_Prime[i,j];                                                  
      END;                                                                      
    (* Normalize vector after each update for product rule *)                   
    IF (sum<>0)                                                                 
      THEN FOR j:=65 to 90 DO                                                   
             P_Prime[i,j]:=SAFE_DIV(P_Prime[i,j],sum)                           
      ELSE BEGIN                                                                
             writeln(outfiler,'PROCEDURE UPDATE: COLUMN SUM=0');                
             halt;                                                              
           END;                                                                 
  END; (* PROCEDURE UPDATE *)                                                   
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE GET_MAPPINGS                            *) 
(*****************************************************************************) 
PROCEDURE GET_MAPPINGS(var state: solution_state);                              
 (* get mappings resulting from an iteration of relaxation algorithm *)         
  VAR                                                                           
    i,j,maxindex: integer;                                                      
    max: longreal;                                                              
  BEGIN                                                                         
    FOR i:= 1 to num_dist DO                                                    
      BEGIN                                                                     
        max:=0.0; maxindex:=0;                                                  
        FOR j:=65 to 90 DO                                                      
          IF (P[i,j] > max) THEN BEGIN max:=P[i,j]; maxindex:=j; END;           
        state.freq[i]:=max;                                                     
        IF (max=0.0)                                                            
          THEN state.c[i]:='-'                                                  
          ELSE state.c[i]:=chr(maxindex);                                       
      END;                                                                      
  END; (* PROCEDURE GET_MAPPINGS *)                                             
                                                                                
(*****************************************************************************) 
(*                       PROCEDURE TRANSFER_P_PRIME                        *)   
(*****************************************************************************) 
PROCEDURE TRANSFER_P_PRIME;                                                     
 (* move P_Prime to P *)                                                        
  VAR                                                                           
    i,j: integer;                                                               
  BEGIN                                                                         
    FOR i:=1 to num_dist DO                                                     
      FOR j:=65 to 90 DO P[i,j]:=P_Prime[i,j];                                  
  END; (* PROCEDURE TRANSFER_P_PRIME *)                                         
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE OUTPUT_ITERATION                        *) 
(*****************************************************************************) 
PROCEDURE OUTPUT_ITERATION(iteration: integer);                                 
(* output the result of an iteration of the relaxation algorithm *)             
  BEGIN                                                                         
    GET_MAPPINGS(view_state);                                                   
    writeln(outfiler,'******************************************************'); 
    writeln(outfiler,'ITERATION=',iteration:3);                                 
    writeln(outfiler);                                                          
    CHECK_STATE(view_state);                                                    
  END; (* PROCEDURE OUTPUT_ITERATION *)                                         
                                                                                
(*****************************************************************************) 
(*                         PROCEDURE RELAXATION                              *) 
(*****************************************************************************) 
PROCEDURE RELAXATION;                                                           
(* Perform relaxation algorithm *)                                              
  VAR                                                                           
    i,iteration,trigram: integer;                                               
    likelihood: lkvector;                                                       
  BEGIN                                                                         
    (* initialize likelihood vector to be used in relaxation *)                 
    likelihood.symbol:=0;                                                       
    FOR i:=65 to 90 DO likelihood.vector[i]:=0.0;                               
                                                                                
    (* outer loop of relaxation algorithm *)                                    
    iteration:=0;                                                               
    OUTPUT_ITERATION(iteration);                                                
    REPEAT                                                                      
      iteration:=iteration+1;                                                   
      FOR trigram:=1 to (num_sym-2) DO                                          
        BEGIN                                                                   
          COMPUTE_LV(likelihood,trigram);                                       
          UPDATE(likelihood);                                                   
        END;                                                                    
      (* transfer new results from P_Prime to P for next iteration *)           
      TRANSFER_P_PRIME;                                                         
      OUTPUT_ITERATION(iteration);                                              
    UNTIL (iteration = MAX_ITERATION);                                          
  END; (* PROCEDURE RELAXATION *)                                               
                                                                                
                                                                                
(*****************************************************************************) 
(*                              MAIN PROGRAM                                 *) 
(*****************************************************************************) 
BEGIN (* MAIN PROGRAM *)                                                        
    INPUT_LANGUAGE_STATS;                                                       
    INPUT_CIPHER;                                                               
    INPUT_PLAIN_TEXT;                                                           
    INIT_P_MATRIX;                                                              
    INIT_P_PRIME_MATRIX;                                                        
    RELAXATION;                                                                 
END.  (* MAIN PROGRAM *)                                                        
                                                                                
