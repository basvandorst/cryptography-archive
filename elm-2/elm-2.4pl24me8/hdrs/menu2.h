struct menu_item {
  char *option;
  char key;
  char offset;
  char type;
  union ptrs { 
    char *c;
    int *i; /* used by BOL and INT */
  } d;
};
