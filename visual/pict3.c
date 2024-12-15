/****************************************************************
 * (C) Copyright 1995 Jouko Holopainen     All rights reserved.
 *
 * Jouko Holopainen
 *
 * $Id: pict3.c,v 1.1 1996/05/15 03:58:32 jhol Exp $
 *
 * Visual cryptography
 *
 * $Log: pict3.c,v $
 * Revision 1.1  1996/05/15  03:58:32  jhol
 * Initial revision
 *
 ****************************************************************/

#ifndef NDEBUG
static const char rcsid[] =
  "$Id: pict3.c,v 1.1 1996/05/15 03:58:32 jhol Exp $";
#endif

/****************************************************************
 *
 * Imported modules
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/****************************************************************
 *
 * Public variables
 *
 ****************************************************************/

/****************************************************************
 *
 * Local constants and defines
 *
 ****************************************************************/

#define MAX_BLOCKS    4

#define NO_OF_WHITES  6 /* six different white blocks */
#define NO_OF_BLACKS  4
#define NO_OF_BLOCKS  (NO_OF_BLACKS + NO_OF_WHITES)

#define FIRST_BLOCK   'A'

#define BUFSIZE       4096

/****************************************************************
 *
 * Local types
 *
 ****************************************************************/

/* enumerate used colours */
typedef enum {
  WHITE,
  BLACK,
  NO_OF_COLOURS
}t_colour;

typedef struct{
  int nr_possible; /* Number of (valid) entries in a_index */
  int a_index[MAX_BLOCKS]; /* Blocks satisfying the colour criteria */
}t_map;

typedef t_map t_block[NO_OF_COLOURS][NO_OF_COLOURS];

typedef struct{
  FILE *pFILE_in;
  int curr_x, curr_y;
  int max_x, max_y;
  int colours;
  long start_pos;
  unsigned char ac_line[BUFSIZE];
}t_file_in;

/****************************************************************
 *
 * Local variables
 *
 ****************************************************************/

const t_block as_blocks[NO_OF_BLOCKS] = {
  {{{4, {2,3,4,5}},     /* W->W (A) white hor 1 */
      {1, {1}}},        /* W->B  -- */
     {{2, {6,7}},       /* B->W  XX */
        {2, {8,9}}}},   /* B->B */
  {{{4, {2,3,4,5}},     /* W->W (B) white hor 2 */
      {1, {0}}},        /* W->B  XX */
     {{2, {8,9}},       /* B->W  -- */
        {2, {6,7}}}},   /* B->B */
  {{{4, {0,1,4,5}},     /* W->W (C) white ver 1 */
      {1, {3}}},        /* W->B  X- */
     {{2, {6,8}},       /* B->W  X- */
        {2, {7,9}}}},   /* B->B */
  {{{4, {0,1,4,5}},     /* W->W (D) white ver 2 */
      {1, {2}}},        /* W->B  -X */
     {{2, {7,9}},       /* B->W  -X */
        {2, {6,8}}}},   /* B->B */
  {{{4, {0,1,2,3}},     /* W->W (E) white diag 1 */
      {1, {5}}},        /* W->B  -X */
     {{2, {6,9}},       /* B->W  X- */
        {2, {7,8}}}},   /* B->B */
  {{{4, {0,1,2,3}},     /* W->W (F) white diag 2 */
      {1, {4}}},        /* W->B  X- */
     {{2, {7,8}},       /* B->W  -X */
        {2, {6,9}}}},   /* B->B */
  {{{3, {0,2,4}},       /* W->W (G) black lu */
      {3, {1,3,5}}},    /* W->B  -X */
     {{1, {6}},         /* B->W  XX */
        {3, {7,8,9}}}}, /* B->B */
  {{{3, {0,3,5}},       /* W->W (H) black ru */
      {3, {1,2,4}}},    /* W->B  X- */
     {{1, {7}},         /* B->W  XX */
        {3, {6,8,9}}}}, /* B->B */
  {{{3, {1,2,5}},       /* W->W (I) black ll */
      {3, {0,3,4}}},    /* W->B  XX */
     {{1, {8}},         /* B->W  -X */
        {3, {6,7,9}}}}, /* B->B */
  {{{3, {1,3,4}},       /* W->W (J) black lr */
      {3, {0,2,5}}},    /* W->B  XX */
     {{1, {9}},         /* B->W  X- */
        {3, {6,7,8}}}}  /* B->B */
};

