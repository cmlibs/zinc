/*******************************************************************************
FILE : cell_types.h

LAST MODIFIED : 09 November 2000

DESCRIPTION :
The different types of values used in the Cell interface.
==============================================================================*/
#if !defined (CELL_TYPES_H)
#define CELL_TYPES_H

/*
Global types
------------
*/
enum Cell_value_type
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
Enumeration of the various value types used in Cell.
==============================================================================*/
{
  CELL_INTEGER_VALUE,
  CELL_REAL_VALUE,
  CELL_STRING_VALUE,
  CELL_UNKNOWN_VALUE
}; /* enum Cell_value_type */

#define CELL_INTEGER_FORMAT "%d"
typedef int CELL_INTEGER;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The integer value type
==============================================================================*/

#define CELL_REAL_FORMAT "%g"
typedef float CELL_REAL;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The real value type
==============================================================================*/

#define CELL_DOUBLE_FORMAT "%lf"
typedef double CELL_DOUBLE;
/*******************************************************************************
LAST MODIFIED : 28 October 2000

DESCRIPTION :
The double value type
==============================================================================*/

#define CELL_STRING_FORMAT "%s"
typedef char* CELL_STRING;
/*******************************************************************************
LAST MODIFIED : 02 July 2000

DESCRIPTION :
The string value type
==============================================================================*/

#endif /* !defined (CELL_TYPES_H) */
