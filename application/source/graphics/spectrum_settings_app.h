
#if !defined (SPECTRUM_SETTINGS_APP_H_)
#define SPECTRUM_SETTINGS_APP_H_

#include "general/enumerator_app.h"

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void);
/*******************************************************************************
LAST MODIFIED : 17 January 2001

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LOG command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_spectrum_settings_field(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void);
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM FIELD command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(Spectrum_settings_colour_mapping);

#endif
