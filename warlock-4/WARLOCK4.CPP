// A Computer Program for Achieving a Public Key Encryption  and
// Decryption Method.
//
// (c) COPYRIGHT  1991   Charles Larry Craig and William Joseph Wilson
// All Rights Reserved
//
/*

Welcome to the wonderful world of WARLOCK 4.0!

************************************************************************

WARNING:   The WARLOCK cryptosystem provided herein is a copy-
righted system protected by patents (awarded and pending) and is
provided solely for private personal use and evaluation only.
Modifications to (or copies of) WARLOCK source or object programs
must retain the warning and proprietary legend displayed on the
first user screen.

The use of WARLOCK cryptosystems for private-sector commercial or
public-sector governmental purposes is strictly prohibited with-
out proper licensing arrangements. Licensing information can be
obtained by writing DataSec Systems, PO Box 4152, Huntsville, AL 35815-4152,
or by sending a request by E-MAIL to WARLOCK@ACM.org,
or by phoning (205) 881-8002.

************************************************************************

*/

// **** CAUTION: Many comments are no longer appropriate.

// This program, when released, was compiled with Borland 3.1,
// using the small memory model option.
// A project file (WARLOCK4.PRJ) is provide which selects this option.

// The following is a C++ source language program (WARLOCK.CPP) written as a
// software embodiment of a high-speed, matrix-based, asymmetric public key 
// cryptographic method with full digital signature capabilities.  The 
// cryptosystem resulting from this method comprises a functional module   
// for the generation of public and private cryptographic keys as well as
// a functional module for encrypting plaintext and another for decrypting
// ciphertext.   The encryption and decryption modules also serve to create
// and verify digital signatures for source message authentication as needed.
//

// To facilitate understanding of the novel public key cryptographic method 
// embodied in this program, extensive annotations and comments have been
// embedded throughout.  These are preceded -- as is this text -- by double 
// slash symbols.
//
// ********************** Begin Program Definition *************************
//
// parameters to specify the word size of the physical (or logical) processor
// of the host computer:

const BITSPERBYTE = 8;
const BYTESPERWORD = 2;
const WORDSIZE = BITSPERBYTE * BYTESPERWORD;

// the raw values for logical symbols:

enum {FALSE, TRUE};
enum {OFF, ON};

// the library files for certain prewritten functions that are used:
 
#include <iostream.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
time_t time1, time2, time3;


// ************************** begin BitMatrix header ***********************

// declare the class "Crypto" so it can be a friend of the class "BitMatrix"

class Crypto;

// use typedef to simplify type specifications:

typedef unsigned int BitMatrixType;
typedef BitMatrixType *BitMat;

// ---------------- begin actual definition of class "BitMatrix" ----------

class BitMatrix{

public:                        // each BitMatrix has:
	int number_of_rows;	//  a certain number of rows,
	int number_of_columns;	//  a certain number of columns,
	int row_width;		//  a certain number of words per row,
	int matrix_size;	//  a certain number of words per matrix

	BitMat bm;		// a pointer to the matrix itself

	friend class Crypto;	// declare Crypto a friend of BitMatrix so
				// Crypto can access its private data

// ------------- declare the member functions of BitMatrix ------------------

public:				// each BitMatrix object can be called upon
				// (or can be sent a "message") to perform
				// the following functions:

	BitMatrix ( int row, int column);	// the constructor
	~BitMatrix (void) {			// the destructor

				delete [matrix_size] bm; // must specify
							 // number of
							 // physical or
							 // logical words
							 // in the matrix
			  }


// get the word index, within a row, where a specified bit resides

BitMatrix::get_index ( int pos ) 

{
	return ( pos / WORDSIZE ); // return word index that bit is in
 }

// get the bit offset, within a word, where a specified bit resides

BitMatrix::get_offset ( int pos )

{
		return ( pos % WORDSIZE ); // return position of bit in word
 }

// set a bit to "1" in a specified row and column in a BitMatrix object
//


// NOTE:
//
// each BitMatrix is a matrix of bits that are not individually addressable
// (if there are 16 bits in a host processor word, then all 16 bit positions
// are used in the bit matrix, which means, for instance, that if a plaintext
// block of 64 bits is used -- so that there are 64 columns, numbered 0 thru
// 63, in that BitMatrix -- then the sixtyth bit -- counting from zero -- 
// would be stored in the third -- counting from zero -- 16 bit word and the
// thirteenth bit -- counting from zero -- in that word)

void BitMatrix::set_matrix_bit ( int row, int column )
{
/*
	check_bounds ( row, column );		// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so
*/
	int index = get_index ( column );	// get the index to the word
						// within the row containing
						// the bit to be set (to 1)

	int offset = get_offset ( column );     // get the bit offset
						// (within that word, within
						// the row) to the bit to be
						// set (to 1)

	// turn on bit at row, word index by "ORing" in the bit at the 
	// computed offset within the computed word position within the 
        // specified row

	*(bm + ( row * row_width ) + index) |= (ON << offset);

	// NOTE:
	//
	// the above statement means, in a more mathematical language:
	//
	// bm[row, index] = bm[row, index] or 2**offset
	//   where 2**offset means 2 to the offset power, e.g.,
	//   2**3 = 2*2*2 = 8
}

// reset a bit to "0" in a specified row and column in a BitMatrix object
//

// NOTE:
//
// each BitMatrix is a matrix of bits that are not individually addressable
// (if there are 16 bits in a host processor word, then all 16 bit positions
// are used in the bit matrix, which means, for instance, that if a plaintext
// block of 64 bits is used -- so that there are 64 columns, numbered 0 thru
// 63, in that BitMatrix -- then the sixtyth bit -- counting from zero -- 
// would be stored in the third -- counting from zero -- 16 bit word and the
// thirteenth bit -- counting from zero -- in that word)


void BitMatrix::reset_matrix_bit ( int row, int column )
{
/*
	check_bounds ( row, column );		// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so
*/
	int index = get_index ( column );	// get the index to the word
						// within the row containing
						// the bit to be set (to 1)

	int offset = get_offset ( column );	// get the bit offset within
						// that word within the row
						// to the bit to be set (to 1)

	// turn off bit at row, word index by "ANDing" out the bit at the 
	// computed offset within the computed word position within the 
	// specified row

	*(bm + ( row * row_width ) + index) &= (~(ON << offset));

	// NOTE:
	//
	// the above statement means, in a more mathematical language:
	//
	// bm[row, index] = bm[row, index] and ~(2**offset)
	//   where 2**offset means 2 to the offset power, e.g.,
	//   2**3 = 2*2*2 = 8
	//   and where ~(2**offset) means the two's complement of (2**offset)
	//   e.g., if a 16 bit word is used
	//   2**3 = 0000000000000100 in binary and the two's complement is
	//          1111111111111011

}

/*
	void set_matrix_bit ( int row, int column);   	// at [row, column]
	void reset_matrix_bit ( int, int ); 		// at [row, column]
*/
	void swap_rows ( int row1, int row2);  	    	// swap row1 and row2
	void swap_columns ( int column1, int column2);	// ditto columns
// test if a bit is set (one) at a specified row and column in a BitMatrix
//
// NOTE:
//
// each BitMatrix is a matrix of bits that are not individually addressable
// (if there are 16 bits in a host processor word, then all 16 bit positions
// are used in the bit matrix, which means, for instance, that if a plaintext
// block of 64 bits is used -- so that there are 64 columns, numbered 0 thru
// 63, in that BitMatrix -- then the sixtyth bit -- counting from zero --
// would be stored in the third -- counting from zero -- 16 bit word and the
// thirteenth bit -- counting from zero -- in that word)

int BitMatrix::test_bit_set ( int row, int column)
{
/*
	check_bounds ( row, column );		// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so
*/

//	int index = get_index ( column );	// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so

//	int offset = get_offset ( column );	// get the bit offset within
						// that word within the row
						// to the bit to be set (to 1)

	// test bit at row, word index by "ANDing" to select the bit at the
	// the computed offset within the computed word position within the
	// specified row

	if ( *(bm + ( row * row_width ) + (column>>4)) & (ON << (column & 0xF)) )
		return TRUE;
	return FALSE;

	// NOTE:
	//
	// the above statements mean, in a more mathematical language:
	//
	// if ((bm[row, index] = bm[row, index] and (2**offset)) = 0)
	// 	return FALSE
	// else return TRUE; (TRUE=1, FALSE=0)
	//   where 2**offset means 2 to the offset power, e.g.,
	//   2**3 = 2*2*2 = 8

	//   and where ~(2**offset) means the two's complement of (2**offset)
	//   e.g., if a 16 bit word is used
	//   2**3 = 0000000000001000 in binary and the two's complement is
	//          1111111111110111

}


// test if a bit is reset (zero) at a specified row and column in a BitMatrix
//
// NOTE:
//
// each BitMatrix is a matrix of bits that are not individually addressable
// (if there are 16 bits in a host processor word, then all 16 bit positions
// are used in the bit matrix, which means, for instance, that if a plaintext
// block of 64 bits is used -- so that there are 64 columns, numbered 0 thru
// 63, in that BitMatrix -- then the sixtyth bit -- counting from zero --
// would be stored in the third -- counting from zero -- 16 bit word and the
// thirteenth bit -- counting from zero -- in that word)

int BitMatrix::test_bit_reset ( int row, int column)
{
/*
	check_bounds ( row, column );		// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so
*/

//	int index = get_index ( column );	// determine if either the row
						// or column is out of bounds
						// and write an error message
						// if so

//	int offset = get_offset ( column );	// get the bit offset within
						// that word within the row
						// to the bit to be set (to 1)

	// test bit (for zero) at row, word index by "ANDing" to select the
	// bit at the computed offset within the computed word position
	// within the specified row

	if ( *(bm + ( row * row_width ) + (column>>4)) & (ON << (column&0xF) ))
		return FALSE;
	return TRUE;

	// NOTE:
	//
	// the above statements mean, in a more mathematical language:
	//
	// if ((bm[row, index] = bm[row, index] and (2**offset)) = 0)
	// 	return TRUE
	// else return FALSE; (TRUE=1, FALSE=0)
	//   where 2**offset means 2 to the offset power, e.g.,
	//   2**3 = 2*2*2 = 8
	//   and where ~(2**offset) means the two's complement of (2**offset)
	//   e.g., if a 16 bit word is used

	//   2**3 = 0000000000001000 in binary and the two's complement is
	//          1111111111110111
}

/*
	int test_bit_set ( int row, int column);       	// at [row, column]
	int test_bit_reset ( int row, int column);    	// at [row, column]
*/
	void xor_row ( int row1, int row2);	    	// row 1 ^= row 2
	void xor_column ( int column1, int column2);	// column 1^= column 2
	void reset (void);				// reset (zero) matrix
	void print_matrix( void );			// print matrix
	void BitMatrix::identitize ( void );		// make an I matrix



// .....the following, for technical reasons, are friend functions,
// .....(functions that can access private data of BitMatrix
// .....objects) but which must be called upon sightly differently
// .....than the pure function members of class BitMatrix

	friend void multiply_vector_matrix	// V1 = V2 * M
						// where V1 and V2 are
						// row vectors or, in
						// reality, matrices of
						// dimension 1 x N where
						// N is the number of 
						// entries in each vector
			   (
				BitMatrix& V1,	// Vector results
				BitMatrix& V2,	// Vector multiplier
				BitMatrix& M	// Matrix
			    );

	friend void multiply_matrix_matrix	// M1 = M2 * M3

			(
				BitMatrix& M1,	// results
				BitMatrix& M2,	// first
				BitMatrix& M3	// second
			);

	friend void remove_T_row_noise       	// remove the noise bits
                                                // (deconvolute)
      						// from the specified row
						// ( row_0 )
						// [currently always row 0]
						// of the specified matrix
						// ( intermediate_text )
						// [currently a vector
						// cast as a matrix]
						// injected by the specified
						// ( T_row ) row of the matrix
						// ( key_T_with noise )
		(
			BitMatrix& intermediate_text,
			int row_0,
			BitMatrix& key_T_with_noise,
			int T_row
		);

	friend void make_pair                   // form two random matrices
						// M and its inverse M_inverse
						// using a specified random
						// function ( specified_rand )
						// to generate the random bits
			(
				BitMatrix& MorA,
				BitMatrix& MorA_inverse,
				int (*specified_rand) ( void ),
				BitMatrix& Work

			);

	friend void generate_jumble_matrix      // generate a jumble matrix
						// (J) using the random

						// function (specified_rand)
						// where J is used to jumble
						// (four rows at a time) the
                                                // row groups of the matrix
						// to be released as the public
						// key K 
		(
			BitMatrix& J,
			int (*specified_rand) ( void )
		);

      friend  void  jumble_row_groups_of_matrix // using the matrix J generated
						// by the above function
						// (generate_jumble_matrix),
						// jumble the row groups of
						// matrix (T_replaced)
						// placing the results in
						// M which will serve
						// as the public key K
						// (after its row groups
						// are appropriately jumbled
                                                // four rows at a time
		(
			BitMatrix& M ,    	// results
			BitMatrix& T_replaced,
			BitMatrix& J
		);

	friend  void generate_T_with_noise	// generate  the initial
						// T matrix (T_with_noise)
						// with "noise bits" and
						// "row identifier bits" in
						// place using the specified
						// random function
						// (specified_rand)
			(
				BitMatrix& T_with_noise,
				int ( *specified_rand ) ( void )
			);


	friend  void generate_B_with_noise	// generate  the initial
						// B matrix (B_with_noise)
						// with "noise bits" and
						// "row identifier bits" in
						// place using the specified
						// random function
						// (specified_rand)
			(
				BitMatrix& B_with_noise,
				int ( *specified_rand ) ( void )
			);

