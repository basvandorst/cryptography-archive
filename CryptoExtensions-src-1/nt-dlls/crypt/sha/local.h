/*
 * Copyright (C) 1996 Systemics Ltd (http://www.systemics.com/)
 * All rights reserved.
 *
 * This file allows watcom to compile java nt dlls.
 * This file is not required if you are compiling with Microsoft VC++
 */
 
#ifndef _I_can_do_watcom_to_java_
#define _I_can_do_watcom_to_java_

/* This pragma sets the export name to be the name with NO prefixed underscore as required but java. */
#pragma aux default "*"

/*
 * This macro cannot be redefined so that Watcom C understands it,
 * therefore a perl script is run on one of the java generated files,
 * and other uses of this macro are not required.
 */
#define __declspec(dllexport)

/* i386 etc are little endian, sparcs are big endian */
#define LITTLE_ENDIAN

#endif
