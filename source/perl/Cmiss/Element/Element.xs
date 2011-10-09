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

static double
constant(char *name, int len, int arg)
{
	int return_code;

	errno = 0;
	if (strEQ(name, "LINEAR_LAGRANGE"))
	{
		return_code = LINEAR_LAGRANGE;
	}
	else
	{
    	errno = ENOENT;
		return_code = 0;
	}
   return (return_code);
}

MODULE = Cmiss::Element		PACKAGE = Cmiss::Element		PREFIX = Cmiss_region_

PROTOTYPES: DISABLE

double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

int
DESTROY(Cmiss::Element element)
	CODE:
		{
			struct Cmiss_element *temp_element;

			temp_element=element;
			RETVAL=DEACCESS(Cmiss_element)(&temp_element);
		}
	OUTPUT:
		RETVAL

Cmiss::Element
create_with_line_shape_xs(int identifier, \
	Cmiss::Region region, int dimension)
	CODE:
		if (RETVAL=create_Cmiss_element_with_line_shape(identifier, region, dimension))
		{
			ACCESS(Cmiss_element)(RETVAL);
		}
	OUTPUT:
		RETVAL

int
set_node_xs(Cmiss::Element element, int node_index, Cmiss::Node node)
	CODE:
		RETVAL=Cmiss_element_set_node(element, node_index, node);
	OUTPUT:
		RETVAL

int
get_identifier_xs(Cmiss::Element element)
	CODE:
		RETVAL=Cmiss_element_get_identifier(element);
	OUTPUT:
		RETVAL