	friend void generate_replacement_rows	// generate the "replacement
						// row values" in matrix form
						// in matrix replacement_rows
						// using the specified random
						// function ( specified_rand )
			(

				BitMatrix& replacement_rows,
				int ( *specified_rand ) ( void )
			);

	friend void add_matrix_matrix		// added

			(
				BitMatrix& T_replaced,
				BitMatrix& replacement_rows,
				BitMatrix& T_with_noise
			);


//      new copy matrix to matrix function

	friend void copy_matrix_matrix
			(
				BitMatrix& target,
				BitMatrix& source
			);

//	new, more general matrix copy function

	friend void copy_to_part_of_matrix
		(
			BitMatrix& target,
			BitMatrix& source,
			int row_number,
			int column_number
		);
	friend void expand_A ( BitMatrix& A_expanded, BitMatrix& A );


//------------ new I/O functions


	friend void write_BitMatrix (FILE* output, BitMatrix& bitm);

	friend void read_BitMatrix (FILE* input, BitMatrix& bitm);


//------------- new other functions

	friend void set_BitMatrix (BitMatrix& bitm, unsigned int* array);

	friend void inc_BitMatrix (BitMatrix& bitm);

	friend void get_BitMatrix (BitMatrix& bitm,
                                  unsigned int* array);


// ----------- functions used only internally by BitMatrix:-----------------

private:	// helping functions


// check the bounds of the row and column indices

/*
void BitMatrix::check_bounds ( int row, int column )
{

	// make sure the column is within bounds of BitMatrix
	if ( column < 0 || column >= number_of_columns )
	{
		cerr << "\nBitMatrix column Out of Bounds: "
		     << "<< " << column
		     << "' number_of_columns: "
		     << number_of_columns
		     << " >>\n";
		exit( -1 ); // stdlib.h
	}

	// make sure the row is within bounds of BitMatrix
	if ( row < 0 || row >= number_of_rows )
	{
		cerr << "\nBitMatrix row Out of Bounds: "
		     << "<< " << row
		     << "' number_of_rows: " << number_of_rows << " >>\n";
		exit( -1 );
	}

}
*/
/*
	void check_bounds ( int row, int column);	// check that the
							// row, column indices
							// of a matrix
							// are within bounds
*/
/*
	int get_index ( int column); 			// return word that
							// bit is positioned
							// in given the column
							// of the bit

	int get_offset ( int column );                  // return the bit
							// position within
							// a word given the
							// column of the bit
*/



// ********************* end BitMatrix header ************************

};

// ********************* begin Crypto header *************************

class Crypto{

private:		                // each Crypto has:

	BitMatrix *public_key_K;	// a (pointer to a) rectangular nxm
					// public key matrix K and the 
				        // following private key matrices: 

	BitMatrix *key_M;		// a (pointer to a) square, nonsingular
					// matrix M of nxn dimension (where n 
					// is an even number equal to the
                                        // system block length)
                                        
	BitMatrix *key_M_inverse;	// a (pointer to a) square, nonsingular
					// nxn matrix which is the inverse of
                                        // matrix M above

	BitMatrix *replacement_rows;	// a (pointer to a) rectangular matrix
					// containing "replacement rows"
					// made up of "replacement bits"
					// to be added row by row to matrix
					// T below to eliminate the "sea of
					// zeroes" below its diagonal 
                                        //
                                        // NOTE:
					//
                                        // throughout this program the terms 
                                        // "add" and "adding" designate 
                                        // modulo 2 addition (exclusive-OR);
                                        // the term "deconvolute" signifies
                                        // the removal of "noise bits" in the
					// decryption process
                                        //
	BitMatrix *key_T_with_noise;	// a (pointer to a) rectangular matrix
					// T containing 2x4 submatrices
                                        // stationed along a diagonal running
					// from the upper left portion to the 
					// lower right portion.  The   
                                        // submatrices contain "row identifier
                                        // values".  The area above the 
                                        // diagonal is filled with randomly 
					// generated "noise bits" while the
                                        // area below the diagonal contains
                                        // a "sea of zeroes" that will later
                                        // be filled with non-zero values
                                        // designated as "replacement bits"


	BitMatrix *key_T_replaced;	// another (pointer to a) rectangular
					// matrix formed by adding "replacement
					// bits" to T to eliminate the "sea of
					// zeros" under the diagonal making 
                                        // making the code harder to break

					// NOTE:

                                        //
					// a unique "replacement row" value
                                        // comprised of randomly generated
                                        // "replacement bits" is replicated
       					// four times (for each of the four
					// rows of each 4x2 submatrix) to
					// replace the zeroes to the left of
                                        // each submatrix.                   
                                        // However, the replacement row value
                                        // for the bottommost submatrix is
                                        // computed as the modulo 2 sum of
                                        // all other "replacement row" values
                                        // used.  As a result of this
                                        // technique, the "replacement row 
                                        // values" cancel out in the 
                                        // encryption process since 


					// every "replacement value" is
                                        // selected exactly once and added
                                        // modulo 2 which produces an 
                                        // all-zero result in every case

	BitMatrix *key_T_to_be_jumbled;	// another (pointer to a) rectangular
					// matrix product formed by multiplying
					// the above completed T matrix
					// (containing both noise bits and  
					// replacement bits) by M for the pur-
					// pose of disguising the bit patterns
                                        // of T in the resulting matrix thereby
                                        // making the code harder to break
                                        //
					// NOTE:
                                        //
					//  This matrix will serve as the
					//  public key K after its rows are
                                        //  ramdomly jumbled four rows at a
                                        //  time.  As a result of the 
                                        //  foregoing steps in the creation
                                        //  of K, it is computationaly 
                                        //  unfeasible to derive private
                                        //  key matrices T and M from public
                                        //  key matrix K 


	BitMatrix *key_Jumble;		// another (pointer to a) BitMatrix
					// used to jumble the BitMatrix above
					// to produce the public key K 
					//
					// NOTE:
					//
					// this is an array of numbers
					// (integers) cast as a BitMatrix --
					// when printed out by print_matrix,
                                        // the number is displayed in binary
					// notation with its bits reversed,
                                        // e.g., 6 prints as 011 followed by
                                        // a string of zeroes

	BitMatrix *key_I;		// a (pointer to an) I-matrix

					// (identity matrix) used to visually
					// check that an occurrence of matrix
					// M times matrix M inverse equals I

			      // the following are row vectors (cast as 1 x m
                              // matrices where m is the system block length)
                              // used in manipulating textual elements in the
			      // encryption and decryption processes

	BitMatrix *plain_text;		// this is the (pointer to the)
					// original plaintext block to be
					// encrypted


	BitMatrix *expanded_text;	// this is the (pointer to the)
					// plaintext block after it has
					// been expanded to twice its
					// original length
					//
					// NOTE:
					//
                                        // each two bit segment of the 
                                        // plaintext is used to select
					// one of four rows of corresponding
					// four-row sets of public key matrix
                                        // K (there are m/2 occurrences of 
                                        // four-row sets in K where m is the
                                        // system block length); each row 
                                        // selection is added modulo 2 with
                                        // all other like selections with the
                                        // result that the process of 
					// selecting and adding rows of K
					// is equivalent to the (row) vector
                                        // (of the expanded text) times
					// the key matrix K.  This technique
					// works because each two-bit segment
					// of the plaintext is expanded to a
					// four-bit segment in which exactly
					// one selecting bit is set to 1 


	BitMatrix *encrypted_text;	// this is the (pointer to the)
					// ciphertext block resulting from
					// multiplying the expanded plaintext
					// block above by the public key
                                        // matrix K

	BitMatrix *intermediate_text;   // this is the (pointer to the)
					// ciphertext block after it has
					// been "unmasked" by multiplying it
					// by the inverse of the masking
					// matrix M producing a partially
					// decrypted or "intermediate text"
                                        // block.  This is the first step of
					// the decryption process

	BitMatrix *resultant_text;      // this is the (pointer to the)
					// above intermediate text block
					// after deconvolution (removal of 
                                        // noise bit perturbations induced by
                                        // the public key) which constitutes
					// the second step of the decryption
                                        // process. This block, designated the 
                                        // "resultant text block" is a "bit
					// jumbled" (two bit permutation)
                                        // version of the original plaintext




	BitMatrix *decrypted_text;      // this is the (pointer to the)
					// above resultant text block after
					// it has been unjumbled to produce
					// the decrypted plaintext block by
					// the final step in the decryption
                                        // process


	BitMatrix *reverted_text;       // this is the (pointer to the)
					// reverted text which is the
					// encrypted text reverted to
					// its state before being multipled
					// by (big) M (key_M).
					// It is reverted to that state
					// by multiplying it by
					// (big) M inverse (key_M_inverse).
					// Once in that state, it may be
					// deconvoluted; it is copied to
					// intermediate text which is
					// input to deconvolute_text
					// which modifies the intermediate
					// text. Formally, there was no
					// explicit reverted text; the
					// output of the reversion process
					// was stored directly in the
					// intermediate text. The
					// deconvolution process then
					// overwrote this results,
					// leaving no record of it.
					// The reverted text was defined
					// so that there would be a record
					// of it.
	BitMatrix *Work_M;
	BitMatrix *Work_A;
/*
	int block_length;               // this is the system block length,
					// i.e. the length of plaintext and
					// ciphertext blocks
*/

	BitMatrix *A;
	BitMatrix *A_inverse;
	BitMatrix *B;
	BitMatrix *A_expanded;

	BitMatrix *X;
	BitMatrix *X_decoded;

	BitMatrix *unpadded_plain_text;
	BitMatrix *unpadded_decrypted_text;

	int padded_length;
	int unpadded_length;


// ------------------ declare the member functions of Crypto

public:

// the constructor for an object of Crypto class

	Crypto ( int );        // length of text block

// the destructor for an object of Crypto class

	~Crypto ( void );

// sets the plaintext block (to be encrypted) to a specified block

	void set_plain_text ( BitMatrix& V );

// **** new stuff for Crypto I/O

// reads a plaintext block (to be encrypted) from a specified FILE
// or returns an EOF indication if at EOF (to start with).

        int Crypto::read_block_of_plain_text (FILE* file_name);

// reads a encrypted_text block (to be decrypted) from a specified FILE
// or returns an EOF indication if at EOF (to start with).

        int Crypto::read_block_of_encrypted_text (FILE* file_name);

// writes a decrypted_text block (which has been decrypted) to a specified FILE

	void Crypto::write_block_of_decrypted_text (FILE* file_name);


// writes a encrypted_text block (which has been decrypted) to a specified FILE


        void Crypto::write_block_of_encrypted_text (FILE* file_name);

// **** end of section of new stuff for Crypto I/O


// ***** new stuff for other functions

	 friend void new_randomize ( char* key_phrase, Crypto& C);

	 friend int new_random ();


// sets the encrypted text block (to be decrypted) to a specified block

	void set_encrypted_text ( BitMatrix& V );


// gets the encrypted text block (to be decrypted) to a specified block

	void get_encrypted_text ( BitMatrix& V );

// gets the decrypted text block to a specified block


	void get_decrypted_text ( BitMatrix& V );

// encrypts the plaintext block

	void encrypt ( void );

// decrypts the encrypted text block

	void decrypt ( void );

// the following generates the public and private keys given a specified
// random number generator function ------------------------------------

	void generate_keys ( int (*specified_rand) ( void ) );

// new for reading and writing

// writes the public key

	friend void write_public_key (FILE* file_name, Crypto& to_write);

// writes the private and public keys

	friend void write_private_and_public_keys
                    (FILE* file_name, Crypto& to_write);

// reads the public key

	friend void read_public_key (FILE* file_name, Crypto& to_read);

// reads the private and public keys

	friend void read_private_and_public_keys
                    (FILE* file_name, Crypto& to_read);
/*
	int get_block_length ()
	{
		return(block_length);
	}
*/


// ENCRYPTs a text file

   void ENCRYPT();

// DECRYPTs a text file

   void DECRYPT();


// to generate keys for demo

   void GENERATE();

// to show stuff

   void show_stuff();

private:

// the following expands the plaintext to twice its original size so that
// it can be multiplied as a row vector by the public key to produce encrypted 
// text

	void expand_text ( void );


// the following pads the intermediate text with bits the make
// the deconvolution process come out correctly

	void pad_intermediate_text ( void );

// the following "deconvolutes" the intermediate_text (produced by [matrix]
// multiplying the encrypted text by the key_M_inverse) to produce the
// resultant_text, which, when unjumbled, gives the decrypted text

	void deconvolute_text ( void );

// the following unjumbles the "deconvoluted" text to produce the final
// decrypted text -----------------------------------------------------

	void unjumble ( void );

// not currently used -- matrix muliplication used with expanded text instead
	void convolute_matrices ( void );

public:

	void print_X();
	void print_X_decoded();

	void print_A ();		  // Prints A
	void print_A_inverse();		  // Prints A_inverse
	void print_A_expanded();	  // Prints A_expanded
	void print_B();			  // Prints B
	void print_public_key_K ();	  // prints the public_key_K
	void print_key_M ();              // prints the key_M
	void print_key_M_inverse ();      // prints the key_M_inverse
	void print_key_I ();		  // prints the key_I
	void print_replacements_rows ();  // prints the replacements_rows
	void print_key_T_with_noise  ();  // prints the key_T_with_noise
	void print_key_T_replaced ();	  // prints the key_T_replaced
	void print_key_T_to_be_jumbled ();// prints the key_T_to_be_jumbled
	void print_key_Jumble ();	  // prints the key_Jumble
	void print_unpadded_plain_text(); // prints the unpadded_plain_text
	void print_plain_text();	  // prints the plain_text
	void print_expanded_text();	  // prints the expanded_text
	void print_encrypted_text();	  // prints the encrypted_text
	void print_intermediate_text();	  // prints the intermediated_text
	void print_resultant_text();	  // prints the resultant_text
	void print_decrypted_text();      // prints the decrypted_text
	void print_reverted_text();       // prints the reverted_text

// ********************* end Crypto header ****************************
};