static t_file_in as_picts[3];

/****************************************************************
 *
 * Local function prototypes
 *
 ****************************************************************/

static void print_header(FILE *pFILE_out);
static void get_pixel(int x, int y, t_colour *pe_sheet1_colour, 
                      t_colour *pe_sheet2_colour, 
                      t_colour *pe_composite_colour);
static void print_pixel(FILE *pFILE_out, int sheet_nro,
                        t_colour e_sheet1_colour,
                        t_colour e_sheet2_colour,
                        t_colour e_composite_colour);
static int open_file(int indx, char *pc_fname);

/****************************************************************
 *
 * Local functions
 *
 ****************************************************************/

/****************************************************************
 * FUNCTION:    open_file
 * DESCRIPTION: Opens a picture file
 * INPUT:
 *  PARAMETERS: indx - index to as_picts,
 *              pc_fname - name of the file
 *  GLOBALS:    none
 * OUTPUT:
 *  RESULT:     0 = fail, 1 = OK
 *  GLOBALS:    none
 * CALLS:       none
 * COMMENTS:    none
 ****************************************************************/

static int open_file(int indx, char *pc_fname)
{
  FILE *pFILE_in;
  char ac_line[BUFSIZE];

  /* Open input file */
  if ((pFILE_in = fopen(pc_fname, "r")) == NULL){
    fprintf(stderr, "Can not open\n");
    return 0;
  }
  
  assert(indx <= (sizeof(as_picts) / sizeof(t_file_in)));

  as_picts[indx].pFILE_in = pFILE_in;

  /* Check that the file is PGM */
  fgets(ac_line, BUFSIZE - 1, pFILE_in);

  /* Magic number must be "P5" */
  if (strncmp(ac_line, "P5", 2) != 0){
    fprintf(stderr, "Magic number (P5) does not match\n");
    fclose(pFILE_in);
    return 0;
  }

  /* Remove possible comment */
  do{
    fgets(ac_line, BUFSIZE - 1, pFILE_in);
  }while(ac_line[0] == '#');

  /* Read X and Y size */
  sscanf(ac_line, "%d %d", &as_picts[indx].max_x, &as_picts[indx].max_y);

  /* Get number of gray scales */
  fgets(ac_line, BUFSIZE - 1, pFILE_in);
  sscanf(ac_line, "%d", &as_picts[indx].colours);

  as_picts[indx].curr_x = 0;
  as_picts[indx].curr_y = 0;
  as_picts[indx].start_pos = ftell(pFILE_in);
  return 1;
}

/****************************************************************
 * FUNCTION:    print_header
 * DESCRIPTION: Prints header for the ps file
 * INPUT:
 *  PARAMETERS: pFILE_out - output file
 *  GLOBALS:    none
 * OUTPUT:
 *  RESULT:     none
 *  GLOBALS:    none
 * CALLS:       none
 * COMMENTS:    none
 ****************************************************************/

static void print_header(FILE *pFILE_out)
{
  FILE *pFILE_in;
  char ac_buf[BUFSIZE];
  size_t got;

  /* open header file */
  if ((pFILE_in = fopen("ps_hdr.psi", "r")) == NULL){
    fprintf(stderr, "Cannot open 'ps_hdr.psi'\n");
    perror("fopen()");
    exit(1);
  }

  /* copy the input file to output file */
  while(!feof(pFILE_in)){
    got = fread(ac_buf, 1, BUFSIZE, pFILE_in);
    fwrite(ac_buf, 1, got, pFILE_out);
  }
}

