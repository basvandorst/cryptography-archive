/* Generated By: Maker.jj,v 1.3 1997/11/22 07&JavaCC: Do not edit this line. MakerTokenManager.java */
package cryptix.util.test;
import cryptix.test.BaseTest;
import cryptix.util.core.ArrayUtil;
import cryptix.util.core.Hex;
import cryptix.provider.key.RawSecretKey;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.security.Cipher;
import java.security.FeedbackCipher;
import java.security.MessageDigest;
import java.security.Mode;
import java.security.Parameterized;
import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;

public class MakerTokenManager implements MakerConstants
{
private final int jjStopAtPos(int pos, int kind)
{
   jjmatchedKind = kind;
   jjmatchedPos = pos;
   return pos + 1;
}
private final int jjMoveStringLiteralDfa0_1()
{
   switch(curChar)
   {
      case 62:
         return jjStopAtPos(0, 18);
      default :
         return 1;
   }
}
private final int jjStopStringLiteralDfa_0(int pos, long active0)
{
   switch (pos)
   {
      case 0:
         if ((active0 & 0x1000L) != 0L)
         {
            jjmatchedKind = 21;
            return 23;
         }
         if ((active0 & 0x1ed80L) != 0L)
         {
            jjmatchedKind = 21;
            return 9;
         }
         return -1;
      case 1:
         if ((active0 & 0x2080L) != 0L)
            return 9;
         if ((active0 & 0x1dd00L) != 0L)
         {
            jjmatchedKind = 21;
            jjmatchedPos = 1;
            return 9;
         }
         return -1;
      case 2:
         if ((active0 & 0x1dd00L) != 0L)
         {
            jjmatchedKind = 21;
            jjmatchedPos = 2;
            return 9;
         }
         return -1;
      case 3:
         if ((active0 & 0x19100L) != 0L)
         {
            jjmatchedKind = 21;
            jjmatchedPos = 3;
            return 9;
         }
         if ((active0 & 0x4c00L) != 0L)
            return 9;
         return -1;
      case 4:
         if ((active0 & 0x8000L) != 0L)
            return 9;
         if ((active0 & 0x11100L) != 0L)
         {
            jjmatchedKind = 21;
            jjmatchedPos = 4;
            return 9;
         }
         return -1;
      case 5:
         if ((active0 & 0x11000L) != 0L)
         {
            jjmatchedKind = 21;
            jjmatchedPos = 5;
            return 9;
         }
         if ((active0 & 0x100L) != 0L)
            return 9;
         return -1;
      default :
         return -1;
   }
}
private final int jjStartNfa_0(int pos, long active0)
{
   return jjMoveNfa_0(jjStopStringLiteralDfa_0(pos, active0), pos + 1);
}
private final int jjStartNfaWithStates_0(int pos, int kind, int state)
{
   jjmatchedKind = kind;
   jjmatchedPos = pos;
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) { return pos + 1; }
   return jjMoveNfa_0(state, pos + 1);
}
private final int jjMoveStringLiteralDfa0_0()
{
   switch(curChar)
   {
      case 42:
         return jjStopAtPos(0, 26);
      case 60:
         return jjStopAtPos(0, 17);
      case 61:
         return jjStopAtPos(0, 27);
      case 97:
         return jjMoveStringLiteralDfa1_0(0x4000L);
      case 99:
         return jjMoveStringLiteralDfa1_0(0x100L);
      case 100:
         return jjMoveStringLiteralDfa1_0(0x400L);
      case 105:
         return jjMoveStringLiteralDfa1_0(0x12000L);
      case 109:
         return jjMoveStringLiteralDfa1_0(0x880L);
      case 112:
         return jjMoveStringLiteralDfa1_0(0x1000L);
      case 115:
         return jjMoveStringLiteralDfa1_0(0x8000L);
      default :
         return jjMoveNfa_0(5, 0);
   }
}
private final int jjMoveStringLiteralDfa1_0(long active0)
{
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(0, active0);
      return 1;
   }
   switch(curChar)
   {
      case 97:
         return jjMoveStringLiteralDfa2_0(active0, 0x1400L);
      case 100:
         if ((active0 & 0x80L) != 0L)
            return jjStartNfaWithStates_0(1, 7, 9);
         break;
      case 105:
         return jjMoveStringLiteralDfa2_0(active0, 0x100L);
      case 110:
         return jjMoveStringLiteralDfa2_0(active0, 0x10000L);
      case 111:
         return jjMoveStringLiteralDfa2_0(active0, 0x800L);
      case 112:
         return jjMoveStringLiteralDfa2_0(active0, 0x8000L);
      case 117:
         return jjMoveStringLiteralDfa2_0(active0, 0x4000L);
      case 118:
         if ((active0 & 0x2000L) != 0L)
            return jjStartNfaWithStates_0(1, 13, 9);
         break;
      default :
         break;
   }
   return jjStartNfa_0(0, active0);
}
private final int jjMoveStringLiteralDfa2_0(long old0, long active0)
{
   if (((active0 &= old0)) == 0L)
      return jjStartNfa_0(0, old0); 
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(1, active0);
      return 2;
   }
   switch(curChar)
   {
      case 99:
         return jjMoveStringLiteralDfa3_0(active0, 0x10000L);
      case 100:
         return jjMoveStringLiteralDfa3_0(active0, 0x1800L);
      case 101:
         return jjMoveStringLiteralDfa3_0(active0, 0x8000L);
      case 112:
         return jjMoveStringLiteralDfa3_0(active0, 0x100L);
      case 116:
         return jjMoveStringLiteralDfa3_0(active0, 0x4400L);
      default :
         break;
   }
   return jjStartNfa_0(1, active0);
}
private final int jjMoveStringLiteralDfa3_0(long old0, long active0)
{
   if (((active0 &= old0)) == 0L)
      return jjStartNfa_0(1, old0); 
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(2, active0);
      return 3;
   }
   switch(curChar)
   {
      case 97:
         if ((active0 & 0x400L) != 0L)
            return jjStartNfaWithStates_0(3, 10, 9);
         break;
      case 100:
         return jjMoveStringLiteralDfa4_0(active0, 0x1000L);
      case 101:
         if ((active0 & 0x800L) != 0L)
            return jjStartNfaWithStates_0(3, 11, 9);
         return jjMoveStringLiteralDfa4_0(active0, 0x8000L);
      case 104:
         return jjMoveStringLiteralDfa4_0(active0, 0x100L);
      case 108:
         return jjMoveStringLiteralDfa4_0(active0, 0x10000L);
      case 111:
         if ((active0 & 0x4000L) != 0L)
            return jjStartNfaWithStates_0(3, 14, 9);
         break;
      default :
         break;
   }
   return jjStartNfa_0(2, active0);
}
private final int jjMoveStringLiteralDfa4_0(long old0, long active0)
{
   if (((active0 &= old0)) == 0L)
      return jjStartNfa_0(2, old0); 
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(3, active0);
      return 4;
   }
   switch(curChar)
   {
      case 100:
         if ((active0 & 0x8000L) != 0L)
            return jjStartNfaWithStates_0(4, 15, 9);
         break;
      case 101:
         return jjMoveStringLiteralDfa5_0(active0, 0x100L);
      case 105:
         return jjMoveStringLiteralDfa5_0(active0, 0x1000L);
      case 117:
         return jjMoveStringLiteralDfa5_0(active0, 0x10000L);
      default :
         break;
   }
   return jjStartNfa_0(3, active0);
}
private final int jjMoveStringLiteralDfa5_0(long old0, long active0)
{
   if (((active0 &= old0)) == 0L)
      return jjStartNfa_0(3, old0); 
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(4, active0);
      return 5;
   }
   switch(curChar)
   {
      case 100:
         return jjMoveStringLiteralDfa6_0(active0, 0x10000L);
      case 110:
         return jjMoveStringLiteralDfa6_0(active0, 0x1000L);
      case 114:
         if ((active0 & 0x100L) != 0L)
            return jjStartNfaWithStates_0(5, 8, 9);
         break;
      default :
         break;
   }
   return jjStartNfa_0(4, active0);
}
private final int jjMoveStringLiteralDfa6_0(long old0, long active0)
{
   if (((active0 &= old0)) == 0L)
      return jjStartNfa_0(4, old0); 
   try { curChar = input_stream.readChar(); }
   catch(java.io.IOException e) {
      jjStopStringLiteralDfa_0(5, active0);
      return 6;
   }
   switch(curChar)
   {
      case 101:
         if ((active0 & 0x10000L) != 0L)
            return jjStartNfaWithStates_0(6, 16, 9);
         break;
      case 103:
         if ((active0 & 0x1000L) != 0L)
            return jjStartNfaWithStates_0(6, 12, 9);
         break;
      default :
         break;
   }
   return jjStartNfa_0(5, active0);
}
private final void jjCheckNAdd(int state)
{
   if (jjrounds[state] != jjround)
   {
      jjstateSet[jjnewStateCnt++] = state;
      jjrounds[state] = jjround;
   }
}
private final void jjAddStates(int start, int end)
{
   do {
      jjstateSet[jjnewStateCnt++] = jjnextStates[start];
   } while (start++ != end);
}
private final void jjCheckNAddTwoStates(int state1, int state2)
{
   jjCheckNAdd(state1);
   jjCheckNAdd(state2);
}
private final void jjCheckNAddStates(int start, int end)
{
   do {
      jjCheckNAdd(jjnextStates[start]);
   } while (start++ != end);
}
private final void jjCheckNAddStates(int start)
{
   jjCheckNAdd(jjnextStates[start]);
   jjCheckNAdd(jjnextStates[start + 1]);
}
static final long[] jjbitVec0 = {
   0x0L, 0x0L, 0xffffffffffffffffL, 0xffffffffffffffffL
};
private final int jjMoveNfa_0(int startState, int curPos)
{
   int[] nextStates;
   int startsAt = 0;
   jjnewStateCnt = 33;
   int i = 1;
   jjstateSet[0] = startState;
   int j, kind = 0x7fffffff;
   for (;;)
   {
      if (++jjround == 0x7fffffff)
         ReInitRounds();
      if (curChar < 64)
      {
         long l = 1L << curChar;
         MatchLoop: do
         {
            switch(jjstateSet[--i])
            {
               case 5:
                  if ((0x3ff000000000000L & l) != 0L)
                  {
                     if (kind > 24)
                        kind = 24;
                     jjCheckNAdd(10);
                  }
                  else if (curChar == 34)
                     jjCheckNAdd(12);
                  else if (curChar == 47)
                     jjstateSet[jjnewStateCnt++] = 0;
                  break;
               case 12:
                  if ((0xfffffffbffffdbffL & l) != 0L)
                     jjCheckNAddStates(0, 2);
                  else if (curChar == 34)
                  {
                     if (kind > 25)
                        kind = 25;
                  }
                  break;
               case 23:
               case 9:
                  if ((0x3ff200800000000L & l) == 0L)
                     break;
                  if (kind > 21)
                     kind = 21;
                  jjCheckNAdd(9);
                  break;
               case 1:
                  if ((0xffffffffffffdbffL & l) != 0L)
                     jjCheckNAddStates(3, 5);
                  else if ((0x2400L & l) != 0L)
                  {
                     if (kind > 6)
                        kind = 6;
                  }
                  if (curChar == 13)
                     jjstateSet[jjnewStateCnt++] = 3;
                  break;
               case 0:
                  if (curChar == 47)
                     jjCheckNAdd(1);
                  break;
               case 3:
                  if (curChar == 10 && kind > 6)
                     kind = 6;
                  break;
               case 7:
                  if ((0x3ff200800000000L & l) == 0L)
                     break;
                  if (kind > 20)
                     kind = 20;
                  jjstateSet[jjnewStateCnt++] = 7;
                  break;
               case 10:
                  if ((0x3ff000000000000L & l) == 0L)
                     break;
                  if (kind > 24)
                     kind = 24;
                  jjCheckNAdd(10);
                  break;
               case 11:
                  if (curChar == 34)
                     jjCheckNAdd(12);
                  break;
               case 14:
                  if ((0x8400000000L & l) != 0L)
                     jjCheckNAddStates(0, 2);
                  break;
               default : break;
            }
         } while(i != startsAt);
      }
      else if (curChar < 128)
      {
         long l = 1L << (curChar & 077);
         MatchLoop: do
         {
            switch(jjstateSet[--i])
            {
               case 5:
                  if ((0x7fffffe07fffffeL & l) != 0L)
                  {
                     if (kind > 21)
                        kind = 21;
                     jjCheckNAdd(9);
                  }
                  if ((0x7fffffeL & l) != 0L)
                  {
                     if (kind > 20)
                        kind = 20;
                     jjCheckNAdd(7);
                  }
                  else if (curChar == 112)
                     jjAddStates(6, 7);
                  break;
               case 12:
                  if ((0xffffffffefffffffL & l) != 0L)
                     jjCheckNAddStates(0, 2);
                  else if (curChar == 92)
                     jjstateSet[jjnewStateCnt++] = 14;
                  break;
               case 23:
                  if ((0x7fffffe07fffffeL & l) != 0L)
                  {
                     if (kind > 21)
                        kind = 21;
                     jjCheckNAdd(9);
                  }
                  if (curChar == 114)
                     jjstateSet[jjnewStateCnt++] = 31;
                  if (curChar == 114)
                     jjstateSet[jjnewStateCnt++] = 22;
                  break;
               case 1:
                  jjCheckNAddStates(3, 5);
                  break;
               case 6:
                  if ((0x7fffffeL & l) == 0L)
                     break;
                  if (kind > 20)
                     kind = 20;
                  jjCheckNAdd(7);
                  break;
               case 7:
                  if ((0x7fffffe07fffffeL & l) == 0L)
                     break;
                  if (kind > 20)
                     kind = 20;
                  jjCheckNAdd(7);
                  break;
               case 8:
                  if ((0x7fffffe07fffffeL & l) == 0L)
                     break;
                  if (kind > 21)
                     kind = 21;
                  jjCheckNAdd(9);
                  break;
               case 9:
                  if ((0x7fffffe07fffffeL & l) == 0L)
                     break;
                  if (kind > 21)
                     kind = 21;
                  jjCheckNAdd(9);
                  break;
               case 14:
                  if ((0x14404410000000L & l) != 0L)
                     jjCheckNAddStates(0, 2);
                  break;
               case 16:
                  if (curChar == 112)
                     jjAddStates(6, 7);
                  break;
               case 17:
                  if (curChar == 121 && kind > 9)
                     kind = 9;
                  break;
               case 18:
                  if (curChar == 116)
                     jjstateSet[jjnewStateCnt++] = 17;
                  break;
               case 19:
                  if (curChar == 114)
                     jjstateSet[jjnewStateCnt++] = 18;
                  break;
               case 20:
                  if (curChar == 101)
                     jjstateSet[jjnewStateCnt++] = 19;
                  break;
               case 21:
                  if (curChar == 112)
                     jjstateSet[jjnewStateCnt++] = 20;
                  break;
               case 22:
                  if (curChar == 111)
                     jjstateSet[jjnewStateCnt++] = 21;
                  break;
               case 24:
                  if (curChar == 115 && kind > 9)
                     kind = 9;
                  break;
               case 25:
                  if (curChar == 101)
                     jjstateSet[jjnewStateCnt++] = 24;
                  break;
               case 26:
                  if (curChar == 105)
                     jjstateSet[jjnewStateCnt++] = 25;
                  break;
               case 27:
                  if (curChar == 116)
                     jjstateSet[jjnewStateCnt++] = 26;
                  break;
               case 28:
                  if (curChar == 114)
                     jjstateSet[jjnewStateCnt++] = 27;
                  break;
               case 29:
                  if (curChar == 101)
                     jjstateSet[jjnewStateCnt++] = 28;
                  break;
               case 30:
                  if (curChar == 112)
                     jjstateSet[jjnewStateCnt++] = 29;
                  break;
               case 31:
                  if (curChar == 111)
                     jjstateSet[jjnewStateCnt++] = 30;
                  break;
               case 32:
                  if (curChar == 114)
                     jjstateSet[jjnewStateCnt++] = 31;
                  break;
               default : break;
            }
         } while(i != startsAt);
      }
      else
      {
         int i2 = (curChar & 0xff) >> 6;
         long l2 = 1L << (curChar & 077);
         MatchLoop: do
         {
            switch(jjstateSet[--i])
            {
               case 12:
                  if ((jjbitVec0[i2] & l2) != 0L)
                     jjCheckNAddStates(0, 2);
                  break;
               case 1:
                  if ((jjbitVec0[i2] & l2) != 0L)
                     jjCheckNAddStates(3, 5);
                  break;
               default : break;
            }
         } while(i != startsAt);
      }
      if (kind != 0x7fffffff)
      {
         jjmatchedKind = kind;
         jjmatchedPos = curPos;
         kind = 0x7fffffff;
      }
      ++curPos;
      if ((i = jjnewStateCnt) == (startsAt = 33 - (jjnewStateCnt = startsAt)))
         return curPos;
      try { curChar = input_stream.readChar(); }
      catch(java.io.IOException e) { return curPos; }
   }
}
static final int[] jjnextStates = {
   12, 13, 15, 1, 2, 4, 23, 32, 
};
public static final String[] jjstrLiteralImages = {
"", null, null, null, null, null, null, "\155\144", 
"\143\151\160\150\145\162", null, "\144\141\164\141", "\155\157\144\145", "\160\141\144\144\151\156\147", 
"\151\166", "\141\165\164\157", "\163\160\145\145\144", "\151\156\143\154\165\144\145", 
null, null, null, null, null, null, null, null, null, "\52", "\75", };
public static final String[] lexStateNames = {
   "DEFAULT", 
   "IN_HEX_LITERAL", 
};
public static final int[] jjnewLexState = {
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 0, -1, -1, -1, -1, -1, -1, 
   -1, -1, -1, 
};
static final long[] jjtoToken = {
   0xf35ff81L, 
};
static final long[] jjtoSkip = {
   0x7eL, 
};
static final long[] jjtoSpecial = {
   0x40L, 
};
static final long[] jjtoMore = {
   0xa0000L, 
};
private ASCII_CharStream input_stream;
private final int[] jjrounds = new int[33];
private final int[] jjstateSet = new int[66];
StringBuffer image;
int jjimageLen;
int lengthOfMatch;
protected char curChar;
public MakerTokenManager(ASCII_CharStream stream)
{
   if (ASCII_CharStream.staticFlag)
      throw new Error("ERROR: Cannot use a static CharStream class with a non-static lexical analyzer.");
   input_stream = stream;
}
public MakerTokenManager(ASCII_CharStream stream, int lexState)
{
   this(stream);
   SwitchTo(lexState);
}
public void ReInit(ASCII_CharStream stream)
{
   jjmatchedPos = jjnewStateCnt = 0;
   curLexState = defaultLexState;
   input_stream = stream;
   ReInitRounds();
}
private final void ReInitRounds()
{
   int i;
   jjround = 0x80000001;
   for (i = 33; i-- > 0;)
      jjrounds[i] = 0x80000000;
}
public void ReInit(ASCII_CharStream stream, int lexState)
{
   ReInit(stream);
   SwitchTo(lexState);
}
public void SwitchTo(int lexState)
{
   if (lexState >= 2 || lexState < 0)
      throw new TokenMgrError("Error: Ignoring invalid lexical state : " + lexState + ". State unchanged.", TokenMgrError.INVALID_LEXICAL_STATE);
   else
      curLexState = lexState;
}

