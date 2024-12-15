/* ======================================================================

   Functions to manage MIME type data structures for use
                 with Eudora translation API on the Mac.
                 
   Last Edited: July 18, 1996

   Laurence Lundblade <lgl@qualcomm.com>
   Myra Callen <t_callen@qualcomm.com>

   Copyright 1995, 1996 QUALCOMM Inc.
   
   Some of this code is from the c-client and is 
      Copyright University of Washington 
*/

void          init_mime_type_maker(Handle ((*)(size_t)), void (*)(Handle));
emsMIMEtype **make_mime_type(ConstStringPtr, ConstStringPtr, ConstStringPtr);
void 	      free_mime_type(emsMIMEtype **);
Handle  	  string_mime_type(emsMIMEtype **);
void          add_mime_parameter(emsMIMEtype **,
					ConstStringPtr name, ConstStringPtr value);
Handle        get_mime_parameter(emsMIMEtype **, ConstStringPtr);
short 		  transfer_encodings(char *, long);
short         match_mime_type(const emsMIMEtype **,
				ConstStringPtr, ConstStringPtr);
void          parse_mime_ct( Handle headherH, emsMIMEtype ***);		

#define kContentTypeHdrPStr		"\pContent-Type:"
#define kContentTypeHdrCStr		"Content-Type:"
#define kContentTypeHdrLen		13