/****************************************************************
 * FUNCTION:    get_pixel
 * DESCRIPTION: Gets the colour of the original pictures at this point
 * INPUT:
 *  PARAMETERS: pe_sheet1_colour - colour of first sheet
 *              pe_sheet1_colour - colour of second sheet
 *              pe_composite_colour - colour of combination
 *  GLOBALS:    none
 * OUTPUT:
 *  RESULT:     none
 *  GLOBALS:    none
 * CALLS:       none
 * COMMENTS:    assumes that y runs faster than x
 ****************************************************************/

static void get_pixel(int x, int y, t_colour *pe_sheet1_colour, 
                      t_colour *pe_sheet2_colour, 
                      t_colour *pe_composite_colour)
{
  int i;
  long pos;
  int c[3];

  *pe_sheet1_colour = WHITE;
  *pe_sheet2_colour = WHITE;
  *pe_composite_colour = WHITE;

#if 0
  /* Next line? */
  if (y == 0 && x != 0){
    /* All pictures */
    for (i = 0; i < 3; i++){
      /* Picture big enough? */
      if (as_picts[i].colours != 0){
        /* Read all bytes at the end of prev line */
        while(as_picts[i].curr_x != as_picts[i].max_x){
          fgetc(as_picts[i].pFILE_in);
          as_picts[i].curr_x++;
        }
      }
      as_picts[i].curr_x = 0;

      /* All of the picture gotten? */
      if (++as_picts[i].curr_y > as_picts[i].max_y){
        as_picts[i].colours = 0;
        fclose(as_picts[i].pFILE_in);
      }
    } /* endfor (i) */
  } /* endif (next line) */

 NULL

  for (i = 0; i < 3; i++){
    if (as_picts[i].colours){
      pos = as_picts[i].start_pos + x + as_picts[i].max_x * y;
      if (fseek(as_picts[i].pFILE_in, pos, SEEK_SET) < 0){
        c[i] = 255;
      }else{
        c[i] = fgetc(as_picts[i].pFILE_in);
      }
    }else{
      c[i] = 255;
    }
  }
#else
  for (i = 0; i < 3; i++){
    c[i] = as_picts[i].ac_line[y];
  }
#endif

  /* Sheet 1 */
  if (c[0] < as_picts[0].colours / 2)
    *pe_sheet1_colour = BLACK;

  /* Sheet 2 */
  if (c[1] < as_picts[1].colours / 2)
    *pe_sheet2_colour = BLACK;

  /* Composite sheet */
  if (c[2] < as_picts[2].colours / 2)
    *pe_composite_colour = BLACK;
}

/****************************************************************
 * FUNCTION:    print_pixel
 * DESCRIPTION: Prints a pixel
 * INPUT:
 *  PARAMETERS: pFILE_out - output file
 *              sheet_nro - sheet we are printing
 *              e_sheet1_colour - colour of first sheet
 *              e_sheet1_colour - colour of second sheet
 *              e_composite_colour - colour of combination
 *  GLOBALS:    none
 * OUTPUT:
 *  RESULT:     none
 *  GLOBALS:    none
 * CALLS:       none
 * COMMENTS:    none
 ****************************************************************/

