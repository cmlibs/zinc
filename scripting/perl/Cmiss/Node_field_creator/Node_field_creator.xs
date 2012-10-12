#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "api/cmiss_region.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/read_fieldml.h"
#include "general/io_stream.h"
#include "typemap.h"

MODULE = Cmiss::Node_field_creator		PACKAGE = Cmiss::Node_field_creator		PREFIX = Cmiss_region_

PROTOTYPES: DISABLE

Cmiss::Node_field_creator
create(int number_of_components)
	CODE:
		RETVAL=CREATE(Cmiss_node_field_creator)(number_of_components);
	OUTPUT:
		RETVAL

int
DESTROY(Cmiss::Node_field_creator node_field_creator)
	CODE:
		{
			struct Cmiss_node_field_creator *temp_node_field_creator;

			temp_node_field_creator=node_field_creator;
			RETVAL=DESTROY(Cmiss_node_field_creator)(&temp_node_field_creator);
		}
	OUTPUT:
		RETVAL

