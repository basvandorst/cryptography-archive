
#ifndef	_module_If_defined_
#define	_module_If_defined_

#if defined (ICR1)
#include "ul/psap.h"
#include "asn1/UNIV-types.h"
#else
#include "psap.h"
#include "UNIV-types.h"
#endif


#ifndef SET_OF
#if defined(MAC) || defined(__STDC__)
#define SET_OF(t) struct set_of_##t {               \
        t *element;                                 \
        struct set_of_##t *next;                    \
}
#else
#define SET_OF(t) struct set_of_/**/t {             \
        t *element;                                 \
        struct set_of_/**/t *next;                  \
}
#endif /* MAC */
#endif


#define	type_SEC_IF_AttributeType	OIDentifier
#define	type_SEC_IF_AttributeValue	PElement

typedef struct type_SEC_IF_Name                        DName;
typedef struct type_SEC_IF_RelativeDistinguishedName   RDName;

typedef struct type_SEC_IF_Attribute                Attr;
typedef struct type_SEC_IF_AttributeType    	    AttrType;
typedef struct type_SEC_IF_AttributeValues  	    AttrValues;
typedef struct type_SEC_IF_AttributeValueAssertion  AttrValueAssertion;

typedef SET_OF(AttrType)                	       SET_OF_AttrType;
typedef SET_OF(Attr)                	               SET_OF_Attr;


struct type_SEC_IF_AttributeValueAssertion {
    struct type_SEC_IF_AttributeType *element_IF_0;

    struct type_SEC_IF_AttributeValue *element_IF_1;
};
/* int	free_IF_AttributeValueAssertion (); */

struct type_SEC_IF_Name {
        struct type_SEC_IF_RelativeDistinguishedName *element_IF_2;

        struct type_SEC_IF_Name *next;
};
/* int	free_IF_Name (); */

struct type_SEC_IF_RelativeDistinguishedName {
        struct type_SEC_IF_AttributeValueAssertion *member_IF_0;

        struct type_SEC_IF_RelativeDistinguishedName *next;
};
/* int	free_IF_RelativeDistinguishedName (); */

struct type_SEC_IF_AttributeValues {
        struct type_SEC_IF_AttributeValue *member_IF_1;

        struct type_SEC_IF_AttributeValues *next;
};
/* int	free_IF_AttributeValues (); */

struct type_SEC_IF_Attribute {
    struct type_SEC_IF_AttributeType *type;

    struct type_SEC_IF_AttributeValues *values;
};
/* int	free_IF_Attribute (); */

#endif