// ********************* begin BitMatrix body *************************

// the constructor for BitMatrix:
//  sets up variables specifying size parameters in the BitMatrix,
//  allocates space for the bit matrix from off the "heap", 
//  stores the pointer to that space, and 
//  clears the bit matrix to zero

BitMatrix::BitMatrix ( int row_size, int column_size )
{
	number_of_rows = row_size;		// store the specified
						// number of rows

	number_of_columns = column_size;	// store the specified
						// number of columns

	row_width = ( number_of_columns + WORDSIZE - 1 )/WORDSIZE;

						// compute the "row_width",
						// i.e. the width of a matrix
						// row expressed as the number
						// of words required to store
						// the row value

	matrix_size = row_width * number_of_rows;
						// compute the "matrix_size",
						// i.e., the size of the
						// bit matrix in terms of
						// the number of words
						// required to store the
						// the bit matrix

	bm = new BitMatrixType [ matrix_size ]; // allocate the space
						// (from off the "heap")
						// for the bit matrix
						// and store the pointer
						// to that space in bm

	// now initialize bm to all 0's
	for ( int i = 0; i <  matrix_size; ++i )// set every word in the
						// bit matrix to zeroes

		*(bm + i) = 0;		
						//
						// NOTE:
                                                //
						// *(bm + i)
						// means the same as
						// bm[i]
}

// swap the specified rows in a BitMatrix
//
// NOTE:
//
// the swap is carried out a word at a time (not a bit at a time) with one
// word swapped before the next word in the series is addressed

void BitMatrix::swap_rows ( int row1, int row2)
{

	int hold;
	int row1_index = ( row1 * row_width ); // compute starting row1_index
	int row2_index = ( row2 * row_width ); // compute starting row2_index
	for ( int i = 0; i < row_width; ++i )  // swap every word in the rows

	{
		hold = *(bm + row1_index + i); // save the iTH word of row1



		*(bm + row1_index + i) = *(bm + row2_index + i);
					       // bm[row1_index+i] =
					       // bm[row2_index+i]
					       // or set the iTH word of row1
					       // = the iTH word of row2

		*(bm + row2_index + i) = hold; // recover the saved word
					       // and put in the iTH word
					       // of row2
	}
}

// swap the specified columns in a BitMatrix --- currently not used

void BitMatrix::swap_columns ( int column1, int column2)
{
	int j;
	int column1_index = get_index ( column1 );
	int column1_offset = get_offset ( column1 );
	int column2_index = get_index ( column2 );
	int column2_offset = get_offset ( column2 );

	for ( int i = 0; i < number_of_rows; ++i )
	{
		j = i * row_width;
		if ( *(bm + j  + column1_index) & ( ON << column1_offset )  )
		{
			if  ( *(bm + j + column2_index)
			      & ( ON << column2_offset ) )
			; // then bits are the same, no need to swap

			else
			{
				reset_matrix_bit ( i, column1 );
				set_matrix_bit ( i, column2);
			}
		}
		else if
			(


				*(bm + j + column2_index)
					&

				( ON << column2_offset )

			)
		{
			set_matrix_bit ( i, column1 );
			reset_matrix_bit ( i, column2 );
		}
		// else bits are the same, no need to swap
	}


}

// exclusive-OR the row indicated by row2 into the row indicated by row1

void BitMatrix::xor_row ( int row1, int row2 )

{ 
	int row1_index = ( row1 * row_width ); // compute row1 starting index
	int row2_index = ( row2 * row_width ); // compute row2 starting index
	for ( int i = 0; i < row_width; ++i ) // for every word in the rows
		*(bm + row1_index + i) ^= *(bm + row2_index + i);
// bm[row1_index+i] = bm[row1_index+i] xor bm[row2_index+i];

}

// exclusive-OR the column indicated by column2 into the column indicated by
// column1 -----------------------------------------------------------------


void BitMatrix::xor_column ( int column1, int column2)
{
	int j;
	int column1_index = get_index (column1);
	int column1_offset = get_offset (column1);
	int column2_index = get_index (column2);
	int column2_offset = get_offset (column2);
	for ( int i = 0; i < number_of_rows; ++i )
	{ // for every column bit position
		j = i * row_width;
		if ( *(bm + j + column2_index ) & ( ON << column2_offset ) )
		{
		  // if the bit in that position in column 2 is ON
		  // then change the state of the corresponding bit
		  // in column 1
			if ( *( bm + j + column1_index )
			    & ( ON << column1_offset ) )
				reset_matrix_bit ( i, column1 );
			else
				set_matrix_bit ( i, column1 );
		}
	}
}

// reinitialize all elements to 0

void BitMatrix::reset (void)
{ 
	for ( int i = 0; i <  matrix_size; ++i )
		*(bm + i) = 0;

}
// multiply a specified (row) vector by a specified matrix V1 = V2 * M; 
// where V1 and V2 are row vectors or, in reality, matrices of dimension
// 1 x N where N is the number of entries in each vector ---------------

void multiply_vector_matrix ( BitMatrix& V1, BitMatrix& V2, BitMatrix& M )
{

	int i,j,k;
	V1.reset();
	if ( V1.number_of_columns != M.number_of_columns )


	{ // if V1 is not compatible with M
		cerr << "\n target vector length of "
		     << V1.number_of_columns
		     << " does not match number of columns ("
		     << M.number_of_columns

		     << ") in matrix. \n";
		exit (-1);
	}


	if ( V2.number_of_columns != M.number_of_rows )
	{ // if V2 is not compatible with M
		cerr << "\n BitVector length of "
		     << V2.number_of_columns

		     << " does not match number of rows ("
		     << M.number_of_rows
		     << ")\n";
		exit (-1);
	}
	for ( i = 0; i < M.number_of_rows; ++i )
	{ // for each row of the matrix

		if (

			   *(V2.bm + (i>>4))
				   &
			    (ON << (i & 0xF))
		   )

// above replaces

//	       if (V2.test_bit_set (0,i) ) 		// if the iTH bit
							// in the vector
							// is set, then
		{
			k = i * M.row_width;

			for ( j = 0; j < V1.row_width; ++j ) 	// accumulate (by
							// checksumming) the
							// corresponding row
							// of the matrix

				*(V1.bm + j) ^= *(M.bm + k + j);
				// V1.bm[j]= V1.bm[j] xor M.bm[i,j]
		}
	}

}

// multiply a matrix by a matrix giving a third matrix:

void multiply_matrix_matrix		// M1 = M2 * M3
		(
			BitMatrix& M1,	// matrix results
			BitMatrix& M2,	// pre matrix
			BitMatrix& M3	// post matrix
		)
{ 
	int i,j,k,l;
	M1.reset();
	if ( M1.number_of_columns != M3.number_of_columns)
	{ // if M1 is not comparible with M3
		cerr << "\n target matrix with "
		     << M1.number_of_columns
		     << " does not match number of columns ("
		     << M3.number_of_columns
		     << ") in post matrix. \n";
		exit (-1);
	}
	if ( M2.number_of_columns != M3.number_of_rows )
	{ // if M2 is not compatible with M3
		cerr << "\n pre matrix number of columns ("

		     << M2.number_of_columns


		     << ") does not match post matrix"
		     << " number of rows ( "
		     << M3.number_of_rows;

		exit (-1);
	}
	for ( int h = 0; h < M1.number_of_rows; ++h)
	{ // for each row in target matrix

		l = h * M1.row_width;
		for ( i = 0; i < M3.number_of_rows; ++i )
		{ // for for each row in post matrix
			k = i * M3.row_width;

			for ( j = 0; j < M1.row_width; ++j )
			if ( M2.test_bit_set ( h, i ) )
			// if the row is selected by the bit at M2.bm[h,i]
				{ // for each word in a row
					*(M1.bm + l + j) ^= *(M3.bm + k + j);
					// M1.bm[h,l+j] =
					// M1.bm[h,l+j] xor M3.bm[i,j];
				}
		}

	}
}

// remove a row of noise bits (and template bits) from the intermediate_text

void remove_T_row_noise
	(
		BitMatrix& intermediate_text,	// the intermediate
		int row_0,			// currently, always row 0
		BitMatrix& key_T_with_noise,	// the (BitMatrix)
						// key_T_with_noise
		int T_row			// the specific row
						// within the matrix
						// whose noise bits
						// are to be "cancelled out"
	)
{
	int row_0_index = ( row_0 * intermediate_text.row_width );
	int T_row_index = ( T_row * key_T_with_noise.row_width );
	for ( int i = 0; i < intermediate_text.row_width; ++i )
		// every word in the row

	{
		*( intermediate_text.bm + row_0_index + i )
				^=
			*( key_T_with_noise.bm +T_row_index + i );
		// intermediate_text.bm [i] =
		// intermediate_text.bm[i]
		// xor


		// key_T_with_noise.bm[T_row,i];
	}
}

// print a BitMatrix, specified or implied

void BitMatrix::print_matrix( void )
{
	int j;
	for ( int i = 0; i < number_of_rows; ++i )
	{ // for each row of M
		cout << "\n";
		for ( j = 0; j < number_of_columns; ++j )
		{ // for each bit in that row
			if ( test_bit_set ( i, j) )
				cout << "1";		// output a "1"
			else
				cout << "0";		// output a "0"
		}
	}
}

// make an identity matrix (identitize) out of this specified or implied 
// BitMatrix

void BitMatrix::identitize ( void )
{ 
	if ( number_of_columns != number_of_rows )
	{
		cerr << "\nMatrix not square\n";
		exit (-1);
	}
	reset();				      // clear the BitMatrix

	for ( int i = 0; i < number_of_columns; ++i ) // set every diagonal
						      // bit to 1     
		set_matrix_bit ( i, i );
}

// create a random pair of (binary) matrices, with one the inverse of the other
//
// NOTE:
//
// the pair is computed iteratively based upon the following mathematics:
//
// (1) I = inverse (I)	(the identity matrix is equal to its inverse)
// (2) inverse (M*A) = inverse (A) * inverse (M)
// (3) inverse (L) = L  (for binary matrices with modulus 2 addition)
//	where L is an identity matrix plus one 1 off diagonal, e.g.,
//
//             1000                        1000                       1000
//	if L = 0101   then   inverse (L) = 0101 and   L*inverse (L) = 0100
//             0010                        0010                       0010
//             0001                        0001                       0001

// 	so the  L  = inverse (L) in this case
//
//	to see why this is so in general 
//	(for binary matrices with modulo 2 addition)
//	let X represent a zero matrix except for one off-diagonal 1, e.g.,
//
//          0000
//	X = 0001
//          0000
//	    0000
//
//	so that
//
//	L = I + X				(mod 2)
//
//	now
//
//	L * L = ( I + X ) * ( I + X )           (mod 2)
//	      = I*I + I*X + X*I + X*X           (mod 2)
//	      =  I  +  X  +  X  + X*X           (mod 2)
//	      =  I  + X*X                       (mod 2)
//	      =  I                              (mod 2)

//	X*X = 0 since the off-diagonal element
//		never gets multiplied by itself
//	therefore, since L * L = I,

//	L = inverse (L) by the definition of the inverse operation
//
// with these three mathematical laws, the following iterative algorithm
// will develop a random pair of (binary) matrices of any specified dimension
// with one being the inverse of the other:
//
// step 1) 	old (A) = I		(to start the process set A = I)
// step 2)	old (inverse (A)) = I	( by law (1) )
//
// 					( the iterative process follows)
//
// step 3)	L = I + X		( randomly select an off-diagonal
//					  element enbodied in X and form L )
//

// step 4)	new (A) = L * (old (A)) ( compute the new A)
//
// step 5)	new (inverse (A)) = old (inverse (A) ) * L
//					( by laws (2) and (3) )
//
// step 6)	old (A) = new (A)
//
// step 7)	repeat steps 3) through 6) a sufficient number of times
//
// this step completes the scheme for a mathematical algorithm for creating
// a random nonsingular matrix and its inverse
//


// however, the actual algorithm differs somewhat for reasons of efficiency:
// the multiplication of L*A can be performed more efficiently by 
// "exclusive-ORing" one selected row with another selected row, since L is
// constructed such that it selects two rows and "exclusive-ORs" one with the
// other
//
// likewise, the multiplication of inverse(A) * L  can be performed more
// efficiently simply by "exclusive-ORing" one selected column with another
// selected column, since L is constructed so that it selects two columns 
// and "exclusive-ORs" one with the other
//
// the trick is to randomly select two numbers, k and l, 
// such that 0 <= k < number_of_rows and  0 <= l < number_of_rows 
// and such that k is not equal to l then add (XOR) the lTH row of A to 
// the kTH row of A and add (XOR) the kTH column of inverse(A) to 
// the lTH column of inverse(A) note that the lTh row is added to 
// the kTH row but the kTH column is added to the lTH column

void make_pair
	(
		BitMatrix& MorA,
		BitMatrix& MorA_inverse,
		int (*specified_rand) ( void ),
		BitMatrix& Work

	)

// ************ new version of make_pair

