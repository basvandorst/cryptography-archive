/* $Id: $
 *
 * $Log: $
 */

/* returns the number of the current selection from the menu */
#define MenuCurrent(X) X.current

/* this will make MenuLoop() update the screen on the next call */
#define MenuUpdate(X) (X.update = 1)

struct menu_object {
  char **data;
  char *title;
  char *prompt;
  char *help;
  int prompt_length;
  int len;
  int max;
  int current;
  int pagetop; /* the index of the first entry on this page */
  unsigned int update : 1;
};
typedef struct menu_object menu_t;
