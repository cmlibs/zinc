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

MODULE = Cmiss::Node		PACKAGE = Cmiss::Node		PREFIX = Cmiss_region_

PROTOTYPES: DISABLE

Cmiss::Node
create(int identifier, Cmiss::Region region)
	CODE:
		if (RETVAL=create_Cmiss_node(identifier, region))
		{
			ACCESS(Cmiss_node)(RETVAL);
		}
	OUTPUT:
		RETVAL

Cmiss::Node
create_from_template(int identifier, Cmiss::Node template)
	CODE:
		if (RETVAL=create_Cmiss_node_from_template(identifier, template))
		{
			ACCESS(Cmiss_node)(RETVAL);
		}
	OUTPUT:
		RETVAL

int
DESTROY(Cmiss::Node node)
	CODE:
		{
			struct Cmiss_node *temp_node;

			temp_node=node;
			RETVAL=DEACCESS(Cmiss_node)(&temp_node);
		}
	OUTPUT:
		RETVAL

int
get_identifier_xs(Cmiss::Node node)
	CODE:
		RETVAL=Cmiss_node_get_identifier(node);
	OUTPUT:
		RETVAL

