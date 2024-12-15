/***    
****   oht.h                              Ordered Hash Table library
****  
****   This library provides very fast lookup for an arbitrarily large
****   dictionary.  Elements of the dictionary can be anything, usually
****   structures of some arbitrary structure, which oht.c only needs to 
****   know the size of.
****
****   In order to use an ordered hash table, a program must declare a 
****   pointer to a "struct OHT", and assign to it the result of a call to
****   OHT_Create.  
****
****   OHT_Create requires the following: 
****      1) The size in bytes of the elements that will be used. 
****      2) A good estimate of how many elements the table will be storing. 
****      3) A pointer to a function which tests two given elements for 
****         equality, returning 0 on failure, and 1 on success. 
****      4) a pointer to a function which computes an integer index based 
****         on the unique information in each element.  This integer index 
****         should ideally range from zero to 5-20 times the size of the table.
****      5) Optionally, a pointer to a function which prints an individual
****         element.
****
****   After this call is made, OHT_Access, OHT_Insert, and OHT_Remove can
****   be used to manipulate the data in the table, and OHT_Dump or OHT_Stats
****   can be called to obtain status information about the table.
****   When the table is no longer needed, OHT_Destroy can be called to free
****   the memory which was allocated by OHT_Create.
***/

#include <stdio.h>
#include <fcntl.h>

#define OHT_ERROR   -1
#define OHT_FAILURE 0
#define OHT_SUCCESS 1

struct OHT {
   long   tablesize;                /* size of table */
   long   num_items;                /* how many slots are full */
   int  (*are_eq)(void *, void *);  /* pointer to equality function */
   int  (*get_i)(void *);           /* pointer to function to get int index */
   void (*print_it)(void *);        /* pointer to function to print element */
   void  *e;                        /* pointer to array of elements */
   int   *ivals;                    /* pointer to array of index values */
   float  s_search_len;             /* total length of successful scans */
   float  s_num_scans;              /* needed for average search length */
   float  u_search_len;             /* total length of unsuccessful scans */
   float  u_num_scans;              /* needed for average search length */
   int    elsize;                   /* size of elements & slots for them */
   } ;


/* 
** Creates and returns a pointer to a new table structure.
*/
struct OHT *
OHT_Create( int elementsize,                /* Size of elements so we know */
                                            /* how big to make the slots   */

            int size,                       /* How many slots              */

            int (*eq_func)(void *, void *), /* Pointer to function that    */
                                            /* tests equality of two       */
                                            /* elements                    */

            int (*ival_func)(void*),        /* Pointer to function that    */
                                            /* returns an integer index    */
                                            /* for any given element       */
            
            void (*print_it)(void *) );     /* Optional pointer to function  */
                                            /* which prints a single element */

/* 
** Destroys table and frees memory.
*/
int OHT_Destroy( struct OHT *table );

/* 
** Access searches table for a certain element and returns a pointer to the 
** element or returns OHT_FAILURE if not found.
*/
void *OHT_Access( void *element, struct OHT *table );      

/* 
** Insert puts element in table, returns OHT_FAILURE if full or duplicate, else
** returns OHT_SUCCESS.
*/
int OHT_Insert( void *element, struct OHT *table );

/* 
** Remove replaces element with NULL in the table, returns OHT_FAILURE if 
** not found, otherwise returns the removed element.
*/
void *OHT_Remove( void *element, struct OHT *table );

/* 
** Prints contents of table plus some stats.
*/
void OHT_Dump( struct OHT *table );

/* 
** Prints just the stats.
*/
void OHT_Stats( struct OHT *table );

/*
** Hash functions.
*/
int OHT_Hash1( int i, int size );
int OHT_Hash2( int i, int size );


