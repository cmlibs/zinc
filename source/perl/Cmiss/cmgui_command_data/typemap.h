#if ! defined (PERL_CMISS_VALUE_TYPEMAP_H)
#define PERL_CMISS_VALUE_TYPEMAP_H

#include "perl/Cmiss/Region/typemap.h"
/* SAB Currently in cmgui it's command data is called struct Cmiss_command_data,
	in perl I am giving it a more specific name */
/*???DB.  The __ is from translating :: in Cmiss::Value */
typedef struct Cmiss_command_data * Cmiss__cmgui_command_data;

#endif /* ! defined (PERL_CMISS_VALUE_TYPEMAP_H) */