{
	int j,k,l,m;
	m = MorA.number_of_rows;
//	BitMatrix Work(m,m);
	if (
		MorA.number_of_rows != MorA.number_of_columns
	   ||	MorA_inverse.number_of_rows != MorA_inverse.number_of_columns
	   ||	MorA.number_of_rows != MorA_inverse.number_of_rows
	   )
							// check that matrices
							// are the right size

	{
		cerr << "\n error forming inverse\n";
		exit (-1);
	}
	MorA.reset ();					// clear A
	MorA_inverse.reset ();				// clear inverse(A)
	MorA.identitize();					// A = I
//	MorA_inverse.identitize();				// inverse(A) = I
	Work.reset();
	Work.identitize();



	for ( j = 0; j <  MorA.number_of_rows  * MorA.number_of_rows ; ++j )

							// do sufficient
							// number of times
	{
		k = abs(specified_rand()) % MorA.number_of_rows;// select an
							// appropriate k



		for ( l=k; l == k;
		      l = abs(specified_rand()) % MorA.number_of_rows );
							// select an
							// appropriate l

		MorA.xor_row ( k, l );			// XOR row l
							// into row k
							// of matrix A

//		MorA_inverse.xor_column ( l, k );		// XOR column k
							// into column l
							// of inverse(A)
		Work.xor_row ( l, k );

//cout << "\nl = " << l  << " k = " << k;
//cout << "\nWork = ";
//Work.print_matrix();
//cout << "\nA =";
//A.print_matrix();

	}
	for (j=0; j<m; ++j)
	    for (k=0; k<m; ++k)
		if (Work.test_bit_set(j,k))
		   MorA_inverse.set_matrix_bit (k,j);
}











// jumble the "key_T_to_be_jumbled" to produce the "public_key_K"
//
// NOTE:
//
// the BitMatrix is jumbled four rows at a time according to the
// specifications stored in "key_Jumble"; that is, the first value in
// "key_Jumble" specifies from where the first four rows are to be obtained
// etc. the number (cast as a bit matrix value) in the "key_Jumble"
// specifies which set of four rows to obtain to place in the next four row
// slot (beginning with the first set of four rows, etc.)
//
// for example, if the beginning row sets are labeled:
//
//      A
//      B
//      C
//      D
//
// and the key_Jumble specifies:
//
//      0 2 3 1
//
// then the row sets will be jumbled to produce:
//
//      A
//      C
//      D
//      B

void jumble_row_groups_of_matrix
	 	(

			BitMatrix& public_key_K ,    	// results
			BitMatrix& key_T_to_be_jumbled,
			BitMatrix& key_Jumble
		)
{
	int i, j;

	// first, check the dimensions of the matrices

	if (public_key_K.number_of_rows < ( 4*key_Jumble.number_of_rows ))
	{
		cerr << "\n error 1 in jumble specifications \n";
		cerr << "K = " << public_key_K.number_of_rows
		     << "J = " << key_Jumble.number_of_rows;
		exit (-1);
	}
	if (  key_Jumble.row_width != 1	   )
	{
		cerr << "\n error 2 in jumble specifications \n";
		exit (-1);
	}
	if (public_key_K.number_of_rows != key_T_to_be_jumbled.number_of_rows)

	{
		cerr << "\n error 3 in jumble specifications \n";
		exit (-1);
	}

	if (public_key_K.number_of_columns
			!=
	    key_T_to_be_jumbled.number_of_columns)
	{

		cerr << "\n error 4 in jumble specifications \n";
		exit (-1);
	}


	// start the jumble process

	public_key_K.reset();
	for ( i = 0; i < key_Jumble.number_of_rows; ++i )      // for every
                                                               // set of four
							       // rows
	{
		for ( j = 0; j < public_key_K.row_width; ++j ) // for every

							       // word within
							       // the row
		{
			*(public_key_K.bm + 4*public_key_K.row_width*i+j)=
			  *(
				key_T_to_be_jumbled.bm
					+
				4*key_T_to_be_jumbled.row_width
					*
				( *(key_Jumble.bm+i) )
					+
				j
			    );
			    // one word of first row assignment

			*(public_key_K.bm + (4*i+1)*public_key_K.row_width+j)=

			  *(
				key_T_to_be_jumbled.bm

					+
				(4*( *(key_Jumble.bm+i) )+1)
					*
				key_T_to_be_jumbled.row_width
					+
				j
			    );
			    // one word of second row assignment

			*(public_key_K.bm + (4*i+2)*public_key_K.row_width+j)=
			  *(
				key_T_to_be_jumbled.bm
					+
				(4*( *(key_Jumble.bm+i) )+2)
					*
				key_T_to_be_jumbled.row_width
					+
				j
			  );


			  // one word of third row assignment

			*(public_key_K.bm + (4*i+3)*public_key_K.row_width+j)=
			  *(
				key_T_to_be_jumbled.bm
					+
				(4*( *(key_Jumble.bm+i) )+3)
					*
				key_T_to_be_jumbled.row_width
					+
				j
			   );
			   // one word of fourth row assignment
		}
	}

}

void generate_jumble_matrix
	(
		BitMatrix& key_Jumble,
		int (*specified_rand) ( void )
	)
{
	int i,k,l;

	// generate the unjumbled key_Jumble (generate a sequence of numbers)

	for ( i = 0; i < key_Jumble.number_of_rows; ++i )
	{
		*(key_Jumble.bm + i*key_Jumble.row_width) = i;
	}



	// jumble the sequence of numbers by randomly selecting two entries
	// and swapping them a sufficient number of times -----------------

	for		// do a sufficient number of times
		(
		 i = 0;
		 i < key_Jumble.number_of_rows*key_Jumble.number_of_rows;
		 ++i
		)
	{
			// select an appropriate entry
		k = abs(specified_rand()) % key_Jumble.number_of_rows;
		for     // select another appropriate entry
			(
			 l = k;
			 l == k;
			 l = abs(specified_rand()) % key_Jumble.number_of_rows
			);

			// swap the entries

		key_Jumble.swap_rows ( k, l );
	}
}

// generate "key_T_with_noise"
//
// NOTE:
//
// the template bits (row identifiers) as well as the noise bits for the 
// initial T matrix are generated here:
//
// noise bits are generated a bit at a time: each possible bit position
// in which a noise bit could appear is cycled through one at a time; 
// a specified random function is called to generate a random number for each
// of these bit positions; if the random number is odd, a 1 bit is set at that
// bit position; if not, the original zero at that position is left intact
//
// a submatrix template containing the following row identifiers is 
// established as follows:
//
//	00
//	01
//	10
//	11
//
// this template is shifted over two columns every four rows to create a
// "diagonal" of template values.  The area of zeroes above the diagonal is
// then filled with randomly generated "noise bits"
//
// a simple example of a generated "key_T_with_noise is shown below
//


//	00NNNN
//	01NNNN
//	10NNNN
//	11NNNN
//	0000NN
//	0001NN
//	0010NN
//	0011NN
//	000000
//	000001
//	000010
//	000011
//
//	where each N represents a randomly generated noise bit which may be 1
//      or 0 independent of every other noise (N) bit value
//
//	notice that noise bits appear only in the upper right "triangle"
//      (the area above the diagonal of submatrices) of the T matrix -- 
//      hence the designation "T matrix").  They are placed there to eliminate
//      the zeroes that would otherwise populate that area helping make the 
//      private key matrix ultimately computationally unfeasible to obtain 
//      from the public key K

void generate_T_with_noise
			(
				BitMatrix& key_T_with_noise,
				int ( *specified_rand ) ( void )
			)

{
	int b,i,j,k,l,m,r, no_to_set, starting_position;
	key_T_with_noise.reset();
	for ( i=0; i < key_T_with_noise.number_of_rows; ++i ) // for every row
	{
		j = i >> 2;
		k = i & 3;
		l = i/2;
		no_to_set = key_T_with_noise.number_of_columns
				- 2 - ( l - ( l & 1 ) );

		starting_position = key_T_with_noise.number_of_columns

					- no_to_set;
		for ( ; no_to_set > 0; --no_to_set, ++starting_position )
				// for every permissible noise bit position
		{

// **** addition to make all four rows "add up"
/*
                        if ( k == 3 )
                        {
				for ( m=0,b=0; m < 3; ++m)
                                {
                                        if (key_T_with_noise.test_bit_set
					     (4*j+m, starting_position) )
                                        {
                                                b = b ^ 1;
                                        }
                                }
                                if (b)
                                {
					key_T_with_noise.set_matrix_bit
                                          (4*j+3, starting_position);
                                }
                                continue;
			}
*/
// **** end addition

			r = specified_rand();
			if ( r & 1 ) 	// decide if the bit should be set
			{
				key_T_with_noise.set_matrix_bit //set the bit

					( i, starting_position );
			}
		}


		// set the submatrix template bits

		if ( k == 0 )
		{
			continue;
		}
		if ( k == 1 )
		{
			key_T_with_noise.set_matrix_bit
				( 4 * j  +  1, 2 * j  +  1 );
			continue;
		}
		if ( k == 2 )
		{
			key_T_with_noise.set_matrix_bit
				( 4 * j  +  2, 2 * j );
			continue;
		}
		if ( k == 3 )
		{
			key_T_with_noise.set_matrix_bit
				( 4 * j  +  3, 2 * j  +  1 );

			key_T_with_noise.set_matrix_bit
				( 4 * j  +  3, 2 * j );

			continue;
		}
		cerr << "\nerror 1 in generate_key_T_with_noise";
	}
}
/*


void generate_B_with_noise
			(
				BitMatrix& B,
				int ( *specified_rand ) ( void )
			)

{
	int b,i,j,k,l,m,r, no_to_set, starting_position;
	B.reset();

	for ( i=0; i < B.number_of_rows; ++i ) // for every row
	{
		j = i >> 2;
		k = i & 3;
		l = i/2;
		no_to_set = B.number_of_columns
				- 2 - ( l - ( l & 1 ) );

		starting_position = B.number_of_columns

					- no_to_set;
		for ( ; no_to_set > 0; --no_to_set, ++starting_position )
				// for every permissible noise bit position
		{

// **** addition to make all four rows "add up"

			if ( k == 3 )
			{
				for ( m=0,b=0; m < 3; ++m)
				{
					if (B.test_bit_set
					     (4*j+m, starting_position) )
					{
						b = b ^ 1;
					}
				}
				if (b)
				{
					B.set_matrix_bit
					  (4*j+3, starting_position);
				}
				continue;
			}
// **** end addition

			r = specified_rand();
			if ( r & 1  ) 	// decide if the bit should be set
			{
				B.set_matrix_bit //set the bit

					( i, starting_position );
			}
		}


		// set the submatrix template bits

		if ( k == 0 )
		{

			B.set_matrix_bit
				( 4 * j  +  3, 2 * j );
			B.set_matrix_bit
				( 4 * j  +  3, 2 * j + 1 );
			B.set_matrix_bit
				( 4 * j  +  3, 2 * j + 2 );
			continue;
		}
		if ( k == 1 )
		{
			B.set_matrix_bit
				( 4 * j, 2 * j  );
			continue;
		}
		if ( k == 2 )
		{
			B.set_matrix_bit
				( 4 * j  +  1, 2 * j + 1 );
			continue;
		}
		if ( k == 3 )
		{
			B.set_matrix_bit
				( 4 * j  +  2, 2 * j  +  2 );

			continue;
		}
		cerr << "\nerror 1 in generate_key_B_with_noise";
	}
}
*/



void generate_B_with_noise
		(
			BitMatrix& B,
			int (*specified_rand)(void)
		)

{

	int b,i,j,l,m,n,p,as;
	B.reset();
	as = B.number_of_rows/4;  // number_of_rows/4 = padded_length/3
				// 3*number_of_rows   = 4*padded_length
				// number_of_columns = unpadded_length
	for (i=0, j=0;   j<as;   ++j, i = i + 4)
	{
		for (m=0;   (j+m*as) < B.number_of_columns;  ++m)
		{


			B.set_matrix_bit (i+m, j+as*m);
			B.set_matrix_bit (i+3, j+as*m);


			for (l=(j+1)%as;   l!=0;   l=(l+1)%as)
			{


				for (n=0; (l+n*as)<B.number_of_columns; ++n)
				{
					if (1 & specified_rand())
						B.set_matrix_bit
						    (i+m, as*n+l);

					if (m==2 && 1 & specified_rand())
						B.set_matrix_bit
						    (i+3, as*n+l);


				}


			}


		}


	}
/*
	for (p=0;   p<B.number_of_columns;   ++p)
	{
		for (i=0;   i<B.number_of_rows;   i += 4)
		{
			for (b=0, m=0;   m<3;   ++m)
				b ^= B.test_bit_set (i+m, p);
			if (b)
				B.set_matrix_bit (i+3, p);
		}
	}
*/
}



// generate the "replacement_rows" BitMatrix
//
// NOTE:
//
// the purpose of replacement_rows BitMatrix is to provide random values
// for eliminating the "sea of zeroes" under the diagonal of submatrices
// in matrix T
//
// each randomly generated replacement row is replicated four times for
// each of the four rows associated with a template submatrix
//
// each different replacement row value is generated a word at a time
// by a specified random number generator
//
// all bit positions are used
//
// the final replacement row value for the bottom-most submatrix is generated
// as the modulo 2 sum all the other replacement row values to insure that
// they all sum to zero and therefore "cancel out" automatically in the
// encryption process
//
void generate_replacement_rows

		(
				BitMatrix& replacement_rows,
				int ( *specified_rand ) ( void )
		)
{
	int i,j,k,l, sum;

	for ( j = 0; j < replacement_rows.row_width; ++j )
	    // do a word at a time so that only one "sum" variable is needed
	{
		sum = 0;
		for		// for every replacement row template set
			(                        //dont do last row set
			 i=0;
			 i < ( replacement_rows.number_of_rows / 4  -  1 );
			 ++i
			)
		{
			l = specified_rand();
			for ( k = 0; k < 4; ++k )  // for each of the 4 rows
			{
				*(replacement_rows.bm
					+ 4 * i	* replacement_rows.row_width
					+ j
					+ k * replacement_rows.row_width )

				= l;
			}
			sum ^= l;	// (check) sum this generated word
		}

		for ( k = 0; k < 4; ++k ) // for the last temmplate set
		{
			*(replacement_rows.bm
                                                  // dont do last row set
				+ 4 * i * replacement_rows.row_width
				+ j
				+ k * replacement_rows.row_width )
			= sum;		  // assign it the (check) sum

		}
	}
}

// add (XOR) a bit matrix to a bit matrix producing a third matrix
//
// NOTES:
//
// each corresponding word in the two BitMatrices (replacement_rows and
// key_T_with_noise) are added together with the results stored in
// corresponding words in a third BitMatrix (key_T_replaced)
//
// this function is used to insert the replacement rows into the T key 
// bit matrix (so called because they "replace" the zeroes otherwise stationed
// below the diagonal) 

void add_matrix_matrix
			(
				BitMatrix& key_T_replaced,
				BitMatrix& replacement_rows,
				BitMatrix& key_T_with_noise
			)
{
	for ( int i = 0; i < key_T_replaced.matrix_size; ++i )
			// for every word in the matrices
	{
		*(key_T_replaced.bm + i) =
			*(replacement_rows.bm + i)
				^                      // (xor)
			*(key_T_with_noise.bm + i);
	}

// ****************** end the old BitMatrix body ***************************
}
//      new copy matrix to matrix function

void copy_matrix_matrix
        (
                BitMatrix& target,
		BitMatrix& source
        )
{
        if ( target.matrix_size != source.matrix_size )
	{
                cerr << "\n BitMatrix sizes do not match "
                     << "target.matrix_size = " << target.matrix_size
                     << "source.matrix_size = " << source.matrix_size
                     << "\n";
                exit (-1);
        }
        for ( int i = 0; i < target.matrix_size; ++i)
            *(target.bm+i) = *(source.bm+i);
}




// ****************** add ons to BitMatrix body ****************************

void write_BitMatrix (FILE* output, BitMatrix& bitm)
{
        putw(bitm.number_of_rows, output);
        putw(bitm.number_of_columns, output);
	putw(bitm.row_width, output);
        putw(bitm.matrix_size, output);
        for (int i = 0; i< bitm.matrix_size; ++i)
            putw( *(bitm.bm+i), output );
}



void read_BitMatrix (FILE* input, BitMatrix& bitm)
{
        int work;

        work = getw (input);
        if (work != bitm.number_of_rows)
        {
		cout << work << " " <<bitm.number_of_rows << " check input\n";
		exit (-2);
        }
        work = getw (input);
        if (work != bitm.number_of_columns)
	{
		cout << "check input\n";
                exit (-2);
        }
        work = getw (input);
        if (work != bitm.row_width)
        {
		cout << "check input\n";
                exit (-2);
        }
        work = getw (input);
        if (work != bitm.matrix_size)
        {
		cout << "check input\n";
                exit (-2);
        }
	for (int i=0; i<bitm.matrix_size; ++i)
            *(bitm.bm+i) = getw(input);
}

// ********* new BitMatrix functions


void set_BitMatrix (BitMatrix& bitm, unsigned int* array)
{
        for (int i = 0; i < bitm.matrix_size; ++i)
            *(bitm.bm + i) = *(array + i);
}

void inc_BitMatrix (BitMatrix& bitm)
{
        unsigned int j = 1;
        for (int i = 0; i < bitm.matrix_size && j == 1; ++i)
	{
		j = *(bitm.bm + i) + j;
		*(bitm.bm + i) = j;
		if (j) j = 0;
		else j = 1;
	}
}

void get_BitMatrix (BitMatrix& bitm,
		    unsigned int* array)
{
	for (int i = 0; i < bitm.matrix_size; ++i)
	    *(array + i) = *(bitm.bm + i);
}




void copy_to_part_of_matrix
		(
			BitMatrix& target,
			BitMatrix& source,
			int row_number,
			int column_number
		)
{
	int m,n,i,j;
	for (m=row_number, i=0;
	     (m<target.number_of_rows) && (i<source.number_of_rows);
	     ++m, ++i)
	{
		for (n=column_number, j=0;
		      (n<target.row_width) && (j<source.row_width);
		      ++n, ++j)
		{
			*(target.bm + m*target.row_width + n)
					=
			*(source.bm + i*source.row_width + j);
		}
	}
}


void expand_A ( BitMatrix& A_expanded, BitMatrix& A )
{
	int i,j,k;
	A_expanded.reset();
	for (i=0, k=0; i < A.number_of_columns; i += 2, k += 4)
	{
		for (j=0; j<A.number_of_columns; ++j)
		{
			if ( A.test_bit_set(i+1,j) )
			{
				A_expanded.set_matrix_bit
					(k, j+2*A.number_of_columns);
				A_expanded.set_matrix_bit
					(k, j+A.number_of_columns);
				A_expanded.set_matrix_bit
					(k, j);
			}
			if ( A.test_bit_set(i,j) )
			{
				A_expanded.set_matrix_bit
					(k+1, j+2*A.number_of_columns);
				A_expanded.set_matrix_bit
					(k+1, j+A.number_of_columns);
				A_expanded.set_matrix_bit
					(k+1, j);
			}
			if ((A.test_bit_set(i,j) && A.test_bit_reset(i+1,j))
					       ||
			    (A.test_bit_reset(i,j) && A.test_bit_set(i+1,j)) )
			{
				A_expanded.set_matrix_bit
					(k+2, j+2*A.number_of_columns);
				A_expanded.set_matrix_bit
					(k+2, j+A.number_of_columns);
				A_expanded.set_matrix_bit
					(k+2, j);
			}
		}
	}
}



// ******************** begin Crypto body **********************************

// the constructor for a Crypto object:
//  sets up the variable, block_length, with a specified length and 
//  allocates space for all of the BitMatrices used by a Crypto object

Crypto::Crypto ( int length )
{
	unpadded_length = length;
	padded_length = (length % 6)? (1 + length/6)*6: length;
	key_M = new BitMatrix ( unpadded_length, unpadded_length );
	key_M_inverse = new BitMatrix ( unpadded_length, unpadded_length );
	key_I = new BitMatrix ( unpadded_length, unpadded_length );
	key_T_with_noise = new BitMatrix ( 2*padded_length, unpadded_length );
	key_Jumble = new BitMatrix ( ( 2*unpadded_length )/4, WORDSIZE );
	key_T_to_be_jumbled = new BitMatrix ( 2 * padded_length, unpadded_length );
	key_T_replaced = new BitMatrix ( 2 * padded_length, unpadded_length );
	public_key_K = new BitMatrix ( 2*padded_length, unpadded_length );
	replacement_rows = new BitMatrix ( 2*padded_length, unpadded_length );

	A = new BitMatrix( padded_length/3, padded_length/3 );
	A_inverse = new BitMatrix( padded_length/3, padded_length/3 );
	A_expanded = new BitMatrix( 2*padded_length/3, padded_length );
	B = new BitMatrix( 4*padded_length/3, unpadded_length );

	X = new BitMatrix( 1, padded_length/3 );
	X_decoded = new BitMatrix ( 1, padded_length/3 );

	unpadded_plain_text = new BitMatrix ( 1, unpadded_length );
	// un_padded_plain_text is copied to plain_text
	plain_text = new BitMatrix ( 1, padded_length );
	// plain_text is expanded to produce expanded_plain_text
	expanded_text = new BitMatrix ( 1, 2*padded_length );
	// multplying the expanded_text by M produces the encrypted_text
	encrypted_text = new BitMatrix ( 1, unpadded_length );

	// multplying the encrypted_text by M_inverse produces the reverted_text
	reverted_text = new BitMatrix ( 1, unpadded_length ); // newly defined
	// the reverted_text is copied to the intermediate_text (padding it)
	intermediate_text = new BitMatrix ( 1, padded_length );
	// deconvoluting the intermediate_text produces the resultant_text
	resultant_text = new BitMatrix ( 1, padded_length );
	// unjumbling resultant_text produces the decrypted_text
	decrypted_text = new BitMatrix ( 1, length );
	// Work_M is used to calucate M_inverse
	Work_M = new BitMatrix ( padded_length, padded_length);
	// Work_A is used to calucate A_inverse
	Work_A = new BitMatrix ( padded_length/3, padded_length/3);
}

// the destructor for a Crypto object:
//  clears the block_length and
//  deallocates the space used by the Crypto object

Crypto::~Crypto ( void )
{
	padded_length = 0;
	unpadded_length = 0;

	delete [ key_M_inverse->matrix_size ] key_M_inverse->bm;
	delete [ key_M->matrix_size ] key_M->bm;
	delete [ key_I->matrix_size ] key_I->bm;
	delete [ key_T_with_noise->matrix_size ] key_T_with_noise->bm;
	delete [  key_Jumble->matrix_size ] key_Jumble->bm;
	delete [ key_T_to_be_jumbled->matrix_size ] key_T_to_be_jumbled->bm;
	delete [ key_T_replaced->matrix_size ] key_T_replaced->bm;
	delete [ intermediate_text->matrix_size ] intermediate_text->bm;
	delete [ public_key_K->matrix_size ] public_key_K->bm;
	delete [ replacement_rows->matrix_size ] replacement_rows->bm;

	delete [ A->matrix_size ] A->bm;
	delete [ A_inverse->matrix_size ] A_inverse->bm;
	delete [ A_expanded->matrix_size ] A_expanded->bm;
	delete [ B->matrix_size ] B->bm;


	delete [ X->matrix_size ] X->bm;
	delete [ X_decoded->matrix_size ] X_decoded->bm;

	delete [ unpadded_plain_text->matrix_size ] unpadded_plain_text->bm;
	delete [ plain_text->matrix_size ] plain_text->bm;
	delete [ expanded_text->matrix_size ] expanded_text->bm;
	delete [ encrypted_text->matrix_size ] encrypted_text->bm;
	delete [ reverted_text->matrix_size ] reverted_text->bm; // new
	delete [ intermediate_text->matrix_size ] intermediate_text->bm;
	delete [ resultant_text->matrix_size ] resultant_text->bm;
	delete [ decrypted_text->matrix_size ] decrypted_text->bm;

	delete [ Work_M->matrix_size ] Work_M->bm;
	delete [ Work_A->matrix_size ] Work_A->bm;
}
// set plain_text to a specified value

void Crypto::set_plain_text ( BitMatrix &V )
{
	if ( V.number_of_rows != 1 )
	{
		cerr << "\n not a vector\n";
		exit (-1);
	}
	if ( V.number_of_columns != unpadded_length )
	{
		cerr << "\n vector not right length \n";
		exit (-1);
	}
	for (int i=0; i < V.matrix_size; ++i)
		*((*unpadded_plain_text).bm + i)= *(V.bm + i);
}


// reads a plaintext block (to be encrypted) from a specified FILE
// or returns an EOF indication if at EOF (to start with).

int Crypto::read_block_of_plain_text (FILE* file_name)
{

	int c;
	if ( ( c = (*unpadded_plain_text).number_of_columns) == 0 )
	{
		cerr << "Crypto object not initialize properly\n";
		exit (-3);
	}
	if ( ( c % 8 ) != 0 )
	{
		cerr << "read_block_of_plain_text currently only deals with"
		     << "byte FILES\n";
		exit (-3);
	}
	c = c / 8;
	(*unpadded_plain_text).reset();
	unsigned int comb_char;
	int i,j;
	for (i = 0; i*BYTESPERWORD < c; ++i)
	    *((*unpadded_plain_text).bm + i) = 0;
	for ( i = 0, j = 0, comb_char = 0; i*BYTESPERWORD + j < c;  )
	{
		unsigned int one_char = fgetc(file_name);
		if (one_char == EOF)
		{
			if ( i | j )
			{
				if (j==0) comb_char = 0;
				*((*unpadded_plain_text).bm + i) = comb_char;
				return ( 0 );
			}
			return (EOF);
		}
		if (j==0) comb_char = 0;
		comb_char = comb_char | (one_char << (j*8) );
		*((*unpadded_plain_text).bm + i) = comb_char;
		j = (j+1) % BYTESPERWORD;
		if (j==0)
		{
		   ++i;
		}
	}
}


// set encrypted_text to a specified value

void Crypto::set_encrypted_text ( BitMatrix &V )

{
	if ( V.number_of_rows != 1 )
	{
		cerr << "\n not a vector\n";
		exit (-1);
	}
	if ( V.number_of_columns != unpadded_length )
	{
		cerr << "\n vector not right length \n";
		exit (-1);
	}
	for (int i=0; i < V.matrix_size; ++i)
		*((*encrypted_text).bm + i)= *(V.bm + i);
}

// reads a encrypted_text block (to be decrypted) from a specified FILE
// or returns an EOF indication if at EOF (to start with).

