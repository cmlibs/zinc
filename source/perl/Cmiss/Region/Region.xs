#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "command/cmiss.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "typemap.h"

/*???DB.  want to use
	region/cmiss_region
		Cmiss_region_get_region_from_path
	finite_element/finite_element_region
		Cmiss_region_get_FE_region
		FE_region_get_FE_field_from_name
		FE_region_get_FE_node_from_identifier
		FE_region_get_FE_element_from_identifier
	*/

MODULE = Cmiss::Region		PACKAGE = Cmiss::Region		PREFIX = Cmiss_region_

PROTOTYPES: DISABLE

Cmiss::Region
create()
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_region structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_region_2=$cmiss_region_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_region)())
		{
			ACCESS(Cmiss_region)(RETVAL);
		}
	OUTPUT:
		RETVAL

int
DESTROY(Cmiss::Region region)
	CODE:
		{
			struct Cmiss_region *temp_region;

			temp_region=region;
			RETVAL=DEACCESS(Cmiss_region)(&temp_region);
		}
	OUTPUT:
		RETVAL

Cmiss::Region
command_data_get_root_region(Cmiss::cmgui_command_data cmgui_command_data)
	CODE:
		if (RETVAL=Cmiss_command_data_get_root_region(cmgui_command_data))
		{
			ACCESS(Cmiss_region)(RETVAL);
		}
	OUTPUT:
		RETVAL

Cmiss::FE_element
region_get_element(Cmiss::Region region,char *path,char *name,char *type)
	CODE:
		RETVAL=0;
		if (region&&path&&name&&type)
		{
			int name_length;
			struct CM_element_information identifier;
			struct Cmiss_region *sub_region;
			struct FE_region *fe_region;

			if (Cmiss_region_get_region_from_path(region,path,&sub_region)&&
				sub_region&&(fe_region=Cmiss_region_get_FE_region(sub_region)))
			{
				if (strcmp(type,"element"))
				{
					if (strcmp(type,"face"))
					{
						if (strcmp(type,"line"))
						{
							identifier.type=CM_ELEMENT_TYPE_INVALID;
						}
						else
						{
							identifier.type=CM_LINE;
						}
					}
					else
					{
						identifier.type=CM_FACE;
					}
				}
				else
				{
					identifier.type=CM_ELEMENT;
				}
				if ((CM_ELEMENT_TYPE_INVALID!=identifier.type)&&
					(1==sscanf(name," %d %n",&(identifier.number),&name_length))&&
					(name_length==strlen(name)))
				{
					RETVAL=FE_region_get_FE_element_from_identifier(fe_region,
						&identifier);
				}
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::FE_field
region_get_field(Cmiss::Region region,char *path,char *name)
	CODE:
		RETVAL=0;
		if (region&&path&&name)
		{
			struct Cmiss_region *sub_region;
			struct FE_region *fe_region;

			if (Cmiss_region_get_region_from_path(region,path,&sub_region)&&
				sub_region&&(fe_region=Cmiss_region_get_FE_region(sub_region)))
			{
				RETVAL=FE_region_get_FE_field_from_name(fe_region,name);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::FE_node
region_get_node(Cmiss::Region region,char *path,char *name)
	CODE:
		RETVAL=0;
		if (region&&path&&name)
		{
			int name_length,node_number;
			struct Cmiss_region *sub_region;
			struct FE_region *fe_region;

			if (Cmiss_region_get_region_from_path(region,path,&sub_region)&&
				sub_region&&(fe_region=Cmiss_region_get_FE_region(sub_region))&&
				(1==sscanf(name," %d %n",&node_number,&name_length))&&
				(name_length==strlen(name)))
			{
				RETVAL=FE_region_get_FE_node_from_identifier(fe_region,node_number);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Region
region_get_sub_region(Cmiss::Region region,char *name)
	CODE:
		RETVAL=0;
		if (region&&name)
		{
			if (Cmiss_region_get_region_from_path(region,name,&RETVAL))
			{
				ACCESS(Cmiss_region)(RETVAL);
			}
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL
