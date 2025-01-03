@ECHO OFF
CLS
ECHO                       LEARNING THE DEA COMMAND LINE
echo ��
echo ��
ECHO EXAMPLE 1 ENCRYPTION
echo ��
echo ��
ECHO WE WILL ENCRYPT THE FILE "THOUGHT.TXT" AND LET THE DEA
ECHO HANDLE THE OUTPUT FILE NAME FOR US
ECHO WE WILL GIVE THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /E /K1 /OTP2000 THOUGHT.TXT��
echo ��
echo ��
PAUSE
CLS
DEA /E /K1 /OTP2000 THOUGHT.TXT
echo ��
echo ��
PAUSE
CLS
ECHO EXAMPLE 1 DECRYPTION
echo ��
echo ��
ECHO NOW WE WILL DECRYPT THE FILE "THOUGHT.DEA"
ECHO BY GIVING THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /D /K1 /OTP2000 THOUGHT.DEA
echo ��
ECHO NOTE THAT WE MUST SPECIFY THE FULL FILE NAME PLUS ANY EXTENSION
ECHO IF THERE IS ONE WHEN WE SELECT /Decrypt MODE
echo ��
echo ��
PAUSE
DEA /D /K1 /OTP2000 THOUGHT.DEA
echo ��
echo ��
ECHO NOW LET'S SEE IF THE DEA CORRECTLY RECOVERED THE FILE
PAUSE
CLS
TYPE THOUGHT.TXT
echo ��
echo ��
PAUSE
@ECHO OFF
CLS
ECHO                       LEARNING THE DEA COMMAND LINE
echo ��
echo ��
ECHO EXAMPLE 2 ENCRYPTION
echo ��
echo ��
ECHO WE WILL ENCRYPT THE FILE "THOUGHT.TXT" AND SPECIFY OUR OWN
ECHO OUTPUT FILE NAME
ECHO WE WILL GIVE THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /E /K1 /OTP2000 THOUGHT.TXT NELLIS.XYZ��
echo ��
echo ��
PAUSE
CLS
DEA /E /K1 /OTP2000 THOUGHT.TXT  NELLIS.XYZ
echo ��
echo ��
PAUSE
CLS
ECHO EXAMPLE 2 DECRYPTION
echo ��
echo ��
ECHO NOW WE WILL DECRYPT THE FILE "NELLIS.XYZ"
ECHO BY GIVING THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /D /K1 /OTP2000 NELLIS.XYZ
echo ��
ECHO NOTE THAT WE MUST SPECIFY THE FULL FILE NAME PLUS ANY EXTENSION
ECHO IF THERE IS ONE WHEN WE SELECT /Decrypt MODE
echo ��
echo ��
PAUSE
DEA /D /K1 /OTP2000 NELLIS.XYZ
echo ��
echo ��
ECHO NOW LET'S SEE IF THE DEA CORRECTLY RECOVERED THE FILE
PAUSE
CLS
TYPE THOUGHT.TXT
echo ��
echo ��
PAUSE
@ECHO OFF
CLS
ECHO                       LEARNING THE DEA COMMAND LINE
echo ��
echo ��
ECHO EXAMPLE 3 ENCRYPTION
echo ��
echo ��
ECHO WE WILL ENCRYPT THE FILE "THOUGHT.TXT" AND SPECIFY OUR OWN
ECHO OUTPUT FILE NAME WITH NO FILE EXTENSION
ECHO WE WILL GIVE THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /E /K1 /OTP2179 THOUGHT.TXT TESTING��
echo ��
echo ��
PAUSE
CLS
DEA /E /K1 /OTP2179 THOUGHT.TXT  TESTING
echo ��
echo ��
PAUSE
CLS
ECHO EXAMPLE 3 DECRYPTION
echo ��
echo ��
ECHO NOW WE WILL DECRYPT THE FILE "TESTING"
ECHO BY GIVING THE FOLLOWING DEA COMMAND LINE:
echo ��
ECHO DEA /D /K1 /OTP2179 TESTING
echo ��
ECHO NOTE THAT AS THE FILE "TESTING" HAS NO EXTENSION, WE DO NOT NEED
ECHO TO SPECIFY IT THIS TIME
echo ��
echo ��
PAUSE
DEA /D /K1 /OTP2179 TESTING
echo ��
echo ��
ECHO NOW LET'S SEE IF THE DEA CORRECTLY RECOVERED THE FILE
PAUSE
CLS
TYPE THOUGHT.TXT
echo ��
echo ��
echo ��
ECHO *** ALWAYS BE SURE TO USE THE SAME KEY ID NUMBER AND SAME /OTP[n] VALUE
ECHO *** FOR BOTH ENCRYPTION AND DECRYPTION
echo ��
echo ��
ECHO IF YOU INTEND TO USE THE DOS REDIRECTION PIPE, IT MUST BE THE LAST
ECHO COMMAND ON THE DEA COMMAND LINE
echo ��
echo ��
ECHO NOW, LET'S HAVE A LOOK AT THE CONTENTS OF THE DEA.LOG FILE WHICH HAS
ECHO RECORDED EVERYTHING WE HAVE JUST DONE.  WE WILL USE THE DOS 'TYPE' COMMAND
ECHO IT WILL SCROLL BY VERY QUICKLY, SO PRESS THE PAUSE KEY
ECHO TO STOP IT.  WHEN READY, PRESS ANY OTHER KEY.
echo ��
echo ��
PAUSE
CLS
ECHO LISTING CONTENTS OF THE DEA.LOG FILE
echo ��
echo ��
echo ��
PAUSE
CLS
TYPE DEA.LOG
echo ��
echo ��
