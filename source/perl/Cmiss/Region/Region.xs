#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdio.h>
#include "api/cmiss_region.h"
#include "api/cmiss_field.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/import_finite_element.h"
#include "finite_element/read_fieldml.h"
#include "general/io_stream.h"
#include "typemap.h"

MODULE = Cmiss::Region		PACKAGE = Cmiss::Region		PREFIX = Cmiss_region_

PROTOTYPES: DISABLE

Cmiss::Region
create()
	PREINIT:
		struct FE_region *fe_region;
		struct LIST(FE_element_shape) *element_shape_list;
		struct MANAGER(FE_basis) *basis_manager;
	CODE:
		/* the result, in Perl, is a reference to a stash (which is a pointer to the
			Cmiss_region structure).  This means that don't need to worry about
			ACCESSing for Perl assignment/copy, $cmiss_region_2=$cmiss_region_1,
			because this increments the reference count for the stash (DESTROY is
			called when the stash reference count gets to zero) */
		if (RETVAL=CREATE(Cmiss_region)())
		{
			ACCESS(Cmiss_region)(RETVAL);
			if ((basis_manager=CREATE_MANAGER(FE_basis)()) && 
				(element_shape_list=CREATE(LIST(FE_element_shape))()))
			{
				if (fe_region=CREATE(FE_region)((struct FE_region *)NULL,basis_manager,
					element_shape_list))
				{
					if (!Cmiss_region_attach_FE_region(RETVAL,fe_region))
					{
						DEACCESS(Cmiss_region)(&RETVAL);
					}
				}
				else
				{
					DEACCESS(Cmiss_region)(&RETVAL);
				}
			}
			else
			{
				DEACCESS(Cmiss_region)(&RETVAL);
			}
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

int
region_read_file(Cmiss::Region region,char *file_name);
	PREINIT:
		struct LIST(FE_element_shape) *element_shape_list;
		struct MANAGER(FE_basis) *basis_manager;
	CODE:
		RETVAL=0;
		if (region&&file_name&&(basis_manager=FE_region_get_basis_manager(
			Cmiss_region_get_FE_region(region)))&&(element_shape_list=
			FE_region_get_FE_element_shape_list(Cmiss_region_get_FE_region(region))))
		{
			char *fieldml;
			struct Cmiss_region *temp_region;

			temp_region=(struct Cmiss_region *)NULL;
			if (fieldml=strrchr(file_name,'.'))
			{
				fieldml++;
				if (0!=strcmp(fieldml,"fml"))
				{
					fieldml=(char *)NULL;
				}
			}
			if (fieldml)
			{
				temp_region=parse_fieldml_file(file_name,basis_manager,
					element_shape_list);
			}
			else
			{
				struct IO_stream_package *io_stream_package;

				if (io_stream_package=CREATE(IO_stream_package)())
				{
					temp_region=read_exregion_file_of_name(file_name,io_stream_package,
						basis_manager,element_shape_list,
						(struct FE_import_time_index *)NULL);
					DESTROY(IO_stream_package)(&io_stream_package);
				}
			}
			if (temp_region)
			{
				ACCESS(Cmiss_region)(temp_region);
				if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
				{
					RETVAL=Cmiss_regions_merge_FE_regions(region,temp_region);
				}
				DEACCESS(Cmiss_region)(&temp_region);
			}
		}
	OUTPUT:
		RETVAL

Cmiss::Element
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
					((unsigned int)name_length==strlen(name)))
				{
					RETVAL=FE_region_get_FE_element_from_identifier(fe_region,
						&identifier);
				}
			}
		}
		if (RETVAL)
		{
			ACCESS(Cmiss_element)(RETVAL);
		}
		else
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Node
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
				((unsigned int)name_length==strlen(name)))
			{
				RETVAL=FE_region_get_FE_node_from_identifier(fe_region,node_number);
			}
		}
		if (RETVAL)
		{
			ACCESS(Cmiss_node)(RETVAL);
		}
		else
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

int
begin_change(Cmiss::Region region)
	CODE:
		RETVAL=0;
		if (region)
		{
			RETVAL=Cmiss_region_begin_change_API(region);
		}
	OUTPUT:
		RETVAL

int
end_change(Cmiss::Region region)
	CODE:
		RETVAL=0;
		if (region)
		{
			RETVAL=Cmiss_region_end_change_API(region);
		}
	OUTPUT:
		RETVAL

Cmiss::Node
merge_Cmiss_node_xs(Cmiss::Region region, Cmiss::Node node)
	CODE:
		if (region && node)
		{
			RETVAL=Cmiss_region_merge_Cmiss_node(region, node);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Element
merge_Cmiss_element_xs(Cmiss::Region region, Cmiss::Element element)
	CODE:
		if (region && element)
		{
			RETVAL=Cmiss_region_merge_Cmiss_element(region, element);
		}
		if (!RETVAL)
		{
			XSRETURN_UNDEF;
		}
	OUTPUT:
		RETVAL

Cmiss::Field
Cmiss_region_find_field_by_name(Cmiss::Region region, char *name)
   POSTCALL:
	if (!RETVAL)
		XSRETURN_UNDEF;

int
Cmiss_region_is_field_defined(Cmiss::Region region, char *name)

Cmiss::Field
Cmiss_region_add_field(Cmiss::Region region, Cmiss::Field field)
   POSTCALL:
	if (RETVAL)
		/* Add the reference as Cmiss_region_add_field does not */
		Cmiss_field_access(RETVAL);
   else
		XSRETURN_UNDEF;