private final Token jjFillToken()
{
   Token t = Token.newToken(jjmatchedKind);
   String im = jjstrLiteralImages[jjmatchedKind];
   t.image = (im == null) ? input_stream.GetImage() : im;
   t.kind = jjmatchedKind;
   t.beginLine = input_stream.getBeginLine();
   t.beginColumn = input_stream.getBeginColumn();
   t.endLine = input_stream.getEndLine();
   t.endColumn = input_stream.getEndColumn();
   return t;
}

int curLexState = 0;
int defaultLexState = 0;
int jjnewStateCnt;
int jjround;
int jjmatchedPos;
int jjmatchedKind;

public final Token getNextToken() 
{
  int kind;
  Token specialToken = null;
  Token matchedToken;
  int curPos = 0;

  EOFLoop :
  for (;;)
  {   
   try   
   {     
      curChar = input_stream.BeginToken();
   }     
   catch(java.io.IOException e)
   {        
      jjmatchedKind = 0;
      matchedToken = jjFillToken();
      matchedToken.specialToken = specialToken;
      return matchedToken;
   }
   image = null;
   jjimageLen = 0;

   for (;;)
   {
     switch(curLexState)
     {
       case 0:
         try { 
            while (curChar <= 32 && (0x100003600L & (1L << curChar)) != 0L)
               curChar = input_stream.BeginToken();
         }
         catch (java.io.IOException e1) { continue EOFLoop; }
         jjmatchedKind = 0x7fffffff;
         jjmatchedPos = 0;
         curPos = jjMoveStringLiteralDfa0_0();
         break;
       case 1:
         jjmatchedKind = 0x7fffffff;
         jjmatchedPos = 0;
         curPos = jjMoveStringLiteralDfa0_1();
         if (jjmatchedPos == 0 && jjmatchedKind > 19)
         {
            jjmatchedKind = 19;
         }
         break;
     }
     if (jjmatchedKind != 0x7fffffff)
     {
        if (jjmatchedPos + 1 < curPos)
           input_stream.backup(curPos - jjmatchedPos - 1);
        if ((jjtoToken[jjmatchedKind >> 6] & (1L << (jjmatchedKind & 077))) != 0L)
        {
           matchedToken = jjFillToken();
           matchedToken.specialToken = specialToken;
           TokenLexicalActions(matchedToken);
       if (jjnewLexState[jjmatchedKind] != -1)
         curLexState = jjnewLexState[jjmatchedKind];
           return matchedToken;
        }
        else if ((jjtoSkip[jjmatchedKind >> 6] & (1L << (jjmatchedKind & 077))) != 0L)
        {
           if ((jjtoSpecial[jjmatchedKind >> 6] & (1L << (jjmatchedKind & 077))) != 0L)
           {
              matchedToken = jjFillToken();
              if (specialToken == null)
                 specialToken = matchedToken;
              else
              {
                 matchedToken.specialToken = specialToken;
                 specialToken = (specialToken.next = matchedToken);
              }
           }
         if (jjnewLexState[jjmatchedKind] != -1)
           curLexState = jjnewLexState[jjmatchedKind];
           continue EOFLoop;
        }
        jjimageLen += jjmatchedPos + 1;
      if (jjnewLexState[jjmatchedKind] != -1)
        curLexState = jjnewLexState[jjmatchedKind];
        curPos = 0;
        jjmatchedKind = 0x7fffffff;
        try {
           curChar = input_stream.readChar();
           continue;
        }
        catch (java.io.IOException e1) { }
     }
     int error_line = input_stream.getEndLine();
     int error_column = input_stream.getEndColumn();
     String error_after = null;
     boolean EOFSeen = false;
     try { input_stream.readChar(); input_stream.backup(1); }
     catch (java.io.IOException e1) {
        EOFSeen = true;
        error_after = curPos <= 1 ? "" : input_stream.GetImage();
        if (curChar == '\n' || curChar == '\r') {
           error_line++;
           error_column = 0;
        }
        else
           error_column++;
     }
     if (!EOFSeen) {
        input_stream.backup(1);
        error_after = curPos <= 1 ? "" : input_stream.GetImage();
     }
     throw new TokenMgrError(EOFSeen, curLexState, error_line, error_column, error_after, curChar, TokenMgrError.LEXICAL_ERROR);
   }
  }
}

final void TokenLexicalActions(Token matchedToken)
{
   switch(jjmatchedKind)
   {
      case 18 :
        if (image == null)
            image = new StringBuffer(new String(input_stream.GetSuffix(jjimageLen + (lengthOfMatch = jjmatchedPos + 1)), 0));
         else
            image.append(new String(input_stream.GetSuffix(jjimageLen + (lengthOfMatch = jjmatchedPos + 1)), 0));
            String h = image.toString();
            // get rid of the enclosing angle brackets
            matchedToken.image = h.substring(1, h.length() - 1);
         break;
      default : 
         break;
   }
}
}