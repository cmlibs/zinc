/*******************************************************************************
FILE : random.h

LAST MODIFIED : 27 November 2000

DESCRIPTION :
Standard macros for returning random numbers.
==============================================================================*/
#if !defined (RANDOM_H)
#define RANDOM_H

/* Must #include <stdlib.h> in calling module to use: */
/* Returns a random number of the given <type> ranging from 0.0 to 1.0 */
#define CMGUI_RANDOM(type) ((type)random() / 2147483647.0)

/* partner random number seed function for CMGUI_RANDOM */
#define CMGUI_SEED_RANDOM(seed) srandom(seed)

#endif /* !defined (RANDOM_H) */
