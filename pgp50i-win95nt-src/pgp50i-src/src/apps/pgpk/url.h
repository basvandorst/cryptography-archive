/*
 * url.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: url.h,v 1.1.2.2 1997/06/07 09:49:16 mhw Exp $
 */

int parse_url(char *protocol,
	      char *server,
	      char *file,
	      int *port,
	      const char *url);
