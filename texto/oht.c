#include "oht.h"

struct OHT *OHT_Create( int elementsize, int size, 
                        int (*eq_func)(void *, void *), 
                        int (*get_ival)(void *),
                        void (*print_element)(void*) )
{
   struct OHT *table;
   int i;
   int isprime = 0;
	int max;
   
   if ( elementsize == (int) NULL    || size        == (int) NULL  || 
        eq_func     == NULL          || get_ival    == NULL           ){
      fprintf(stderr, "OHT_Create: Error, received null parameter.\n");
      return( (struct OHT *) OHT_ERROR );
      }

   table = (struct OHT *) malloc( sizeof(struct OHT) );

	/* Make the size of the table 12% bigger than the expected number of */
	/* elements, so it will be 85-90% full, which is near optimum.       */
	/* Round to nearest multiple of 20, then add 7, so we know it's odd  */
	/* and somewhat likely to be prime.                                  */
   table->tablesize    = ((int)(size + .12*size)/20)*20 + 7;
   table->num_items    = 0;
   table->are_eq       = eq_func;
   table->get_i        = get_ival;
   table->print_it     = print_element;
   table->s_search_len = 0; 
   table->s_num_scans  = 0;
   table->u_search_len = 0; 
   table->u_num_scans  = 0;
   table->elsize       = elementsize;


	/* We want the size of the table to be a prime number so that the */
	/* second hash function will produce a "step" size which will     */
	/* eventually search the entire table.                            */

	while ( isprime == 0 ) {
		/* We already know tablesize is an odd number, see assignment above */
		max = (int) table->tablesize / 2;
		for ( i=3; i<max; i+=2 ) {
			if ( table->tablesize % i == 0 ) { /* not prime */
				table->tablesize += 2;
				break;
				}
			}
		if ( i >= (max-2) ) {
			isprime = 1;
			}
		}

   table->ivals        = (int *)  calloc( sizeof(size), table->tablesize );   
   table->e            = (void *) calloc( elementsize, table->tablesize );

   return(table);

}


int OHT_Destroy( struct OHT *table )
{

   if (table == NULL) return(OHT_ERROR);

   free(table->ivals);
   free(table->e);
   free(table);

   return(OHT_SUCCESS);

}


void *OHT_Access( void *element, struct OHT *table )
{
   
   int index;
   int step_size;
   int this_ival;
   int search_length;

   if ( table == NULL ) {
      fprintf(stderr, "OHT_Access: cannot look anything up on null table.\n");
      return( (void *) OHT_ERROR);
      }

   if ( element == NULL ) {
      fprintf(stderr, "OHT_Access: cannot look up null element.\n");
      return( (void *) OHT_ERROR);
      }

   search_length = 1;

   this_ival = (*table->get_i)(element);
   index     = OHT_Hash1(this_ival, table->tablesize);
   step_size = OHT_Hash2(this_ival, table->tablesize);

   while ( search_length < table->tablesize ) {
      
      if ( *( (char *)table->e + (index*table->elsize) ) == 0 ) {
         /* Found an empty slot, so it's not here */
         table->u_search_len += search_length;
         table->u_num_scans++;
         return( (void *) OHT_FAILURE);
         }
      else if ( table->ivals[index] < this_ival ) {
         /* Found a higher ival, so it's not here */
         table->u_search_len += search_length;
         table->u_num_scans++;
         return( (void *) OHT_FAILURE);
         }
      else if ( 
         (*(table->are_eq))( element, (char*)table->e + index*table->elsize)) {
         /* Found it */
         table->s_search_len += search_length;
         table->s_num_scans++;
         return( (char *)table->e + index*table->elsize );
         }
      else {
         /* Don't know yet, look in next slot in sequence */
         index = (index + step_size) % table->tablesize;
         search_length++;
         }

      } /* end while haven't searched whole table */

   table->u_search_len += search_length;
   table->u_num_scans++;

   return( (void *) OHT_FAILURE );   /* It's not there */

}   /* end of OHT_Access */


