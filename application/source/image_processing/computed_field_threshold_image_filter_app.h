

#if !defined (COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_H_)
#define COMPUTED_FIELD_THRESHOLD_IMAGE_FILTER_H_

#include "general/debug.h"
#include "general/enumerator_app.h"
#include "general/message.h"
#include "command/parser.h"

PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(General_threshold_filter_mode);

int Computed_field_register_types_threshold_image_filter(
	struct Computed_field_package *computed_field_package);

#endif
