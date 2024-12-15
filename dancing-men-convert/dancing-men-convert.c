/*--- dancecnv.c -- Program to convert ASCII text to a form
 *  suitable for use with the Dancing Men Postscript font.
 *
 *  "Dancing Men" is my font based on the cipher in A. Conan Doyle's
 *  Sherlock Holmes story "The Adventure of the Dancing Men".
 *  There are only 18 letters on the font, the letters 
 *  fjkquwxz being absent.
 *  Lowercase letters are used for all but the last letters
 *  in a word.  Uppercase letters are used for the last letter
 *  in a word.  (In the font, an uppercase letter is the same as
 *  its corresponding lowercase letter, with the addition of a
 *  flag or pendant signifying end of word.)  Note:  the last
 *  letter of the *last* word in a message should not be
 *  capitalized, to meet Doyle's implied specifications.
 *
 *  The 8 characters not in the font are each mapped to one or
 *  more of the 18 characters that are in the font.  For example,
 *  'z' is mapped to "hs", in a crude attempt to get roughly the
 *  same sound as a 'z'.  Doyle's cipher alphabet has a few
 *  mistakes, particularily with the letters p and v, so I
 *  avoid using these characters in the mapped strings.
 *
 *  Mark Riordan  3 Aug 93
 */                       
 
#include <stdio.h>
#include <ctype.h> 
#include <string.h>

enum enum_case {LOWER, UPPER};

void PutMappedChar(int letter, enum enum_case letterCase);
void PutCharEOL(int letter);

int CharsPerLine=12;     
int CharsThisLine=0;                        

int 
main(int argc, char *argv[])
{
	int ch;
	int letter=0, inword=0;
	
	while(EOF != (ch=getchar())) { 
		ch = tolower(ch);
		if(isalpha(ch)) {
			if(!inword) {
				/* Beginning of new word--flush last letter of old word. */
				if(letter) PutMappedChar(letter,UPPER);
			} else {                               
				/* Within word--put out previous character of this word. */
				if(letter) PutMappedChar(letter,LOWER);
			}
			letter = ch;      
			inword = 1;
		} else {
			inword = 0;
		}
	}
	if(letter) PutMappedChar(letter,LOWER);
	if(CharsThisLine) putchar('\n');
	
	return 0;		
}
   
/*--- function PutMappedChar ---------------------------------------
 *
 *  Output a character in the given case, first mapping the
 *  specified character into a possibly multi-letter string
 *  for those if it's one of the 8 that is not in our output
 *  character set.
 *
 *  Entry:	letter	is the character to output, possibly after
 *							first mapping it to a string.
 *				letterCase 	is the case (LOWER or UPPER) in which
 *							to output the letter.  UPPER is used for
 *							the last letter in a word.
 */                   
void 
PutMappedChar(int letter, enum enum_case letterCase)
{
	static struct struct_charmap {
		char inchar;
		char *mapping;
	} CharMap[] = {
	 {'f',"v"}, {'j',"i"}, {'k',"c"}, {'q',"c"}, 
	 {'u',"oo"}, {'w',"vv"}, {'x',"cs"}, {'z',"hs"}, {'\0',""} };
	struct struct_charmap *map;   
	char *map_str; 
	int chars_in_map;
	
	for(map=CharMap, map_str=0; !map_str && map->inchar; map++) {
		if(map->inchar == letter) {
			map_str = map->mapping;
		}
	}
	              
	/* Is this a mapped character? */	              
	if(map_str) {                    
		/* Output all but the last character of the mapped-to string
		 * in lower case.
		 */
		chars_in_map = strlen(map_str);
		while(--chars_in_map) {
			PutCharEOL(*(map_str++));
		}
		letter =  *map_str;
	}
	      
	/* Now process either the original character (if unmapped),
	 * or the last character of the mapped string.
	 * Either way, this one should be given the case treatment
	 * specified by letterCase.
	 */	      
	if(letterCase == UPPER) letter = toupper(letter);
	PutCharEOL(letter);
}                    
 
/*--- function PutCharEOL -----------------------------------------
 *
 *  Output a character and move cursor to next line if we
 *  reach the right margin.
 */
void
PutCharEOL(int letter)
{
	putchar(letter);
	
   if(++CharsThisLine >= CharsPerLine) {
   	putchar('\n');
   	CharsThisLine = 0;
   }
}
   	
   