/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/
/*
 * long_seln.c shows how to get an arbitrarily large selection by
 * providing a reading procedure to selection_query().  The panel
 * items allow the user to choose between 3 selection ranks.
 */
#include <xview/xview.h>
#include <xview/textsw.h>
#include <xview/panel.h>
#include <xview/seln.h>


Seln_rank seln_type = SELN_PRIMARY;

#define FIRST_BUFFER            0
#define NOT_FIRST_BUFFER        !FIRST_BUFFER

char *seln_bufs[3];     /* contents of each of the three selections */


Xv_Server       server;

extern char *malloc();
Seln_result read_proc(); /* supplied to selection_query() as reader */
char *get_selection();
void print_seln();



main(argc, argv)
char *argv[];
{
    	server = xv_init(XV_INIT_ARGC_PTR_ARGV, &argc, argv, 0);
	print_seln();
}

void
print_seln()
{
    char *text = get_selection();

    if (text)
        printf("%s", text);
}

/*
 * return the text selected in the current selection rank.  Use
 * selection_query() to guarantee that the entire selection is
 * retrieved.  selection_query() calls our installed routine,
 * read_proc() (see below).
 */
char *
get_selection()
{
    Seln_holder   holder;
    Seln_result   result;
    Seln_request  *response;
    char          context = FIRST_BUFFER;

    holder = selection_inquire(server, seln_type);

    /* result is based on the return value of read_proc() */
    result = selection_query(server, &holder, read_proc, &context,
        SELN_REQ_BYTESIZE,              NULL,
        SELN_REQ_CONTENTS_ASCII,        NULL,
        NULL);
    if (result == SELN_FAILED) {
        return NULL;
    }

    return seln_bufs[seln_type];
}

/*
 * Called by selection_query for every buffer of information received.
 * Short messages (under about 2000 bytes) will fit into one buffer.
 * For larger messages, read_proc is called for each buffer in the
 * selection.  The context pointer passed to selection_query is
 * modified by read_proc so that we know if this is the first buffer
 * or not.
 */
Seln_result
read_proc(response)
Seln_request *response;
{
    char *reply;  /* pointer to the data in the response received */
    long seln_len; /* total number of bytes in the selection */
    static long seln_have_bytes;
        /* number of bytes of the selection
         * which have been read; cumulative over all calls for
         * the same selection (it is reset when the first
         * response of a selection is read)
         */

    if (*response->requester.context == FIRST_BUFFER) {
        reply = response->data;

        /* read in the length of the selection -- first attribute.
         * advance "reply" passed attribute to point to actual data.
         */
        reply += sizeof(SELN_REQ_BYTESIZE);
        /* set seln_len to actual data now. (bytes selected) */
        seln_len = *(int *)reply;
        /* advance "reply" to next attribute in list */
        reply += sizeof(long);

        /* create a buffer large enough to store entire selection */
        if (seln_bufs[seln_type] != NULL)
            free(seln_bufs[seln_type]);
        if (!(seln_bufs[seln_type] = malloc(seln_len + 1))) {
            return(SELN_FAILED);
        }
        seln_have_bytes = 0;

        /* move "reply" passed attribute so it points to contents */
        reply += sizeof(SELN_REQ_CONTENTS_ASCII);
        *response->requester.context = NOT_FIRST_BUFFER;
    } else {
        /* this is not the first buffer, so the contents of the
	 * response is just more of the selection
         */
        reply = response->data;
    }

    /* copy data from received to the seln buffer allocated above */
    (void) strcpy(&seln_bufs[seln_type][seln_have_bytes], reply);
    seln_have_bytes += strlen(reply);

    return SELN_SUCCESS;
}
