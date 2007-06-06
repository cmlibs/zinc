int gfx_minimise(struct Parse_state *state, void *dummy_to_be_modified,
	void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 04 May 2007

DESCRIPTION :
Minimises the <objective_field> by changing the <independent_field> over a 
<region>
==============================================================================*/

struct Minimisation_package *CREATE(Minimisation_package)(
	struct Time_keeper *time_keeper,
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *cmiss_region);
/*******************************************************************************
LAST MODIFIED : 7 May 2007

DESCRIPTION :
Creates the package required for minimisation.
==============================================================================*/

int DESTROY(Minimisation_package)(struct Minimisation_package **package);
/*******************************************************************************
LAST MODIFIED : 7 May 2007

DESCRIPTION :
Creates the package required for minimisation.
==============================================================================*/