static void print_pixel(FILE *pFILE_out, int sheet_nro,
                        t_colour e_sheet1_colour,
                        t_colour e_sheet2_colour,
                        t_colour e_composite_colour)
{
  int rnd1, rnd2, block;

  rnd1 = rand();
  rnd2 = rand();

  /* first sheet? */
  if (sheet_nro == 1){
    /* white? */
    if (e_sheet1_colour == WHITE){
      block = rnd1 % NO_OF_WHITES;
    }else{ /* black */
      block = rnd1 % NO_OF_BLACKS + NO_OF_WHITES;
    }
  }else{
    /* second sheet is much harder ... */
    if (e_sheet1_colour == WHITE){
      rnd1 = rnd1 % NO_OF_WHITES;
    }else{
      rnd1 = rnd1 % NO_OF_BLACKS + NO_OF_WHITES;
    }
    rnd2 %= as_blocks[rnd1][e_sheet2_colour][e_composite_colour].
      nr_possible;
    block = as_blocks[rnd1][e_sheet2_colour][e_composite_colour].
      a_index[rnd2];
  } /* endif (sheet_nro) */
  fprintf(pFILE_out, "%c ", FIRST_BLOCK + block);
}

/****************************************************************
 *
 * Public functions
 *
 ****************************************************************/

/****************************************************************
 * FUNCTION:    main
 * DESCRIPTION: main program
 * INPUT:
 *  PARAMETERS: argc, argv
 *  GLOBALS:    none
 * OUTPUT:
 *  RESULT:     none
 *  GLOBALS:    none
 * CALLS:       none
 * COMMENTS:    none
 ****************************************************************/

int main(int argc, char *argv[])
{
  FILE *pFILE_out;
  int sheet_nro;
  int i, x, y, max_x, max_y;
  int seed = time(NULL);
  t_colour e_sheet1_colour, e_sheet2_colour, e_composite_colour;

  if (argc != 5){
    fprintf(stderr, "Usage: %s in1 in2 inC out\n", argv[0]);
    exit(1);
  }

  /* Output file exists? */
  if ((pFILE_out = fopen(argv[4], "r")) != NULL){
    fprintf(stderr, "Will not overwrite '%s'\n", argv[4]);
    exit(1);
  }

  /* Open output file */
  if ((pFILE_out = fopen(argv[4], "w")) == NULL){
    fprintf(stderr, "Can not open '%s' for writing\n", argv[4]);
    exit(1);
  }

  max_x = 200;
  max_y = 200;

  /* Open input files */
  for (i = 0; i < 3; i++){
    if (open_file(i, argv[i + 1]) == 0){
      fprintf(stderr, "Failed opening '%s'\n", argv[i + 1]);
      exit(1);
    }
    if (as_picts[i].max_x < max_x)
      max_x = as_picts[i].max_x;
    if (as_picts[i].max_y < max_y)
      max_y = as_picts[i].max_y;
  }

  printf("Size: %d %d\n", max_x, max_y);

  print_header(pFILE_out);

  for (sheet_nro = 1; sheet_nro <= 2; sheet_nro++){
    fprintf(pFILE_out, "\n%%%%Page: %d %d\n", sheet_nro, sheet_nro);
    fprintf(pFILE_out, "\ndrawcrosses\n");
    srand(seed);

    /* Rewind ouput files */
    for (i = 0; i < 3; i++){
      fseek(as_picts[i].pFILE_in, as_picts[i].start_pos, SEEK_SET);
    }

    for (x = 0; x < max_x; x++){
      /* Read a line for each picture */
      for (i = 0; i < 3; i++){
        fread(as_picts[i].ac_line, 1, as_picts[i].max_x, as_picts[i].pFILE_in);
      }

      for (y = 0; y < max_y; y++){
        get_pixel(x, y, &e_sheet1_colour, &e_sheet2_colour, 
                  &e_composite_colour);
        print_pixel(pFILE_out, sheet_nro, e_sheet1_colour, e_sheet2_colour,
                    e_composite_colour);
        if ((y & 15) == 15)
          fprintf(pFILE_out, "\n");
      }
      fprintf(pFILE_out, "nl\n");
    } /* endfor (x) */
    fprintf(pFILE_out, "\nnextpage\n");
  } /* endfor (sheet) */
  /* trailer */
  fprintf(pFILE_out, "\n%%%%Trailer\n");
  fprintf(pFILE_out, "%%%%EOF\n");
}
