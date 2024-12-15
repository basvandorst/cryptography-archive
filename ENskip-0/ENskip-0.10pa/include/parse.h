/*
 * Copyright (C) 1995 M. Hauber, Ch. Schneider
 * See COPYING for more details
 */
extern struct skipcache *parse_parse(char *path);
extern int parse_save(struct skipcache *cnf, char *path);
extern int parse_free(struct skipcache *cnf);