int OHT_Insert( void *element, struct OHT *table )
{

   int index;
   int step_size;
   int this_ival;
   char *temp_ptr;
   int search_length;
   int i;

   if ( table == NULL ) {
      fprintf(stderr, "OHT_Insert: cannot look anything up on null table.\n");
      return(OHT_ERROR);
      }

   if ( element == NULL ) {
      fprintf(stderr, "OHT_Insert: cannot insert null element.\n");
      return(OHT_ERROR);
      }

   search_length = 1;

   if (table->tablesize == table->num_items) {
      fprintf(stderr, "OHT_Insert: cannot insert into full table.\n");
      return(OHT_FAILURE);
      }
   if ( OHT_Access( element, table ) != (void*) OHT_FAILURE ) {
      fprintf(stderr, "OHT_Insert: cannot insert duplicate element.\n");
      return(OHT_FAILURE);
      }

   this_ival = (*table->get_i)(element);
   index     = OHT_Hash1(this_ival, table->tablesize);
   step_size = OHT_Hash2(this_ival, table->tablesize);

   while ( search_length < table->tablesize ) {

      /* If empty slot, fill it */
      if ( *( (char *)table->e + (index*table->elsize) ) == 0 ) {

         /* This ugliness is required to do byte-by-byte assignment for */
         /* structures of unknown size */
         for ( i=0; i<table->elsize; i++ ) {
            *( (char *)table->e + (index * table->elsize) + i ) = 
               *( (char*)element + i);
            }
            
         table->ivals[index] = this_ival;
         table->num_items++;
         fflush(stdout);
         return(OHT_SUCCESS);
         }
      else if ( table->ivals[index] < this_ival ) {  /* Boot smaller guy */

         temp_ptr = (char *) malloc( table->elsize );

         /* make a copy of bootee */
         for ( i=0; i<table->elsize; i++ ) {
            *(temp_ptr + i) = *((char*)table->e + (index * table->elsize) + i);
            }
            
         /* insert booter */
         for ( i=0; i<table->elsize; i++ ) {
            *( (char *)table->e + (index * table->elsize) + i ) = 
               *( (char*)element + i);
            }
            
         table->ivals[index] = this_ival;
         OHT_Insert( temp_ptr, table );
         free(temp_ptr);
         return(OHT_SUCCESS);
         }
      else {
         index = (index + step_size) % table->tablesize;
         search_length++;
         }

      }   /* end while haven't searched the whole table */

   printf("Cannot insert %d in any slot, and table is not full.\n", this_ival);
	printf("This is a problem.\n");

   return(OHT_FAILURE);

}    /* end of OHT_Insert */


void *OHT_Remove( void *element, struct OHT *table )
{
   int index;
   int step_size;
   int this_ival;
   void *temp_ptr;
   int search_length;

   if ( table == NULL ) {
      fprintf(stderr, "OHT_Remove: cannot look anything up on null table.\n");
      return( (void *) OHT_ERROR);
      }

   if ( element == NULL ) {
      fprintf(stderr, "OHT_Remove: cannot remove null element.\n");
      return( (void *) OHT_ERROR);
      }

   search_length = 1;

   this_ival = (*table->get_i)(element);
   index     = OHT_Hash1(this_ival, table->tablesize);
   step_size = OHT_Hash2(this_ival, table->tablesize);

   while ( search_length < table->tablesize ) {

      if ((*(table->are_eq))( element, (char*)table->e + index*table->elsize)){
         /* Found it, so yank it */
         temp_ptr = (char *)table->e + (index * table->elsize);
         *( (char *)table->e + (index * table->elsize)) = (int) NULL;
         table->num_items--;
         return( temp_ptr );
         }
      else if ( table->ivals[index] < this_ival ) {
         /* It's not in the table */
         return( (void *) OHT_FAILURE );
         }
      else {
         /* move along */
         index = (index + step_size) % table->tablesize;
         search_length++;
         }
      }

	return( (void *) OHT_FAILURE );

}


int OHT_Hash1( int i, int size ) 
{
	/*
	** i should be equally likely to hash to any given index.
	*/
   return( i % size );
}


int OHT_Hash2( int i, int size )
{
	/*
	** if size is a prime, this will give a step size < size which will
	** eventually scan the whole table.
	*/
   return( ( i % (size - 1)) + 1);
}


void OHT_Stats( struct OHT *table )
{
   printf("\nTable size: %d\n", table->tablesize);
   printf("Slots filled: %d\n", table->num_items);
   printf("Function pointers: %x %x\n", table->are_eq, table->get_i);
   printf("Total successful scan length: %f  over %f scans\n", 
           table->s_search_len, table->s_num_scans);
   if(table->s_num_scans != 0) {
      printf("Average successful search length: %f\n", 
              table->s_search_len/table->s_num_scans);
      }
   printf("Total unsuccessful scan length: %f  over %f scans\n", 
           table->u_search_len, table->u_num_scans);
   if(table->u_num_scans != 0) {
      printf("Average unsuccessful search length: %f\n", 
              table->u_search_len/table->u_num_scans);
      }
   fflush(stdout);
}


void OHT_Dump( struct OHT *table )
{
   int i;

   OHT_Stats(table);
   printf("Indexes\n-------\n");
   for ( i=0; i<table->tablesize; i++) {
      printf("Slot %3d:   ", i);
      if ( *( (char *)table->e + (i * table->elsize)) == (int) NULL ) {
         printf("--- empty ---\n");
         }
      else {
         printf("index = %d\n", table->ivals[i]);
         if (table->print_it != NULL) {
            (*(table->print_it))( (char *)table->e + (i * table->elsize) );
            }
         }
      }
   printf("-------\n");
   fflush(stdout);

}