int Crypto::read_block_of_encrypted_text (FILE* file_name)
{
	int c;
	if ( ( c = (*encrypted_text).number_of_columns) == 0 )
	{
		cerr << "Crypto object not initialize properly\n";
		exit (-3);
	}
	if ( (  c % 8 ) != 0 )
	{
		cerr << "read_block_of_encrypted_text currently only deals"
		     << "with byte FILES\n";
		exit(-3);
	}
	c = c / 8;
	unsigned int comb_char;
	int i,j;
	for (i = 0; i*BYTESPERWORD < c; ++i)
		*((*encrypted_text).bm + i) = 0;
	for ( i = 0, j = 0, comb_char = 0;  i*BYTESPERWORD + j < c; )
	{
		unsigned int one_char = fgetc(file_name);
/*
cout << (char) one_char;
*/
		if (one_char == EOF)
		{
/*
cout << "EOF";
*/
			if ( i | j )
			{
				if (j==0) comb_char = 0;
				*((*encrypted_text).bm + i) = comb_char;
				return ( 0 );
			}
                        return (EOF);
                }
                if (j==0) comb_char = 0;
                comb_char = comb_char | (one_char << (j*8) );
		*((*encrypted_text).bm + i) = comb_char;
                j = (j+1) % BYTESPERWORD;
                if (j==0)
                {
                   ++i;
                }
        }
}

// writes a decrypted_text block (which has been decrypted) to a specified FILE

void Crypto::write_block_of_decrypted_text (FILE* file_name)
{
	int c;
	if ( ( c = (*decrypted_text).number_of_columns) == 0 )
	{
		cerr << "Crypto object not initialize properly\n";
		exit (-3);
	}
	if ( ( c % 8 ) != 0 )
	{
		cerr << "write_block_of_decrypted_text currently only deals "
		     << "with byte FILES\n";
		exit(-3);
	}
	c = c / 8;
	unsigned int one_char;
	int i,j;
	for ( i = 0, j = 0;  i*BYTESPERWORD + j < c; )
	{
		if (j==0)
		   one_char = *((*decrypted_text).bm + i) ;
		fputc((one_char ) & 0xFF, file_name);
		one_char = one_char >> 8;
                j = (j+1) % BYTESPERWORD;
                if (j==0) ++i;
        }
}

// writes a encrypted_text block (which has been decrypted) to a specified FILE

void Crypto::write_block_of_encrypted_text (FILE* file_name)

{
        int c;
        if ( ( c = (*encrypted_text).number_of_columns) == 0 )
        {
                cerr << "Crypto object not initialize properly\n";
                exit (-3);
        }
        if ( ( c % 8 ) != 0 )
        {
                cerr << "write_block_of_encrypted_text currently only deals"
                     << "with byte FILES\n";
                exit (-3);
        }
        c = c / 8;
        unsigned int one_char;
        int i,j;
        for ( i = 0, j = 0;  i*BYTESPERWORD + j < c; )
	{
                if (j==0)
                   one_char = *((*encrypted_text).bm + i) ;
                fputc( (one_char )  & 0xFF, file_name);
                one_char = one_char >> 8;
                j = (j+1) % BYTESPERWORD;
                if (j==0) ++i;
        }
}


// set decrypted_text to a specified value



// fetch the encrypted text block (to be decrypted) to a specified block

void Crypto::get_encrypted_text ( BitMatrix& V)
{
	if ( V.number_of_rows != 1 )
	{
		cerr << "\n not a vector\n";
		exit (-1);
	}
	if ( V.number_of_columns != unpadded_length )
	{
		cerr << "\n vector not right length \n";
		exit (-1);
	}
	for (int i=0; i < V.matrix_size; ++i)
		 *(V.bm + i) = *((*encrypted_text).bm + i);
}
// fetch the decrypted text block to a specified block

void Crypto::get_decrypted_text ( BitMatrix& V )
{
	if ( V.number_of_rows != 1 )
	{
		cerr << "\n not a vector\n";
		exit (-1);
	}
	if ( V.number_of_columns != unpadded_length )
	{
		cerr << "\n vector not right length \n";
		exit (-1);
	}
	for (int i=0; i < V.matrix_size; ++i)
		 *(V.bm + i) = *((*decrypted_text).bm + i);
}


// encrypt the already specified plain_text block
//
// NOTE:
//
// encryption of plaintext using the public key is accomplished by:
//
//  first expanding each two-bit plaintext segment (starting every other bit)
//  to four bits according to the following translation table:
//
//           two-bit         four-bit
//           segment         expansion
//
//              00              1000
//              01              0100
//              10              0010
//              11              0001
//
// (for example, the plaintext bit string 00011011 is expanded to the value
// 1000010000100001)
//
// and second, by multiplying the expanded_text by the public_key_K to produce
// the encrypted_text

void Crypto::encrypt ( void )
{
	(*plain_text).reset();
	copy_to_part_of_matrix
		(*plain_text, *unpadded_plain_text, 0, 0);
	expand_text ();                       // expand the plain_text
	multiply_vector_matrix                // multiply the expanded_text
		(                             // by the public_key_K producing
			*encrypted_text,      // the encrypted_text
			*expanded_text,
			*public_key_K
		);
}
// decrypt the already specified encrypted_text block by the following steps:
//
// first, by multiplying the encrypted_text by the first private key 
// (key_M_inverse) producing the intermediate_text
//
// second, by "deconvoluting" the intermediate_text (removing the noise bits)
// using the second private key (key_T_with_noise) producing the 
// resultant_text --
//
//   the deconvolution is performed by:
//
//   0)  using the first two bits, which are "in the clear", as the "next two
//       bits" for the first iteration below
//
//   1)  using the "next two bits" in the intermediate_text to determine the
//       next two bits to be placed, without any translation into the resultant
//       text
//
//   2)  using the same "next two bits" bits to determine which row in 
//       key_T_with_noise must be used to remove from the intermediate_text,


//       the noise and row identifier bits originally injected by the subject
//       bit pair in the encryption process
//
//   3)  obtaining a new two bits ("in the clear") in the intermediate_text

//       by steps 1) and 2) above and using these as the "next two bits"
//       in the next iteration
//
//   4)  repeating steps 1), 2), and 3) until all the bits are "in the clear"
//
// third, by unjumbling the deconvoluted bits (using the key_Jumble, to
// reverse the row jumbling process performed during encryption) to produce
// the final decrypted_text

void Crypto::decrypt ( void )
{
	multiply_vector_matrix              // multiply the encrypted_text
		(                           // by the key_M_inverse
			*reverted_text,     // obtaining the reverted_text
			*encrypted_text,
			*key_M_inverse

		);
//cout << "\ndid mult v m";
	(*intermediate_text).reset();
	copy_to_part_of_matrix(*intermediate_text, *reverted_text,0,0);
//cout << "\ndid copy yo inter.";
	pad_intermediate_text();


	deconvolute_text();                 // using the intermediate_text
					    // deconvolute_text producing the
					    // resultant_text

//cout << "\ndid deconvolute";

	unjumble();                         // unjumble the resultant_text
//cout <<"\ndid unjumble";
}
// expand the plaintext
//
// expand each two-bit plaintext segment (starting every other bit)
// to a four-bit segment according to the translation table below:
//
//           two-bit         four-bit            
//           segment         expansion
//
//              00              1000
//              01              0100
//              10              0010
//              11              0001
//
// (for example the bit string 00011011 is expanded to 1000010000100001)
//
void Crypto::expand_text ( void )
{
	(*expanded_text).reset();                       // reset the
                                                        // expanded_text
	for ( int i = 0; i < unpadded_length; ++i, ++i )   // for each two bits
	{
                                                     // determine their value
						     // and set the


                                                     // appropriate bit
                                                     // in the expanded_text

		if ( (*plain_text).test_bit_reset ( 0, i) )
		{
			if ( (*plain_text).test_bit_reset ( 0, i + 1 ) ) //00
			{       // 0001
				(*expanded_text).set_matrix_bit ( 0, 2*i + 3 );
			}
			else                                             //01
			{       // 1000
				(*expanded_text).set_matrix_bit ( 0, 2*i );
			}
		}
		else
		{
			if ( (*plain_text).test_bit_reset ( 0, i + 1 ) ) //10
			{       // 0100
				(*expanded_text).set_matrix_bit ( 0, 2*i + 1 );
			}
			else                                             //11
			{       // 0010

				(*expanded_text).set_matrix_bit ( 0, 2*i + 2 );
			}
		}
	}

}
// deconvolute the intermediate_text
//
//  "deconvoluting" the intermediate_text (using the key_T_with_noise)
//   produces the resultant_text -- the deconvolution is performed by:
//
//   0)  using the first two bits, which are always "in the clear", as the
//       "next two bits" for the first iteration below
//
//   1)  using the "next two bits" in the intermediate_text to determine the
//       next two bits to be placed, without any translation, into the
//       resultant text
//
//   2)  using the same "next two bits" bits to determine which row of
//       key_T_with_noise must be used to remove from the intermediate_text,
//       the noise and row identifier bit displacements originally injected
//       by the subject two bits in the encryption process
//
//   3)  obtaining a new two bits ("in the clear") in the intermediate_text
//       by steps 1) and 2) above and using these as the "next two bits"
//       in the next iteration
//
//   4)  repeating steps 1), 2), and 3) until all the bits are "in the clear"



/*


void Crypto::deconvolute_text ( void )
{

	int j;
	(*resultant_text).reset();                // reset the resultant_text

	for ( j = 0; j < block_length; j++, j++ ) // for every two bits

	{                                         // find the value of the
						  // two bits "in the clear"

		if ( (*intermediate_text).test_bit_reset ( 0, j ) )
		{
			if ( (*intermediate_text).test_bit_reset ( 0, j + 1 ) )
			{                                        // 00
				remove_T_row_noise
					(
						*intermediate_text,
						0,
						*key_T_with_noise,
						2*j
					);
				continue;
			}
			else                                    // 01
			{
				remove_T_row_noise
					(
						*intermediate_text,
						0,
						*key_T_with_noise,
						2*j + 1
					);

				(*resultant_text).set_matrix_bit ( 0, j + 1 );
				continue;
			}
		}
		else
		{
			if ( (*intermediate_text).test_bit_reset ( 0, j +1 ) )
			{                                       // 10
				remove_T_row_noise
					(
						*intermediate_text,
						0,
						*key_T_with_noise,

						2*j + 2
					);
				(*resultant_text).set_matrix_bit ( 0, j );

				continue;
			}
			else                                    // 11
			{

				remove_T_row_noise
					(
						*intermediate_text,
						0,
						*key_T_with_noise,

						2*j + 3
					);

				(*resultant_text).set_matrix_bit ( 0, j );
				(*resultant_text).set_matrix_bit ( 0, j + 1 );
				continue;
			}
		}
	}
}
*/





/// new deconvolute_text for trinity

void Crypto::deconvolute_text ( void )
{
	int i,j,k,n,m,A_length;
	A_length = padded_length/3;
	(*X).reset();
	(*resultant_text).reset();
	for (n=0; n < A_length; n = n +1)
	{
		i = j = k = 0;
		if ( (*intermediate_text).test_bit_set(0,n) )
			i = 1;
		if ( (*intermediate_text).test_bit_set (0,A_length+n) )
			j = 1;
		if ( (*intermediate_text).test_bit_set(0,2*A_length+n) )
			k = 1;
		if ( i == j )
		{
			if ( i == k )
			{
				if ( !i )
					(*X).set_matrix_bit (0,n);
				remove_T_row_noise
					(*intermediate_text,0,
					 *key_T_with_noise, 4*n + 3);
			}
			else
			{
				if (i)
					(*X).set_matrix_bit (0,n);
				(*resultant_text).set_matrix_bit(0,2*n);
				(*resultant_text).set_matrix_bit(0,2*n+1);
				remove_T_row_noise
					(*intermediate_text,0,
					 *key_T_with_noise, 4*n + 2);
			}
		}
		else if ( i == k )
		{
			if (i)
				(*X).set_matrix_bit(0,n);
			(*resultant_text).set_matrix_bit(0,2*n);
			remove_T_row_noise
				(*intermediate_text,0,
				 *key_T_with_noise, 4*n + 1);
		}
		else
		{
			if (!i)
				(*X).set_matrix_bit(0,n);
			(*resultant_text).set_matrix_bit(0,2*n+1);
			remove_T_row_noise
				(*intermediate_text,0,
				 *key_T_with_noise, 4*n );
		}
	} // end for

	multiply_vector_matrix(*X_decoded, *X, *A_inverse);
	for (n=2*padded_length/3, m=0; n<unpadded_length; ++n, ++m)
	{
		if ((*X_decoded).test_bit_set(0,m))
			(*resultant_text).set_matrix_bit(0,n);
	}
}


// the following pads the intermediate text with bits the make
// the deconvolution process come out correctly

void Crypto::pad_intermediate_text ( void )
{
/*
printf("\npad_intermediate_text");

	if ( (unpadded_length%3) == 1 )
	{
		(*intermediate_text).set_matrix_bit(0, unpadded_length);
		if ((*reverted_text).test_bit_set(0, unpadded_length - 1))
		{
			(*intermediate_text).set_matrix_bit
				(0, unpadded_length + 1);
		}
	}
	else if ( (unpadded_length%3) == 2 )
	{
		if ((*reverted_text).test_bit_set(0, unpadded_length - 2)
					&&
		    (*reverted_text).test_bit_set(0, unpadded_length - 1) )
		{
			(*intermediate_text).set_matrix_bit
				(0, unpadded_length + 1);
		}
		if ((*reverted_text).test_bit_reset(0, unpadded_length - 2)
					&&
		    (*reverted_text).test_bit_reset(0, unpadded_length - 1) )
		{
			(*intermediate_text).set_matrix_bit
				(0, unpadded_length + 1);
		}
	}
print_intermediate_text();
*/
}


