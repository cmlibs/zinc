/******************************************************************
  FILE: image_cache.h

  LAST MODIFIED: 18 February 2004

  DESCRIPTION: Define Image_cache structure and implement basic operation on Image_cache
==================================================================*/
#if !defined (IMAGE_CACHE_H)
#define IMAGE_CACHE_H

#include <stdio.h>
#include <math.h>
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
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Resizes the dynamic storage in the cache for the specified <dimension> and
<depth> and recording the sizes, minimums and maximums.
==============================================================================*/
int Image_cache_update_data_storage(struct Image_cache *image);
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Resizes the dynamic data storage in the cache.
==============================================================================*/
int Image_cache_update_from_fields(struct Image_cache *image,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int element_dimension,
	struct Cmiss_region *region, struct User_interface      *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 December 2003

DESCRIPTION :
Creates an Image_cache, <image> that represents the source field by evaluating the
<source_field> at each pixel and using the element_xi from find_element_xi on
the <texture_coordinate_field>.  The <image> is created with the specified
<dimension> and the corresponding vectors of number of pixels <sizes> and
<minimums> and <maximums> for the texture coordinate ranges.
An <element_dimension> of 0 searches in elements of all dimension, any other
value searches just elements of that dimension.
==============================================================================*/
int Image_cache_evaluate_field(struct Image_cache *image, struct Computed_field *field);
/*******************************************************************************
    LAST MODIFIED: 23 February 2004
    DESCRIPTION: Copy the result image to computed_field
=================================================================================*/

#endif /* !defined (IMAGE_CACHE_H) */
