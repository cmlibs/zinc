/*******************************************************************************
FILE : random.h

LAST MODIFIED : 4 May 2001

DESCRIPTION :
Standard macros for returning random numbers.
==============================================================================*/
#if !defined (RANDOM_H)
#define RANDOM_H

/* Must #include <stdlib.h> in calling module to use: */
/* Returns a random number of the given <type> in the range [0.0, 1.0]
	 ie. INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM(type) ((type)random() / 2147483647.0)

/* Returns a random number of the given <type> in the range (0.0, 1.0)
	 ie. NOT INCLUDING 0.0 and 1.0 in the possible results */
#define CMGUI_RANDOM_NON_INCLUSIVE(type) (((type)random() + 1.0) / 2147483649.0)

/* partner random number seed function for CMGUI_RANDOM */
#define CMGUI_SEED_RANDOM(seed) srandom(seed)

#endif /* !defined (RANDOM_H) */
