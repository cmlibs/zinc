/*******************************************************************************
FILE : cmiss_finite_element.h

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_finite_elements.
==============================================================================*/
#ifndef __CMISS_FINITE_ELEMENT_H__
#define __CMISS_FINITE_ELEMENT_H__

/*
Global types
------------
*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_element FE_element

struct Cmiss_element;
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_element * Cmiss_element_id;

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
#define Cmiss_node FE_node

struct Cmiss_node;
/*******************************************************************************
LAST MODIFIED : 14 August 2002

DESCRIPTION :
==============================================================================*/

typedef struct Cmiss_node * Cmiss_node_id;

typedef int (*Cmiss_node_iterator_function)(struct Cmiss_node *node, void *user_data);

int Cmiss_node_get_identifier(struct Cmiss_node *node);
/*******************************************************************************
LAST MODIFIED : 1 April 2004

DESCRIPTION :
Returns the integer identifier of the <node>.
==============================================================================*/

/* SAB Temporary until we decide how to fix things up internally instead of externally.*/
enum FE_nodal_value_type
/*******************************************************************************
LAST MODIFIED : 27 January 1998

DESCRIPTION :
The type of a nodal value.
Must add new enumerators and keep values in sync with functions
ENUMERATOR_STRING, ENUMERATOR_GET_VALID_STRINGS and STRING_TO_ENUMERATOR.
Note these functions expect the first enumerator to be number 1, and all
subsequent enumerators to be sequential, unlike the default behaviour which
starts at 0.
==============================================================================*/
{
	FE_NODAL_VALUE,
	FE_NODAL_D_DS1,
	FE_NODAL_D_DS2,
	FE_NODAL_D_DS3,
	FE_NODAL_D2_DS1DS2,
	FE_NODAL_D2_DS1DS3,
	FE_NODAL_D2_DS2DS3,
	FE_NODAL_D3_DS1DS2DS3,
	FE_NODAL_UNKNOWN
}; /* enum FE_nodal_value_type */

#endif /* __CMISS_FINITE_ELEMENT_H__ */