// unjumble the resultant_text, producing the decrypted_text
//
// the unjumbling process reverses the encryption jumbling process
//
// the jumbling process is equivalent to multiplying by an I (identity)
// matrix whose rows have been shuffled four at a time (sometimes called a
// "permutation matrix") ignoring the replacement rows or bits (and they can
// be ignored here)
//
// to better understand the unjumbling (and jumbling) process, the encryption
// process is reviewed here first:
//
//   the encryption process can be expressed in two steps
//
//   step 1)    P -> X
//        where P is the plain_text
//        and   X is the expanded_text
//        step 1) says that the plain_text is expanded into the expanded_text
//
//   step 2)    X * K -> E
//        where X is the expanded_text
//        and   K is the public_key_K
//
//   but        K = J * T * M
//        where K is the public_key_K
//        and   J is the key_Jumble
//        and   T is the key_T_with_noise (ignoring the replacement_rows)
//        and   M is the non-inversed key matrix of the generated matrix pair
//                ( key_M * key_M_inverse = I)
//
//   so step 2) could be expressed as
//
//              X * ( J * T * M ) -> E
//
//   and rewritten as
//
//              ( X * J ) * ( T * M ) -> E


//   combining this results with step 1) gives
//
//              ( expansion ( P ) * J ) * ( T * M ) -> E
//
// with these relationships in mind, a quick review of the decryption process
//
// shows that it is composed of the following steps:
//
//   step 1)    E * inverse (M) -> N
//
//   step 2)    denoise (N) -> R
//
//      where the function "denoise" removes the noise bits from N, the
//      intermediate_text, and produces, R, the resultant_text, which is not
//      in expanded form (since the two-bit to four-bit expansion vanishes in
//      the encryption process)
//
//   step 3)    unjumble (R) -> D
//

//      where the function "unjumble" unjumbles the resultant_text, R,
//      producing the decrypted_text, D
//
// with these reviews, it can be seen that
//
//      1)      compression ( expansion ( P ) * J ) = R
//
//                 where compression is the reverse of the expansion process
//
//      2)      compression ( expansion ( P ) )  *  compression (J) = R
//
//                 where compression ( J ) is just a smaller identity matrix
//                 shuffled two rows at a time instead of four but with the
//                 same "J pattern"
//
//                 let j = compression (J)
//
//      3)      P * j = R
//
//                 where j is still a matrix (with half as many rows as J)
//
//      4)      P * j * inverse (j) = R * inverse (j)
//
//      5)      P * I = R * inverse (j)
//
//                 since j * inverse (j) = I
//
//      6)      P = R * inverse (j)
//
//                      since P * I = P
//
//      7)      P = R * transpose (j)
//
//                 where the transpose (j) means to make the
//                 columns of j into the rows of j (or vice versa)

//                 ( it turns out that
//                     inverse (j) = transpose (j)
//                   when j is a jumble of the I matrix)
//
//      8)      D = R * transpose (j)
//
//                 since D, the decrypted_text, is equal to P, the plain_text
//
// these results suggest a fairly simple algorithm for the unjumble function.
//
// However, the actual algorithm differs somewhat for reasons of efficiency:
// both the J and j matrices are stored as a single list of numbers
// (in the key_Jumble); to unjumble the bits in R, almost the same process is
// used as was used to jumble the rows of key_T_to_be_jumbled except that it
// deals with the bits of R instead of the rows of key_T_to_be_jumbled
// that are unjumbled (instead of jumbled)
//
// (the key_Jumble is a post-multipler to R, but a pre-multipler to 
// key_T_to_be_jumbled and going from pre-  to post-  multiplication is 
// another transpose operation for a jumbled I matrix)
//
// R is unjumbled two bits at a time according to specifications stored in
// "key_Jumble", that is, the first value in "key_Jumble" specifies from where

// the first two bits are to be obtained, etc. the number (cast as a bit
// matrix value) in the "key_Jumble" specifies which set of two bits to obtain 
// to place in the next two bit slot (beginning with the first set of two 
// bits, etc.)
//
// for example, if the bits to be unjumbled are labeled:
//
//      A
//      D
//      B
//      C
//
// and the key_Jumble specifies:
//
//      0 2 3 1
//
// then the bit sets will be jumbled to produce:
//
//      A
//      B
//      C
//      D
//
void Crypto::unjumble ( void )
{
	int i;
	int j = (2*unpadded_length)/4;
	(*decrypted_text).reset();

	for ( i = 0; i < j; ++i)  // for every two bit positions
	{

		if ( (*resultant_text).test_bit_set
				( 0, 2*( *( (*key_Jumble).bm + i) ) )
		   )            // test the first bit indicated by the
                                // corresponding entry in key_Jumble
		{
			(*decrypted_text).set_matrix_bit
				( 0, 2*i );
       		}               // and set the bit in
                                // the current bit position

		if ( (*resultant_text).test_bit_set
				( 0, 2*( *( (*key_Jumble).bm + i) ) +1 )
		   )            // test the second bit indicated by the
				// corresponding entry in key_Jumble
		{
			(*decrypted_text).set_matrix_bit
				( 0, 2*i+1 );
		}               // and set the bit in
                                // the current bit position (+1)
                                // if the test results was true
	}
}
// **** generate all system keys required to encrypt and decrypt*********

void Crypto::generate_keys ( int (*specified_rand) ( void ) )
{
	generate_jumble_matrix ( *key_Jumble, specified_rand );
	generate_replacement_rows ( *replacement_rows, specified_rand );
	make_pair ( *key_M, *key_M_inverse, specified_rand, *Work_M );
	multiply_matrix_matrix ( *key_I, *key_M, *key_M_inverse );

	generate_B_with_noise ( *B,specified_rand);

	make_pair ( *A, *A_inverse, specified_rand, *Work_A);

	(*key_T_with_noise).reset();

	copy_to_part_of_matrix ( *key_T_with_noise, *B, 0, 0 );

	expand_A ( *A_expanded, *A);

	copy_to_part_of_matrix
		(*key_T_with_noise, *A_expanded,
		 4*padded_length/3, 0);


	add_matrix_matrix
		(
			*key_T_replaced,
			*replacement_rows,
			*key_T_with_noise
		);


	multiply_matrix_matrix
		(

			*key_T_to_be_jumbled,
			*key_T_replaced,
			*key_M
		);
	jumble_row_groups_of_matrix
		(

			*public_key_K,
			*key_T_to_be_jumbled,
			*(key_Jumble)
		);
}


// the following is a set of functions for printing
// the various keys and text of a Crypto object:

void Crypto::print_X ()
{

	cout << "\nX is";
	(*X).print_matrix();
 }

void Crypto::print_X_decoded ()
{

	cout << "\nX_decoded is";
	(*X_decoded).print_matrix();
 }


void Crypto::print_B ()
{

	cout << "\nB is";
	(*B).print_matrix();
 }



void Crypto::print_A ()
{

	cout << "\nA is";
	(*A).print_matrix();
 }


void Crypto::print_A_inverse ()
{

	cout << "\nA_inverse is";
	(*A_inverse).print_matrix();
 }


void Crypto::print_A_expanded ()
{

	cout << "\nA_expanded is";
	(*A_expanded).print_matrix();
 }



void Crypto::print_public_key_K ()
{

	cout << "\npublic_key_K is";
	(*public_key_K).print_matrix();
 }
void Crypto::print_key_M ()
{
	cout << "\nkey_M is";
	(*key_M).print_matrix();
 }
void Crypto::print_key_M_inverse ()
{
	cout << "\nkey_M_inverse is";
	(*key_M_inverse).print_matrix();
 }
void Crypto::print_replacements_rows ()
{
	cout << "\nreplacement_rows are";
	(*replacement_rows).print_matrix();
 }
void Crypto::print_key_T_with_noise  ()

{
	cout << "\nkey_T_with_noise is";
	(*key_T_with_noise).print_matrix();
 }
void Crypto::print_key_T_replaced ()
{
	cout << "\nkey_T_replaced is";
	(*key_T_replaced).print_matrix();
 }
void Crypto::print_key_T_to_be_jumbled ()
{
	cout << "\nkey_T_to_be_jumbled is";
	(*key_T_to_be_jumbled).print_matrix();
 }
void Crypto::print_key_Jumble ()
{
	cout << "\nkey_Jumble is";
	(*key_Jumble).print_matrix();
 }
void Crypto::print_key_I ()
{
	cout << "\nkey_I is";
	(*key_I).print_matrix ();
}
void Crypto::print_unpadded_plain_text()
{
	cout << "\nunpadded_plain_text_is";
	(*unpadded_plain_text).print_matrix();
 }

void Crypto::print_plain_text()
{
	cout << "\nplain_text is";
	(*plain_text).print_matrix ();
 }
void Crypto::print_expanded_text()
{
	cout << "\nexpanded_text is";
	(*expanded_text).print_matrix ();
 }
void Crypto::print_encrypted_text()
{
	cout << "\nencrypted_text is";
	(*encrypted_text).print_matrix ();
 }
void Crypto::print_reverted_text()
{
	cout << "\nreverted_text is";
	(*reverted_text).print_matrix();
 }
void Crypto::print_intermediate_text()
{
	cout << "\nintermediate_text is";
	(*intermediate_text).print_matrix ();
 }
void Crypto::print_resultant_text()
{
	cout << "\nresultant_text is";
	(*resultant_text).print_matrix ();
 }
void Crypto::print_decrypted_text()
{
	cout << "\ndecrypted_text is";
	(*decrypted_text).print_matrix ();

// ************************ end the old Crypto body ***********************
}

// ************************* start the new portion of Crypto body *********


// writes the public key

void write_public_key (FILE* file_name, Crypto& to_write)
{
        write_BitMatrix (file_name, *(to_write.public_key_K) );
}

// writes the private and public keys

void write_private_and_public_keys (FILE* file_name, Crypto& to_write)
{
        write_BitMatrix (file_name, *(to_write.public_key_K) );
        write_BitMatrix (file_name, *(to_write.key_M_inverse) );
        write_BitMatrix (file_name, *(to_write.key_T_with_noise) );
        write_BitMatrix (file_name, *(to_write.key_Jumble) );

}

// reads the public key

void read_public_key (FILE* file_name, Crypto& to_read)
{
        read_BitMatrix (file_name, *(to_read.public_key_K) );

}


// reads the private and public keys

void read_private_and_public_keys (FILE* file_name, Crypto& to_read)
{
        read_BitMatrix (file_name, *(to_read.public_key_K) );
        read_BitMatrix (file_name, *(to_read.key_M_inverse) );
        read_BitMatrix (file_name, *(to_read.key_T_with_noise) );
        read_BitMatrix (file_name, *(to_read.key_Jumble) );
}


// ENCRYPTs a text file

void Crypto::ENCRYPT() // was (&Crypto crypto_object)
{
        FILE* input;
        FILE* output;
        char input_file[128];
        char output_file[128];
        int l;

	l = unpadded_length % 8; // was C::
        if (l !=0)
        {
                printf("for the text encrypt demo,");
                printf(" the block length must be a multiple of 8 (bits)\n");
                return;
        }
	printf("Please enter the name of the file to be encrypted: ");
	scanf ("%s", input_file);
	if ( (input = fopen(input_file,"rb") ) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s\n", input_file);
		return;
	}
	printf("Please enter the name of the file to store the encrypted text: ");
	scanf ("%s", output_file);
	if ( (output = fopen(output_file,"wb") ) == NULL)
	{
		fprintf(stderr, "Couldn't open output file %s\n", output_file);
		fclose(input);
		return;
	}

	for (int bc = 0;; bc++)
	{

		if (read_block_of_plain_text(input)== EOF) break;
		encrypt();
		write_block_of_encrypted_text(output);
	}
	fclose(output);
	fclose(input);
}


void Crypto::DECRYPT() // was (&Crypto crypto_object)
{
	FILE* input;
	FILE* output;
	char input_file[128];
	char output_file[128];
	int l,m;
	l = unpadded_length % 8; // was C::
	if (l !=0)
	{
		printf("for the text encrypt demo,");
		printf(" the block length must be a multiple of 8 (bits)\n");
		return;
	}
	printf("Please enter the name of the file to be decrypted: ");
	scanf ("%s", input_file);
	if ( (input = fopen(input_file,"rb") ) == NULL)
	{
		fprintf(stderr, "Couldn't open input file %s\n", input_file);
		return;
	}
	printf("Please enter the name of the file to store the decrypted text: ");
	scanf ("%s", output_file);
	if ( (output = fopen(output_file,"wb") ) == NULL)
	{
		fprintf(stderr, "Couldn't open output file %s\n", output_file);
		fclose(input);
		return;
	}
	for (int bc =0;; bc++)
	{

		if (read_block_of_encrypted_text(input)== EOF) break;
		decrypt();
		write_block_of_decrypted_text(output);
	}
	fclose(output);
	fclose(input);
}

// ********************* random functions ******************

char working_key_phrase [255] = "This is a default key phrase";
int index_into_working_key_phrase = 0;
unsigned int hash_of_working_key_phrase;
Crypto* Warlock_random_number_private_key;
BitMatrix* Work_BitMatrix;
BitMatrix* Another_Work_BitMatrix;
void new_randomize ( char* key_phrase /* , Crypto& C */ )
{
	int n;
	hash_of_working_key_phrase = 0;
	index_into_working_key_phrase = 0;
	for (n = 0; n<255; ++n)
		working_key_phrase[n] = 0;

	for (int i = 0; key_phrase[i] != 0 && i < 254; ++i)
	{
		working_key_phrase[i] = key_phrase[i];
		hash_of_working_key_phrase += (  (i+1)*key_phrase[i]
						+ key_phrase[i] << 8  );
	}
	srand(hash_of_working_key_phrase);
/*
	Work_BitMatrix = (*Warlock_random_number_private_key).encrypted_text;
	set_BitMatrix (*Work_BitMatrix, &hash_of_working_key_phrase);
*/
}

