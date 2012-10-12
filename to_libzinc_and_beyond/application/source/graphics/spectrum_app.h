#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include "command/parser.h"
#include "api/cmiss_graphics_module.h"
int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/

int set_Spectrum_minimum(struct Spectrum *spectrum,float minimum);
int set_Spectrum_maximum(struct Spectrum *spectrum,float maximum);
int set_Spectrum_minimum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *Spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
*/
