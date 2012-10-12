
int set_Element_point_ranges(struct Parse_state *state,
	void *element_point_ranges_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Modifier function to set an element_point_ranges. <element_point_ranges_address>
should point to a currently-NULL pointer to a struct Element_point_ranges. Upon
successful return an Element_point_ranges will be created and the pointer to it
returned in this location, for the calling function to use or destroy.
==============================================================================*/