int new_random ()
{
        unsigned int r,x,y,z;
	int s;
/*
        if ( Warlock_random_number_private_key == NULL )
        {
                return (rand());
        }
*/


/*
        (*Warlock_random_number_private_key).decrypt();
        inc_BitMatrix (*Work_BitMatrix);
        get_BitMatrix (*Work_BitMatrix, &hash_of_working_key_phrase);
*/
       if ( working_key_phrase[index_into_working_key_phrase] == 0)
       {
                cerr << "\nnew_random error";
                exit (-4);
       }
       if ( ( working_key_phrase[index_into_working_key_phrase+1] == 0 )
	  || ( working_key_phrase[index_into_working_key_phrase+2] == 0 )
	  || ( working_key_phrase[index_into_working_key_phrase+3] == 0 ) )
	     index_into_working_key_phrase = -2;
       index_into_working_key_phrase += 2;
/*
       (*Warlock_random_number_private_key).decrypt();
       Another_Work_BitMatrix = (*Warlock_random_number_private_key).decrypted_text;
       get_BitMatrix(*Another_Work_BitMatrix,
                     &hash_of_working_key_phrase);
*/
/*
(*Warlock_random_number_private_key).print_decrypted_text();
*/
       r = ((x=rand()) /*hash_of_working_key_phrase*/
               + (y=(unsigned int) (working_key_phrase[index_into_working_key_phrase] << 8))
               + (z=(unsigned int) (working_key_phrase[index_into_working_key_phrase+1]) ));
/*
cout << "\nr= " << r << " x= " << x << " y= " << y << " z= " << z
     << " working= " << index_into_working_key_phrase;
*/
/*
cout << "\nr = " << r << " hash =" << hash_of_working_key_phrase
     << " working[1] = " << (unsigned int) (working_key_phrase[index_into_working_key_phrase] << 8)
     << " working[2] = " << (unsigned int) (working_key_phrase[index_into_working_key_phrase+1])
     << " r should = " <<(hash_of_working_key_phrase
               + (working_key_phrase[index_into_working_key_phrase] << 8 )
               + working_key_phrase[index_into_working_key_phrase+1] );
*/
       s = (int) r;
       s = abs (r);
       for (;s<0; s = s + 5000);

       return (s);
}



// generate system keys, perform encryption and decryption, and display the
// results of the various stages of key generation as well as the various
// functional stages of encryption and decryption

void show(int BLOCK_LENGTH)
{
int x = BLOCK_LENGTH;
x = x+1;
int n;

	BitMatrix P(1,BLOCK_LENGTH),
		  E(1,BLOCK_LENGTH),
		  D(1,BLOCK_LENGTH);
	Crypto C(BLOCK_LENGTH);
	P.set_matrix_bit(0,1);  // test value
	randomize();
	int ii, jj;
	C.generate_keys (rand);
/*
cout << "\n starting encryption/decryption\n";

	C.print_public_key_K ();
	C.print_key_M ();

	C.print_key_M_inverse ();
	C.print_key_I ();
	C.print_replacements_rows ();
	C.print_key_T_with_noise  ();


	C.print_key_T_replaced ();
	C.print_key_T_to_be_jumbled ();
	C.print_key_Jumble ();
*/
	{
		P.reset();
		for ( jj = 0; jj < BLOCK_LENGTH; ++jj )
		{
			if ( rand() & 1 )
			{
				P.set_matrix_bit( 0, jj );
			}
		}
/*
		C.print_B();
		C.print_A_inverse();
		C.print_A_expanded();
		C.print_A();
		C.set_plain_text (P);
*/

cout << "\nstart";

for (n=0;n<10000;++n)

		C.encrypt();

/*
		C.print_unpadded_plain_text();

		C.print_plain_text();

		C.print_expanded_text();
		C.print_encrypted_text();
*/

for (n=0;n<10000;++n)		C.decrypt();		// look inside for another print ###

/*
		C.print_reverted_text();
		C.print_intermediate_text();
		C.print_resultant_text();
		C.print_decrypted_text();
*/
	}
//	cout << "\f";

}

void show_sig(int BLOCK_LENGTH)
{
int x = BLOCK_LENGTH;
x = x+1;

/*	BitMatrix P(1,BLOCK_LENGTH),
		  E(1,BLOCK_LENGTH),
		  D(1,BLOCK_LENGTH);
	Crypto C(BLOCK_LENGTH);
        Crypto S(BLOCK_LENGTH);
	randomize();
	int ii, jj;
	C.generate_keys (rand);
        S.generate_keys (rand);

cout << "\n starting encryption/decryption\n";
	C.print_public_key_K ();
	C.print_key_M ();
	C.print_key_M_inverse ();
	C.print_key_I ();
	C.print_replacements_rows ();
	C.print_key_T_with_noise  ();
	C.print_key_T_replaced ();
	C.print_key_T_to_be_jumbled ();
	C.print_key_Jumble ();
	S.print_public_key_K ();
	S.print_key_M ();
	S.print_key_M_inverse ();
	S.print_key_I ();
	S.print_replacements_rows ();
	S.print_key_T_with_noise  ();
	S.print_key_T_replaced ();
	S.print_key_T_to_be_jumbled ();
	S.print_key_Jumble ();

	{
		P.reset();
		for ( jj = 0; jj < BLOCK_LENGTH; ++jj )
		{
			if ( rand() & 1 )
			{
				P.set_matrix_bit( 0, jj );
			}
		}
		S.set_encrypted_text (P);
                S.print_encrypted_text();
		S.decrypt();
		S.print_resultant_text();
		S.print_decrypted_text();
                S.get_decrypted_text(E);
		C.set_plain_text (E);
		C.encrypt();
		C.print_plain_text();
		C.print_expanded_text();
		C.print_encrypted_text();
		C.decrypt();		// look inside for another print ###
		C.print_resultant_text();
		C.print_decrypted_text();
		C.get_decrypted_text(D);
                S.set_plain_text (D);
		S.encrypt();

                S.print_plain_text();
*/

/*
                S.print_expanded_text();  // was commented out before
*/

/*
                S.print_encrypted_text();

	}
	cout << "\f";
*/
}





// to generate keys for demo

void Crypto::GENERATE()
{
/*
        randomize();
*/
	generate_keys(/*rand*/ new_random );
/*
print_key_M();
print_key_M_inverse();
print_key_I();
cout << "\n";
*/

}




void SAVE(Crypto& to_write)
{
        FILE* input;
        FILE* output;
        char input_file[128];
        char output_file[128];
	printf("\nWhat is the file name of the public key? ");
	scanf("%s", output_file);
	if ( (output = fopen(output_file,"wb")) == NULL )
	{
		printf ("\nCouldn't open output file %s\n",output_file);
		return;
	}
        write_public_key(output, to_write);
	fclose(output);
}

void USE(Crypto& to_read)
{
	FILE* input;
        FILE* output;
        char input_file[128];
        char output_file[128];
	printf("\nWhat is the file name of the public key? ");
	scanf("%s", input_file);
	if ( (input = fopen(input_file,"rb")) == NULL )
	{
		printf ("\nCouldn't open input file %s\n",input_file);
		return;
	}
        read_public_key(input, to_read);
	fclose(input);
}



void HIDE(Crypto& to_write)
{
        FILE* input;
	FILE* output;
        char input_file[128];
        char output_file[128];
	printf("\nWhat is the file name of the private key? ");
	scanf ("%s", output_file);
	if ( (output=fopen(output_file,"wb")) == NULL)
	{
		printf("\nCouldn't open output file %s\n", output_file);
		return;
	}
        write_private_and_public_keys (output, to_write);
	fclose(output);
}


void RETRIEVE(Crypto& to_read)
{
        FILE* input;
	FILE* output;
	char input_file[128];
	char output_file[128];
	printf("\nWhat is the file name of the private key? ");
	scanf ("%s", input_file);
	if ( (input=fopen(input_file,"rb")) == NULL)
	{
		printf("\nCouldn't open input file %s\n", input_file);
		return;
	}
        read_private_and_public_keys (input, to_read);
	fclose(input);
}


void Crypto::show_stuff()
{

print_key_M();
print_public_key_K();
print_key_M_inverse();
print_key_T_with_noise();
print_key_T_to_be_jumbled();
print_plain_text();
print_expanded_text();
print_encrypted_text();
print_reverted_text();
// print_intermediate_text();
print_resultant_text();
print_decrypted_text();

}

// /*
void test_rw(int BLOCK_LENGTH)
{
	BitMatrix P(1,BLOCK_LENGTH),
		  E(1,BLOCK_LENGTH),
		  D(1,BLOCK_LENGTH);
	Crypto C(BLOCK_LENGTH);
	Crypto S(BLOCK_LENGTH);
	randomize();
	int ii, jj;
	C.generate_keys (rand);
        S.generate_keys (rand);
	C.print_public_key_K ();
	C.print_key_M ();
	C.print_key_M_inverse ();
	C.print_key_I ();
	C.print_replacements_rows ();
	C.print_key_T_with_noise  ();
	C.print_key_T_replaced ();
	C.print_key_T_to_be_jumbled ();
	C.print_key_Jumble ();
	S.print_public_key_K ();
	S.print_key_M ();
	S.print_key_M_inverse ();
	S.print_key_I ();
	S.print_replacements_rows ();
	S.print_key_T_with_noise  ();
	S.print_key_T_replaced ();
	S.print_key_T_to_be_jumbled ();
	S.print_key_Jumble ();
        HIDE(C);
        RETRIEVE(S);
	C.print_public_key_K ();
	C.print_key_M ();
	C.print_key_M_inverse ();
	C.print_key_I ();
	C.print_replacements_rows ();
	C.print_key_T_with_noise  ();
	C.print_key_T_replaced ();
	C.print_key_T_to_be_jumbled ();
	C.print_key_Jumble ();
	S.print_public_key_K ();
	S.print_key_M ();
	S.print_key_M_inverse ();
	S.print_key_I ();
	S.print_replacements_rows ();
	S.print_key_T_with_noise  ();
	S.print_key_T_replaced ();
	S.print_key_T_to_be_jumbled ();
	S.print_key_Jumble ();

   exit(0);
}
// */

// the following program segment is the Main Driver which causes results to be
// automatically displayed for system block lengths of 4, 6, 8, 16, 32,
// 64, and 76 bits.

int old_main();



main()

{
cout << "\n\n" <<

"Welcome to the wonderful world of WARLOCK!\n\n"
<<
"***********************************************************************\n\n"
<<
"WARNING:   The WARLOCK cryptosystem provided herein is a copy-\n"
<<
"righted system protected by patents (awarded and pending) and is\n"
<<
"provided solely for private personal use and evaluation only.\n"
<<
"Modifications to (or copies of) WARLOCK source or object programs\n"
<<
"must retain the warning and proprietary legend displayed on the\n"
<<
"first user screen.\n\n"
<<
"The use of WARLOCK cryptosystems for private-sector commercial or\n"
<<
"public-sector governmental purposes is strictly prohibited with-\n"
<<
"out proper licensing arrangements. Licensing information can be\n"
<<
"obtained by writing DataSec Systems, PO Box 4152, Huntsville, AL 35815-4152,\n"
<<
"or by sending a request by E-MAIL to WARLOCK@ACM.org,\n"
<<
"or by phoning (205) 881-8002.\n\n"
<<
"***********************************************************************"
<< "\n\n";


for (/*srand(1234)*/; old_main(); /*srand(1234)*/ );

//	show(4);
//	show(96);

cout << "\ndone";

/*
//	show(8);

//	show(48);

	show(32);
	show(64);
	show(76);

	show_sig(4);
	show_sig(6);

	show_sig(8);

	show_sig(16);
	show_sig(32);


	show_sig(64);
	show_sig(76);


test_rw(6);
*/

}





int old_main()
{
/*
        Crypto R(16);
	R.generate_keys(rand);
*/


	char answer[256];
	char ch;
	int specified_block_length;
	cout << "\nHow many 24 bit (3 byte) groups do you wish to"
	     << " encrypt at a time \n(enter 1,2,3,4, or 5)? ";
	for (answer[0] = '\n';
	     gets ( answer), answer[0] == '\n' || answer[0] == 0; );


	ch = answer[0];
	specified_block_length = (ch -'0');
	if ( ( ch = answer[1] ) != 0 )
	{
		specified_block_length =
		 ( ( (specified_block_length) * 10 + ch - '0') );
	}
	specified_block_length *= (8*3);

	Crypto C(specified_block_length);

	cout << "\nPlease enter your key (seed) phrase (in one line)\n";
	gets (answer);
	new_randomize( answer /* , R */ );
	C.GENERATE();
	for (; ch!= 'X' && ch != 'x';)
	{
		printf ("\nDo you wish to: ");
		printf ("S)ave ");
		printf ("U)se ");
		printf ("E)ncrypt ");
		printf ("D)ecrypt ");
		printf ("L)ook ");
		printf ("R)estart ");
		printf ("eX)it\n");

		printf ("Press S, U, E, D, L, R, or X: ");
		for (ch = '\n'; ch == '\n';)
                {
		        ch = getc (stdin);
                }
		switch (ch)
		{


			case 'S': case 's': SAVE(C); break;
			case 'U': case 'u': USE(C); break;
			case 'E': case 'e': C.ENCRYPT(); break;
			case 'D': case 'd': C.DECRYPT(); break;
                        case 'L': case 'l': C.show_stuff(); break;
                        case 'R': case 'r': return(1);
			case 'X': case 'x': return(0);

			default: break;
		}
	}
}
