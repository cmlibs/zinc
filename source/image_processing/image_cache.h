#if !defined (IMAGE_CACHE_H)
#define IMAGE_CACHE_H

#include <stdio.h>
#include <math.h>
//#include <time.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"

struct Image_cache
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Stores an intermediate representation of a field in an image structure so that
spatially dependent algorithms can be implemented in computed fields.
==============================================================================*/
{
	int dimension;
	int *sizes;
	FE_value *minimums;
	FE_value *maximums;
	int depth;
	char *data;

	int valid;

	int access_count;
}; /* struct Image_cache */

struct Image_cache *CREATE(Image_cache)(void);
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
==============================================================================*/
int DESTROY(Image_cache)(struct Image_cache **image_address);
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Frees memory/deaccess mapping at <*mapping_address>.
==============================================================================*/
PROTOTYPE_OBJECT_FUNCTIONS(Image_cache);

int Image_cache_update_dimension(struct Image_cache *image,
	int dimension, int depth, int *sizes, FE_value *minimums, FE_value *maximums);

int Image_cache_update_data_storage(struct Image_cache *image);

int Image_cache_update_from_fields(struct Image_cache *image,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int element_dimension,
	struct Cmiss_region *region, struct User_interface      *user_interface);

int Copy_image_to_field(struct Image_cache *image, struct Computed_field *field);
/*******************************************************************************
    LAST MODIFIED: 23 February 2004
    DESCRIPTION: Copy the result image to computed_field
=================================================================================*/

#endif /* !defined (IMAGE_CACHE_H) */
