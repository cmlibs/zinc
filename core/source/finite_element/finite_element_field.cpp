/**
 * FILE : finite_element_field.cpp
 *
 * Internal class for field interpolated over finite elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/status.h"
#include "finite_element/finite_element_field_private.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_value_storage.hpp"
#include "general/change_log_private.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "general/indexed_list_private.h"
#include "general/message.h"
#include "general/mystring.h"

/*
Module types
------------
*/

FULL_DECLARE_CHANGE_LOG_TYPES(FE_field);

/*
Module functions
----------------
*/

DECLARE_CHANGE_LOG_MODULE_FUNCTIONS(FE_field)

static char *get_automatic_component_name(char **component_names,
	int component_no)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Used internally by some FE_field functions. Returns an allocated string
containing the component name for <component_no> (starting at 0 for the first).
If <component_names> or component_names[component_no] are NULL a name consisting
of the value component_no+1 is created and returned.
If <component_names> is not NULL, the function assumes that component_no is
less than the number of names in this array.
It is up to the calling function to deallocate the returned string.
==============================================================================*/
{
	char *component_name,*source_name,temp_string[20];

	ENTER(get_automatic_component_name);
	if (0<=component_no)
	{
		if (component_names&&component_names[component_no])
		{
			source_name=component_names[component_no];
		}
		else
		{
			sprintf(temp_string,"%i",component_no+1);
			source_name=temp_string;
		}
		component_name = duplicate_string(source_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_automatic_component_name.  Invalid component_no");
		component_name=(char *)NULL;
	}
	LEAVE;

	return (component_name);
} /* get_automatic_component_name */

/*
Global functions
----------------
*/

/**
 * Get definition of field on element, or higher dimension element it is a face
 * of, or a face of a face. If field is inherited, returns the xi coordinate
 * transformation to the higher dimensional element.
 * Can also force inheritance onto a specified face of the supplied element.
 * Function is recursive.
 *
 * @param element  The element we want to evaluate on.
 * @param inheritFaceNumber  If non-negative, inherit onto this face number
 * of element, as if the face element were supplied to this function.
 * @param topLevelElement  If supplied, forces field definition to be
 * obtained from it, otherwise field is undefined.
 * @param coordinateTransformation  If field is inherited from higher dimension
 * element, gives mapping of element coordinates to return element coordinates.
 * This is a matrix of dimension(return element) rows X dimension(element)+1
 * columns. This represents an affine transformation, b + A xi for calculating
 * the return element xi coordinates from those of element, where b is the
 * first column of the <coordinate_transformation> matrix. Caller must pass an
 * array of size sufficient for the expected dimensions, e.g.
 * MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS.
 * @return  The element the field is actually defined on, or 0 if not defined.
 * Return element either matches supplied element or is an ancestor of it.
 */
cmzn_element *FE_field::getOrInheritOnElement(cmzn_element *element,
	int inheritFaceNumber, cmzn_element *topLevelElement,
	FE_value *coordinateTransformation)
{
	const FE_mesh *mesh = element->getMesh();
	if (!mesh)
		return 0;
	const int dim = mesh->getDimension() - 1;
	// fast case of field directly defined on element:
	// check first component only
	// note this formerly only used topLevelElement for inheritance only
	// i.e. would return initial element if it had field directly defined on it
	const bool useDefinitionOnThisElement = ((!topLevelElement) || (element == topLevelElement))
		&& (0 != this->meshFieldData[dim])
		&& (this->meshFieldData[dim]->getComponentMeshfieldtemplate(0)->getElementEFTIndex(element->getIndex()) >= 0);
	if (useDefinitionOnThisElement && (inheritFaceNumber < 0))
		return element;

	FE_value parentCoordinateTransformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_element *fieldElement = 0;
	cmzn_element *parent = 0;
	int faceNumber = inheritFaceNumber;
	if (inheritFaceNumber >= 0)
	{
		parent = element;
		if (useDefinitionOnThisElement)
			fieldElement = element;
		else // inherit onto this element, then onto specified face below to handle two coordinate transformations
			fieldElement = this->getOrInheritOnElement(parent, /*inheritFaceNumber*/-1, topLevelElement, parentCoordinateTransformation);
	}
	else
	{
		const FE_mesh *parentMesh = mesh->getParentMesh();
		if (!parentMesh)
			return 0;
		// try to inherit field from any of the element's parents
		const DsLabelIndex *parents;
		const int parentsCount = mesh->getElementParents(element->getIndex(), parents);
		for (int p = 0; p < parentsCount; ++p)
		{
			parent = parentMesh->getElement(parents[p]);
			fieldElement = this->getOrInheritOnElement(parent, /*inheritFaceNumber*/-1, topLevelElement, parentCoordinateTransformation);
			if (fieldElement)
			{
				faceNumber = parentMesh->getElementFaceNumber(parents[p], element->getIndex());
				break;
			}
		}
	}
	if (!fieldElement)
		return 0;
	FE_element_shape *parentShape = parent->getElementShape();
	const FE_value *faceToElement = get_FE_element_shape_face_to_element(parentShape, faceNumber);
	if (!faceToElement)
	{
		display_message(ERROR_MESSAGE, "FE_field::getOrInheritOnElement.  Missing parent shape or invalid face number");
		return 0;
	}
	const int parentDimension = get_FE_element_shape_dimension(parentShape);
	const int fieldElementDimension = fieldElement->getDimension();
	if (fieldElementDimension > parentDimension)
	{
		// incorporate the face to element map in the coordinate transformation
		const FE_value *parentValue = parentCoordinateTransformation;
		FE_value *faceValue = coordinateTransformation;
		const int faceDimension = parentDimension - 1;
		// this had used DOUBLE_FOR_DOT_PRODUCT, but FE_value will be at least double precision now
		FE_value sum;
		const FE_value *faceToElementValue;
		for (int i = fieldElementDimension; i > 0; --i)
		{
			/* calculate b entry for this row */
			sum = *parentValue;
			++parentValue;
			faceToElementValue = faceToElement;
			for (int k = parentDimension; k > 0; --k)
			{
				sum += (*parentValue)*(*faceToElementValue);
				++parentValue;
				faceToElementValue += parentDimension;
			}
			*faceValue = sum;
			faceValue++;
			/* calculate A entries for this row */
			for (int j = faceDimension; j > 0; --j)
			{
				++faceToElement;
				faceToElementValue = faceToElement;
				parentValue -= parentDimension;
				sum = 0.0;
				for (int k = parentDimension; k > 0; --k)
				{
					sum += (*parentValue)*(*faceToElementValue);
					++parentValue;
					faceToElementValue += parentDimension;
				}
				*faceValue = sum;
				++faceValue;
			}
			faceToElement -= faceDimension;
		}
	}
	else
	{
		// use the face to element map as the transformation
		memcpy(coordinateTransformation, faceToElement, parentDimension*parentDimension*sizeof(FE_value));
	}
	return fieldElement;
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(CM_field_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(CM_field_type));
	switch (enumerator_value)
	{
		case CM_ANATOMICAL_FIELD:
		{
			enumerator_string = "anatomical";
		} break;
		case CM_COORDINATE_FIELD:
		{
			enumerator_string = "coordinate";
		} break;
		case CM_GENERAL_FIELD:
		{
			enumerator_string = "field";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(CM_field_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(CM_field_type)

struct FE_field_info *CREATE(FE_field_info)(struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Creates a struct FE_field_info with a pointer to <fe_region>.
Note:
This should only be called by FE_region functions, and the FE_region must be
its own master. The returned object is owned by the FE_region.
It maintains a non-ACCESSed pointer to its owning FE_region which the FE_region
will clear before it is destroyed. If it becomes necessary to have other owners
of these objects, the common parts of it and FE_region should be extracted to a
common object.
==============================================================================*/
{
	struct FE_field_info *fe_field_info;

	ENTER(CREATE(FE_field_info));
	fe_field_info = (struct FE_field_info *)NULL;
	if (fe_region)
	{
		if (ALLOCATE(fe_field_info, struct FE_field_info, 1))
		{
			/* maintain pointer to the the FE_region this information belongs to.
				 It is not ACCESSed since FE_region is the owning object and it
				 would prevent the FE_region from being destroyed. */
			fe_field_info->fe_region = fe_region;
			fe_field_info->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_field_info).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(FE_field_info).  Invalid argument(s)");
	}
	LEAVE;

	return (fe_field_info);
} /* CREATE(FE_field_info) */

int DESTROY(FE_field_info)(
	struct FE_field_info **fe_field_info_address)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Destroys the FE_field_info at *<field_info_address>. Frees the
memory for the information and sets <*field_info_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_field_info *fe_field_info;

	ENTER(DESTROY(FE_field_info));
	if ((fe_field_info_address) &&
		(fe_field_info = *fe_field_info_address))
	{
		if (0 == fe_field_info->access_count)
		{
			DEALLOCATE(*fe_field_info_address);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_field_info).  Non-zero access count");
			return_code = 0;
		}
		*fe_field_info_address = (struct FE_field_info *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(FE_field_info).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_field_info) */

DECLARE_OBJECT_FUNCTIONS(FE_field_info)

int FE_field_info_clear_FE_region(struct FE_field_info *field_info)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Clears the pointer to FE_region in <field_info>.
Private function only to be called by destroy_FE_region.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_info_clear_FE_region);
	if (field_info)
	{
		field_info->fe_region = (struct FE_region *)NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_info_clear_FE_region.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_info_clear_FE_region */

struct FE_field *CREATE(FE_field)(const char *name, struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Creates and returns a struct FE_field of <name> belonging to the ultimate
master FE_region of <fe_region>. The new field has no name/identifier, zero
components, field_type FIELD, NOT_APPLICABLE coordinate system, no field values.
???RC Used to pass <fe_time> in here and store in FE_field; can now get it from
FE_region.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(CREATE(FE_field));
	field = (struct FE_field *)NULL;
	if (name && fe_region)
	{
		if (ALLOCATE(field, struct FE_field, 1))
		{
			return_code = 1;
			if (!(field->name = duplicate_string(name)))
			{
				return_code = 0;
			}
			/* get and ACCESS FE_field_info relating this field back to fe_region */
			if (!(field->info =
				ACCESS(FE_field_info)(FE_region_get_FE_field_info(fe_region))))
			{
				return_code = 0;
			}
			field->fe_field_type = GENERAL_FE_FIELD;
			field->indexer_field = (struct FE_field *)NULL;
			field->number_of_indexed_values = 0;
			field->cm_field_type = CM_GENERAL_FIELD;
			field->number_of_components = 0;
			/* don't allocate component names until we have custom names */
			field->component_names = (char **)NULL;
			field->coordinate_system.type = NOT_APPLICABLE;
			field->number_of_values = 0;
			field->values_storage = (Value_storage *)NULL;
			field->value_type = UNKNOWN_VALUE;
			field->element_xi_host_mesh = 0;
			field->number_of_times = 0;
			field->time_value_type = UNKNOWN_VALUE;
			field->times = (Value_storage *)NULL;
			for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
				field->meshFieldData[d] = 0;
			field->number_of_wrappers = 0;
			field->access_count = 0;
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"CREATE(FE_field).  Could not construct contents");
				DEALLOCATE(field);
				field = (struct FE_field *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(FE_field).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "CREATE(FE_field).  Invalid argument(s)");
	}
	LEAVE;

	return (field);
} /* CREATE(FE_field) */

int DESTROY(FE_field)(struct FE_field **field_address)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Frees the memory for the field and sets <*field_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_field *field;

	ENTER(DESTROY(FE_field));
	if ((field_address)&&(field= *field_address))
	{
		if (0==field->access_count)
		{
			if (field->element_xi_host_mesh)
				FE_mesh::deaccess(field->element_xi_host_mesh);
			for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
				delete field->meshFieldData[d];

			/* free the field name */
			if (field->name)
			{
				DEALLOCATE(field->name);
			}
			DEACCESS(FE_field_info)(&field->info);
			REACCESS(FE_field)(&(field->indexer_field),(struct FE_field *)NULL);
			if (field->values_storage)
			{
				/* free any arrays pointed to by field->values_storage */
				free_value_storage_array(field->values_storage,field->value_type,
					(struct FE_time_sequence *)NULL,field->number_of_values);
				/* free the global values */
				DEALLOCATE(field->values_storage);
			}

			// free component names
			if (field->component_names)
			{
				for (int c = 0; c < field->number_of_components; ++c)
					DEALLOCATE(field->component_names[c]);
				DEALLOCATE(field->component_names);
			}

			DEALLOCATE(*field_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(FE_field).  Non-zero access_count (%d)",field->access_count);
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_field) */

int list_FE_field(struct FE_field *field,void *dummy)
/*******************************************************************************
LAST MODIFIED : 15 May 2003

DESCRIPTION :
Outputs the information contained in <field>.
==============================================================================*/
{
	char *component_name;
	int i, number_of_components, return_code;

	ENTER(list_FE_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code=1;
		/* write the identifier */
		display_message(INFORMATION_MESSAGE, "field : %s\n", field->name);
		display_message(INFORMATION_MESSAGE,
			"  access count = %d\n", field->access_count);
		display_message(INFORMATION_MESSAGE,"  type = %s",
			ENUMERATOR_STRING(CM_field_type)(field->cm_field_type));
		display_message(INFORMATION_MESSAGE,"  coordinate system = %s",
			ENUMERATOR_STRING(Coordinate_system_type)(field->coordinate_system.type));
		number_of_components=field->number_of_components;
		display_message(INFORMATION_MESSAGE,", #Components = %d\n",
			number_of_components);
		i=0;
		while (return_code&&(i<number_of_components))
		{
			if (NULL != (component_name = get_FE_field_component_name(field, i)))
			{
				display_message(INFORMATION_MESSAGE,"    %s", component_name);
				DEALLOCATE(component_name);
			}
			/* display field based information*/
			if (field->number_of_values)
			{
				int count;

				display_message(INFORMATION_MESSAGE,"field based values: ");
				switch (field->value_type)
				{
					case FE_VALUE_VALUE:
					{
						display_message(INFORMATION_MESSAGE,"\n");
						/* output in columns if FE_VALUE_MAX_OUTPUT_COLUMNS > 0 */
						for (count=0;count<field->number_of_values;count++)
						{
							display_message(INFORMATION_MESSAGE, " %" FE_VALUE_STRING,
								*((FE_value*)(field->values_storage + count*sizeof(FE_value))));
							if ((0<FE_VALUE_MAX_OUTPUT_COLUMNS)&&
								(0==((count+1) % FE_VALUE_MAX_OUTPUT_COLUMNS)))
							{
								display_message(INFORMATION_MESSAGE,"\n");
							}
						}
					} break;
					default:
					{
						display_message(INFORMATION_MESSAGE,"list_FE_field: "
							"Can't display that field value_type yet. Write the code!");
					} break;
				}	/* switch () */
			}
			display_message(INFORMATION_MESSAGE,"\n");
			i++;
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,"list_FE_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_FE_field */

DECLARE_OBJECT_FUNCTIONS(FE_field)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(FE_field)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(FE_field)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(FE_field,name,const char *)
DECLARE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(FE_field,name)

DECLARE_CHANGE_LOG_FUNCTIONS(FE_field)

int FE_field_copy_without_identifier(struct FE_field *destination,
	struct FE_field *source)
{
	if (!(destination && destination->info && destination->info->fe_region
		&& source && source->info && source->info->fe_region))
	{
		display_message(ERROR_MESSAGE,
			"FE_field_copy_without_identifier.  Invalid argument(s)");
		return 0;
	}
	int return_code = 1;
	const bool externalMerge = destination->info->fe_region != source->info->fe_region;
	char **component_names=(char **)NULL;
	if (source->component_names)
	{
		ALLOCATE(component_names, char *, source->number_of_components);
		if (!component_names)
			return false;
		for (int c = 0; c < source->number_of_components; ++c)
		{
			if (source->component_names[c])
			{
				component_names[c] = duplicate_string(source->component_names[c]);
				if (!component_names[c])
				{
					for (int c2 = 0; c2 < c; ++c)
						DEALLOCATE(component_names[c2]);
					DEALLOCATE(component_names);
					return_code = 0;
					break;
				}
			}
			else
			{
				component_names[c] = 0;
			}
		}
	}
	Value_storage *values_storage = 0;
	Value_storage *times = 0;
	if (0<source->number_of_values)
	{
		if (!((values_storage=make_value_storage_array(source->value_type,
			(struct FE_time_sequence *)NULL,source->number_of_values))&&
			copy_value_storage_array(values_storage,source->value_type,
				(struct FE_time_sequence *)NULL,(struct FE_time_sequence *)NULL,
				source->number_of_values,source->values_storage, /*optimised_merge*/0)))
		{
			return_code=0;
		}
	}
	if (0<source->number_of_times)
	{
		if (!((times=make_value_storage_array(source->time_value_type,
			(struct FE_time_sequence *)NULL,source->number_of_times))&&
			copy_value_storage_array(times,source->time_value_type,
				(struct FE_time_sequence *)NULL,(struct FE_time_sequence *)NULL,
				source->number_of_times,source->times, /*optimised_merge*/0)))
		{
			return_code=0;
		}
	}
	FE_field *indexer_field = 0;
	if (INDEXED_FE_FIELD == source->fe_field_type)
	{
		if (!source->indexer_field)
			return_code = 0;
		else
		{
			if (externalMerge)
			{
				indexer_field = FE_region_get_FE_field_from_name(destination->info->fe_region, source->indexer_field->name);
				if (!indexer_field)
					return_code = 0;
			}
			else
			{
				indexer_field = source->indexer_field;
			}
		}
	}
	const FE_mesh *element_xi_host_mesh = 0;
	if (ELEMENT_XI_VALUE == source->value_type)
	{
		if (!source->element_xi_host_mesh)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_copy_without_identifier.  Source element:xi valued field %s does not have a host mesh", source->name);
			return_code = 0;
		}
		else if (externalMerge)
		{
			// find equivalent host mesh in destination FE_region
			// to be fixed in future when arbitrary meshes are allowed:
			element_xi_host_mesh = FE_region_find_FE_mesh_by_dimension(destination->info->fe_region, source->element_xi_host_mesh->getDimension());
			if (strcmp(element_xi_host_mesh->getName(), source->element_xi_host_mesh->getName()) != 0)
			{
				display_message(ERROR_MESSAGE,
					"FE_field_copy_without_identifier.  Cannot find destination host mesh named %s for merging 'element:xi' valued field %s. Needs to be implemented.",
					source->element_xi_host_mesh->getName(), source->name);
				return_code = 0;
			}
		}
		else
		{
			element_xi_host_mesh = source->element_xi_host_mesh;
		}
	}
	if (return_code)
	{
		// don't want to change info if merging to external region. In all other cases should be same info anyway
		if (!externalMerge)
			REACCESS(FE_field_info)(&(destination->info), source->info);
		if (destination->cm_field_type != source->cm_field_type)
		{
			display_message(WARNING_MESSAGE, "Changing field %s CM type from %s to %s",
				source->name, ENUMERATOR_STRING(CM_field_type)(destination->cm_field_type),
				ENUMERATOR_STRING(CM_field_type)(source->cm_field_type));
			destination->cm_field_type = source->cm_field_type;
		}
		destination->fe_field_type=source->fe_field_type;
		REACCESS(FE_field)(&(destination->indexer_field), indexer_field);
		destination->number_of_indexed_values=
			source->number_of_indexed_values;
		if (destination->component_names)
		{
			for (int i = 0; i < destination->number_of_components; i++)
			{
				if (destination->component_names[i])
				{
					DEALLOCATE(destination->component_names[i]);
				}
			}
			DEALLOCATE(destination->component_names);
		}
		destination->number_of_components=source->number_of_components;
		destination->component_names=component_names;
		COPY(Coordinate_system)(&(destination->coordinate_system),
			&(source->coordinate_system));
		destination->value_type=source->value_type;
		if (element_xi_host_mesh)
			element_xi_host_mesh->access();
		if (destination->element_xi_host_mesh)
			FE_mesh::deaccess(destination->element_xi_host_mesh);
		destination->element_xi_host_mesh = element_xi_host_mesh;
		destination->time_value_type=source->time_value_type;
		/* replace old values_storage with new */
		if (0<destination->number_of_values)
		{
			free_value_storage_array(destination->values_storage,
				destination->value_type,(struct FE_time_sequence *)NULL,
				destination->number_of_values);
			DEALLOCATE(destination->values_storage);
		}
		destination->number_of_values=source->number_of_values;
		destination->values_storage=values_storage;
		/* replace old times with new */
		if (0<destination->number_of_times)
		{
			free_value_storage_array(destination->times,
				destination->time_value_type,(struct FE_time_sequence *)NULL,
				destination->number_of_times);
			DEALLOCATE(destination->times);
		}
		destination->number_of_times=source->number_of_times;
		destination->times=times;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_copy_without_identifier.  Invalid source field or could not copy dynamic contents");
	}
	if (!return_code)
	{
		if (component_names)
		{
			for (int i=0;i<source->number_of_components;i++)
			{
				if (component_names[i])
				{
					DEALLOCATE(component_names[i]);
				}
			}
			DEALLOCATE(component_names);
		}
		if (values_storage)
		{
			free_value_storage_array(values_storage,source->value_type,
				(struct FE_time_sequence *)NULL,source->number_of_values);
			DEALLOCATE(values_storage);
		}
		if (times)
		{
			free_value_storage_array(times,source->time_value_type,
				(struct FE_time_sequence *)NULL,source->number_of_times);
			DEALLOCATE(times);
		}
	}
	return (return_code);
}

bool FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2)
{
	if (!(field1 && field2))
	{
		display_message(ERROR_MESSAGE,
			"FE_fields_match_fundamental.  Missing field(s)");
		return false;
	}
	if (!((field1->value_type == field2->value_type)
		&& (field1->fe_field_type == field2->fe_field_type)
		&& (field1->number_of_components == field2->number_of_components)
		&& (0 != Coordinate_systems_match(&(field1->coordinate_system),
			&(field2->coordinate_system)))))
		return false;
	if (ELEMENT_XI_VALUE == field1->value_type)
	{
		if (!field1->element_xi_host_mesh)
		{
			display_message(ERROR_MESSAGE,
				"FE_fields_match_fundamental.  Source element xi field %s does not have a host mesh", field1->name);
			return false;
		}
		// This will need to be improved once multiple meshes or meshes from different regions are allowed:
		if (field1->element_xi_host_mesh->getDimension() != field2->element_xi_host_mesh->getDimension())
			return false;
	}
	return true;
}

bool FE_fields_match_exact(struct FE_field *field1, struct FE_field *field2)
{
	if (field1 && field2)
	{
		// does not match until proven so
		if ((0 == strcmp(field1->name, field2->name)) &&
			(field1->fe_field_type == field2->fe_field_type) &&
			((INDEXED_FE_FIELD != field1->fe_field_type) ||
				(field1->indexer_field && field2->indexer_field &&
					(0 == strcmp(field1->indexer_field->name,
						field2->indexer_field->name)) &&
					FE_fields_match_fundamental(field1->indexer_field,
						field2->indexer_field) &&
					(field1->number_of_indexed_values ==
						field2->number_of_indexed_values))) &&
			(field1->cm_field_type == field2->cm_field_type) &&
			Coordinate_systems_match(&(field1->coordinate_system),
				&(field2->coordinate_system)) &&
			(field1->value_type == field2->value_type) &&
			(field1->number_of_components == field2->number_of_components) &&
			(field1->number_of_times == field2->number_of_times) &&
			(field1->time_value_type == field2->time_value_type))
		{
			// matches until disproven
			// check component names match
			for (int i = field1->number_of_components; i <= 0; --i)
			{
				char *component_name1 = get_automatic_component_name(field1->component_names, i);
				char *component_name2 = get_automatic_component_name(field2->component_names, i);
				bool matching = (component_name1) && (component_name2) &&
					(0 == strcmp(component_name1, component_name2));
				DEALLOCATE(component_name2);
				DEALLOCATE(component_name1);
				if (!matching)
					return false;
			}
			return true;
		}
	}
	else
		display_message(ERROR_MESSAGE, "FE_fields_match_exact.  Missing field(s)");
	return false;
}

/**
 * List iterator function which fetches a field with the same name as <field>
 * from <field_list>. Returns 1 (true) if there is either no such field in the
 * list or the two fields return true for FE_fields_match_fundamental(),
 * otherwise returns 0 (false).
 */
int FE_field_can_be_merged_into_list(struct FE_field *field, void *field_list_void)
{
	struct LIST(FE_field) *field_list = reinterpret_cast<struct LIST(FE_field) *>(field_list_void);
	if (field && field_list)
	{
		FE_field *other_field = FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(field->name, field_list);
		if ((!(other_field)) || FE_fields_match_fundamental(field, other_field))
			return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_can_be_merged_into_list.  Invalid argument(s)");
	}
	return 0;
}

int FE_field_has_multiple_times(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns true if any node_fields corresponding to <field> have time_sequences.
This will be improved when regionalised, so that hopefully the node field
list we will be looking at will not be global but will belong to the region.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_has_multiple_times);
	if (fe_field)
	{
		return_code = FE_region_FE_field_has_multiple_times(
			FE_field_get_FE_region(fe_field), fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_has_multiple_times.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_has_multiple_times */

bool FE_field_uses_non_linear_basis(struct FE_field *fe_field)
{
	if (fe_field && fe_field->info && fe_field->info->fe_region)
	{
		for (int d = 0; d < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++d)
			if ((fe_field->meshFieldData[d]) && fe_field->meshFieldData[d]->usesNonLinearBasis())
				return true;
	}
	return false;
}

int ensure_FE_field_is_in_list(struct FE_field *field, void *field_list_void)
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/
{
	int return_code;
	struct LIST(FE_field) *field_list;

	ENTER(ensure_FE_field_is_in_list);
	if (field&&(field_list=(struct LIST(FE_field) *)field_list_void))
	{
		if (!IS_OBJECT_IN_LIST(FE_field)(field,field_list))
		{
			return_code=ADD_OBJECT_TO_LIST(FE_field)(field,field_list);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"ensure_FE_field_is_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* ensure_FE_field_is_in_list */

struct FE_region *FE_field_get_FE_region(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/
{
	struct FE_region *fe_region;

	ENTER(FE_field_get_FE_region);
	if (fe_field && fe_field->info)
	{
		fe_region = fe_field->info->fe_region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_get_FE_region.  Invalid argument(s)");
		fe_region = (struct FE_region *)NULL;
	}
	LEAVE;

	return (fe_region);
} /* FE_field_get_FE_region */

int FE_field_add_wrapper(struct FE_field *field)
{
	if (field)
		return (++(field->number_of_wrappers));
	return 0;
}

int FE_field_remove_wrapper(struct FE_field *field)
{
	if (field)
		return (--(field->number_of_wrappers));
	return 0;
}

int FE_field_set_FE_field_info(struct FE_field *fe_field,
	struct FE_field_info *fe_field_info)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Changes the FE_field_info at <fe_field> to <fe_field_info>.
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_set_FE_field_info);
	if (fe_field && fe_field_info)
	{
		return_code =
			REACCESS(FE_field_info)(&(fe_field->info), fe_field_info);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_set_FE_field_info.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_set_FE_field_info */

int FE_field_get_access_count(struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 11 March 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/
{
	int access_count;

	ENTER(FE_field_get_access_count);
	if (fe_field)
	{
		access_count = fe_field->access_count;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_get_access_count.  Invalid argument(s)");
		access_count = 0;
	}
	LEAVE;

	return (access_count);
} /* FE_field_get_access_count */

char *get_FE_field_component_name(struct FE_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns the name of component <component_no> of <field>. If no name is stored
for the component, a string comprising the value component_no+1 is returned.
Up to calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char *component_name;

	ENTER(get_FE_field_component_name);
	if (field&&(0<=component_no)&&(component_no<field->number_of_components))
	{
		component_name=
			get_automatic_component_name(field->component_names,component_no);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_component_name.  Invalid argument(s)");
		component_name=(char *)NULL;
	}
	LEAVE;

	return (component_name);
} /* get_FE_field_component_name */

int set_FE_field_component_name(struct FE_field *field,int component_no,
	const char *component_name)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Sets the name of component <component_no> of <field>. Only sets name if it is
different from that already returned for field to preserve default names if can.
==============================================================================*/
{
	char *temp_component_name;
	int different_name,i,return_code;

	ENTER(set_FE_field_component_name);
	if (field&&(0<=component_no)&&(component_no<field->number_of_components)&&
		component_name)
	{
		if (NULL != (temp_component_name=get_FE_field_component_name(field,component_no)))
		{
			different_name=strcmp(temp_component_name,component_name);
			DEALLOCATE(temp_component_name);
		}
		else
		{
			different_name=1;
		}
		if (different_name)
		{
			if (ALLOCATE(temp_component_name,char,strlen(component_name)+1))
			{
				strcpy(temp_component_name,component_name);
				/* component_names array may be non-existent if default names used */
				if (field->component_names)
				{
					if (field->component_names[component_no])
					{
						DEALLOCATE(field->component_names[component_no]);
					}
				}
				else
				{
					if (ALLOCATE(field->component_names,char *,
						field->number_of_components))
					{
						/* clear the pointers to names */
						for (i=0;i<field->number_of_components;i++)
						{
							field->component_names[i]=(char *)NULL;
						}
					}
				}
				if (field->component_names)
				{
					field->component_names[component_no]=temp_component_name;
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_component_name.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_component_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_component_name */

struct Coordinate_system *get_FE_field_coordinate_system(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Returns a pointer to the coordinate system for the <field>.
???RC Should not be returning pointer to internal structure; change to
fill struct Coordinate_system at address passed to this function.
==============================================================================*/
{
	struct Coordinate_system *coordinate_system;

	ENTER(get_FE_field_coordinate_system);
	if (field)
	{
		coordinate_system = &field->coordinate_system;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_coordinate_system.  Invalid field");
		coordinate_system = (struct Coordinate_system *)NULL;
	}
	LEAVE;

	return (coordinate_system);
} /* get_FE_field_coordinate_system */

int set_FE_field_coordinate_system(struct FE_field *field,
	struct Coordinate_system *coordinate_system)
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Sets the coordinate system of the <field>.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_coordinate_system);
	if (field&&coordinate_system)
	{
		return_code=
			COPY(Coordinate_system)(&field->coordinate_system,coordinate_system);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_coordinate_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_coordinate_system */

void FE_field_clearMeshFieldData(struct FE_field *field, FE_mesh *mesh)
{
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
	{
		const int dim = mesh->getDimension() - 1;
		if (field->meshFieldData[dim])
		{
			delete field->meshFieldData[dim];
			field->meshFieldData[dim] = 0;
			field->info->fe_region->FE_field_change(field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
		}
	}
}

FE_mesh_field_data *FE_field_createMeshFieldData(struct FE_field *field,
	FE_mesh *mesh)
{
	// GRC check field type general, real/int only?
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
	{
		const int dim = mesh->getDimension() - 1;
		if (!field->meshFieldData[dim])
			field->meshFieldData[dim] = FE_mesh_field_data::create(field, mesh);
		return field->meshFieldData[dim];
	}
	return 0;
}

FE_mesh_field_data *FE_field_getMeshFieldData(struct FE_field *field,
	const FE_mesh *mesh)
{
	if (field && mesh && (mesh->get_FE_region() == field->info->fe_region))
		return field->meshFieldData[mesh->getDimension() - 1];
	display_message(ERROR_MESSAGE, "FE_field_getMeshFieldData.  Invalid argument(s)");
	return 0;
}

int get_FE_field_number_of_components(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of components for the <field>.
==============================================================================*/
{
	int number_of_components;

	ENTER(get_FE_field_number_of_components);
	if (field)
	{
		number_of_components=field->number_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_components.  Missing field");
		number_of_components=0;
	}
	LEAVE;

	return (number_of_components);
} /* get_FE_field_number_of_components */

int set_FE_field_number_of_components(struct FE_field *field,
	int number_of_components)
/*******************************************************************************
LAST MODIFIED : 16 January 2002

DESCRIPTION :
Sets the number of components in the <field>. Automatically assumes names for
any new components. Clears/reallocates the values_storage for FE_field_types
that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but only if number
of components changes. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/
{
	char **component_names;
	int i,number_of_values,return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_number_of_components);
	if (field && (0<number_of_components) && ((1 == number_of_components) ||
		((field->value_type != ELEMENT_XI_VALUE) && (field->value_type != STRING_VALUE) &&
			(field->value_type != URL_VALUE))))
	{
		return_code=1;
		if (number_of_components != field->number_of_components)
		{
			/* 1. make dynamic allocations for number_of_components-specific data */
			component_names=(char **)NULL;
			if (field->component_names)
			{
				if (ALLOCATE(component_names,char *,number_of_components))
				{
					/* copy the old names, clear any new ones */
					for (i=0;i<number_of_components;i++)
					{
						if (i<field->number_of_components)
						{
							component_names[i]=field->component_names[i];
						}
						else
						{
							component_names[i]=(char *)NULL;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			values_storage=(Value_storage *)NULL;
			number_of_values=0;
			switch (field->fe_field_type)
			{
				case CONSTANT_FE_FIELD:
				{
					number_of_values=number_of_components;
				} break;
				case GENERAL_FE_FIELD:
				{
					number_of_values=0;
				} break;
				case INDEXED_FE_FIELD:
				{
					number_of_values=field->number_of_indexed_values*number_of_components;
				} break;
				default:
				{
					return_code=0;
				} break;
			}
			if (number_of_values != field->number_of_values)
			{
				if (!(values_storage=make_value_storage_array(field->value_type,
					(struct FE_time_sequence *)NULL,number_of_values)))
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				/* 2. free current number_of_components-specific data */
				if (field->component_names)
				{
					/* free component_names no longer used */
					for (i = number_of_components; i < field->number_of_components; i++)
					{
						if (field->component_names[i])
						{
							DEALLOCATE(field->component_names[i]);
						}
					}
					DEALLOCATE(field->component_names);
				}
				if (field->values_storage)
				{
					free_value_storage_array(field->values_storage,field->value_type,
						(struct FE_time_sequence *)NULL,field->number_of_values);
					DEALLOCATE(field->values_storage);
				}
				/* 3. establish the new number_of_components and associated data */
				field->number_of_components=number_of_components;
				field->component_names=component_names;
				field->values_storage=values_storage;
				field->number_of_values=number_of_values;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_number_of_components.  Not enough memory");
				DEALLOCATE(component_names);
				DEALLOCATE(values_storage);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_number_of_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_number_of_components */

int get_FE_field_number_of_values(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Returns the number of global values for the <field>.
==============================================================================*/
{
	int number_of_values;

	ENTER(get_FE_field_number_of_values);
	if (field)
	{
		number_of_values=field->number_of_values;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_values.  Invalid field");
		number_of_values=0;
	}
	LEAVE;

	return (number_of_values);
} /* get_FE_field_number_of_values */

int get_FE_field_number_of_times(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the number of global times for the <field>.
==============================================================================*/
{
	int number_of_times;

	ENTER(get_FE_field_number_of_times);
	if (field)
	{
		number_of_times=field->number_of_times;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_number_of_times.  Invalid field");
		number_of_times=0;
	}
	LEAVE;

	return (number_of_times);
} /* get_FE_field_number_of_times */

int set_FE_field_number_of_times(struct FE_field *field,
	int number_of_times)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the number of times stored with the <field>
REALLOCATES the requires memory in field->value_storage, based upon the
field->time_value_type.

For non-array types, the contents of field->times_storage is:
   | data type (eg FE_value) | x number_of_times

For array types, the contents of field->times is:
   ( | int (number of array values) | pointer to array (eg double *) | x number_of_times )

Sets data in this memory to 0, pointers to NULL.

MUST have called set_FE_field_time_value_type before calling this function.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int i,j, return_code,size;
	Value_storage *new_value, *times;

	ENTER(set_FE_field_number_of_times);
	if (field&&(0<=number_of_times))
	{
		return_code=1;
		if (number_of_times != 0)
		{
			field->number_of_times=number_of_times;
			size = get_Value_storage_size(field->time_value_type,
				(struct FE_time_sequence *)NULL);

			if (REALLOCATE(times,field->times,Value_storage,
				size*number_of_times))
			{
				field->times = times;
				for (i=0;i<number_of_times;i++)
				{
					switch (field->time_value_type)
					{
						/* set values to zero*/
						case DOUBLE_VALUE:
						{
							*((double *)times) = 0;
						}	break;
						case ELEMENT_XI_VALUE:
						{
							new_value = times;
							*((cmzn_element **)new_value) = 0;
							new_value += sizeof(cmzn_element *);
							for (j = 0 ; j < MAXIMUM_ELEMENT_XI_DIMENSIONS ; j++)
							{
								*((FE_value *)new_value) = 0;
								new_value+=sizeof(FE_value);
							}
						}	break;
						case FE_VALUE_VALUE:
						{
							*((FE_value *)times) = 0;
						}	break;
						case FLT_VALUE:
						{
							*((float *)times) = 0;
						} break;
						case SHORT_VALUE:
						{
							display_message(ERROR_MESSAGE," set_FE_field_number_of_times."
								"SHORT_VALUE. Code not written yet. Beware alignment problems ");
							return_code =0;
						} break;
						case INT_VALUE:
						{
							*((int *)times) = 0;
						}	break;
						case UNSIGNED_VALUE:
						{
							*((unsigned *)times) = 0;
						}	break;
						/* set number of array values to 0, array pointers to NULL*/
						case DOUBLE_ARRAY_VALUE:
						{
							double **array_address;
							/* copy the number of array values (0!) to times*/
							*((int *)times) = 0;
							/* copy the pointer to the array values (currently NULL), to times*/
							array_address = (double **)(times+sizeof(int));
							*array_address = (double *)NULL;
						} break;
						case FE_VALUE_ARRAY_VALUE:
						{
							FE_value **array_address;
							*((int *)times) = 0;
							array_address = (FE_value **)(times+sizeof(int));
							*array_address = (FE_value *)NULL;
						} break;
						case FLT_ARRAY_VALUE:
						{
							float **array_address;
							*((int *)times) = 0;
							array_address = (float **)(times+sizeof(int));
							*array_address = (float *)NULL;
						} break;
						case SHORT_ARRAY_VALUE:
						{
							short **array_address;
							*((int *)times) = 0;
							array_address = (short **)(times+sizeof(int));
							*array_address = (short *)NULL;
						} break;
						case INT_ARRAY_VALUE:
						{
							int **array_address;
							*((int *)times) = 0;
							array_address = (int **)(times+sizeof(int));
							*array_address = (int *)NULL;
						} break;
						case UNSIGNED_ARRAY_VALUE:
						{
							unsigned **array_address;
							*((int *)times) = 0;
							array_address = (unsigned **)(times+sizeof(int));
							*array_address = (unsigned *)NULL;
						} break;
						case STRING_VALUE:
						{
							char **str_address;
							str_address = (char **)(times);
							*str_address = (char *)NULL;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE," set_FE_field_number_of_times."
								"  Unsupported value_type");
							return_code =0;
						} break;
					}	/*	switch (field->time_value_type) */
					times += size;
				}/* (i=0;i<number_of_times;i++) */
			}/* if (REALLOCATE */
			else
			{
				display_message(ERROR_MESSAGE,"set_FE_field_number_of_times."
					" Not enough memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_number_of_times.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_number_of_times */

enum CM_field_type get_FE_field_CM_field_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Returns the CM_field_type for the <field>.
==============================================================================*/
{
	enum CM_field_type type;

	ENTER(get_FE_field_CM_field_type);
	if (field)
	{
		type=field->cm_field_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_CM_field_type.  Invalid field");
		type=CM_GENERAL_FIELD;
	}
	LEAVE;

	return (type);
} /* get_FE_field_CM_field_type */

int set_FE_field_CM_field_type(struct FE_field *field,
	enum CM_field_type cm_field_type)
/*******************************************************************************
LAST MODIFIED : 30 August 2001

DESCRIPTION :
Sets the CM_field_type of the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_CM_field_type);
	if (field)
	{
		field->cm_field_type=cm_field_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_CM_field_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_CM_field_type */

enum FE_field_type get_FE_field_FE_field_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the FE_field_type for the <field>.
==============================================================================*/
{
	enum FE_field_type fe_field_type;

	ENTER(get_FE_field_FE_field_type);
	if (field)
	{
		fe_field_type=field->fe_field_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_FE_field_type.  Invalid field");
		fe_field_type=UNKNOWN_FE_FIELD;
	}
	LEAVE;

	return (fe_field_type);
} /* get_FE_field_FE_field_type */

int set_FE_field_type_constant(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type CONSTANT_FE_FIELD.
Allocates and clears the values_storage of the field to fit
field->number_of_components of the current value_type.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int number_of_values, return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_type_constant);
	return_code = 0;
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_values = field->number_of_components;
		if (NULL != (values_storage = make_value_storage_array(field->value_type,
			(struct FE_time_sequence *)NULL, number_of_values)))
		{
			/* 2. free current type-specific data */
			if (field->values_storage)
			{
				free_value_storage_array(field->values_storage, field->value_type,
					(struct FE_time_sequence *)NULL, field->number_of_values);
				DEALLOCATE(field->values_storage);
			}
			REACCESS(FE_field)(&(field->indexer_field), NULL);
			field->number_of_indexed_values = 0;
			/* 3. establish the new type */
			field->fe_field_type = CONSTANT_FE_FIELD;
			field->values_storage = values_storage;
			field->number_of_values = number_of_values;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_type_constant.  Could not allocate values_storage");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_constant.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_constant */

int set_FE_field_type_general(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type GENERAL_FE_FIELD.
Frees any values_storage currently in use by the field.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_type_general);
	return_code = 0;
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		if (field->values_storage)
		{
			free_value_storage_array(field->values_storage, field->value_type,
				(struct FE_time_sequence *)NULL, field->number_of_values);
			DEALLOCATE(field->values_storage);
		}
		REACCESS(FE_field)(&(field->indexer_field), NULL);
		field->number_of_indexed_values = 0;
		/* 3. establish the new type */
		field->fe_field_type = GENERAL_FE_FIELD;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_general.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_general */

int get_FE_field_type_indexed(struct FE_field *field,
	struct FE_field **indexer_field, int *number_of_indexed_values)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
If the field is of type INDEXED_FE_FIELD, the indexer_field and
number_of_indexed_values it uses are returned - otherwise an error is reported.
Use function get_FE_field_FE_field_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_type_indexed);
	if (field&&(INDEXED_FE_FIELD==field->fe_field_type)&&indexer_field&&
		number_of_indexed_values)
	{
		*indexer_field = field->indexer_field;
		*number_of_indexed_values = field->number_of_indexed_values;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_type_indexed.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_type_indexed */

int set_FE_field_type_indexed(struct FE_field *field,
	struct FE_field *indexer_field, int number_of_indexed_values)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Converts the <field> to type INDEXED_FE_FIELD, indexed by the given
<indexer_field> and with the given <number_of_indexed_values>. The indexer_field
must return a single integer value to be valid.
Allocates and clears the values_storage of the field to fit
field->number_of_components x number_of_indexed_values of the current
value_type. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	int number_of_values, return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_type_indexed);
	return_code = 0;
	if (field&&indexer_field&&(0<number_of_indexed_values)&&
		(1==get_FE_field_number_of_components(indexer_field))&&
		(INT_VALUE==get_FE_field_value_type(indexer_field))&&
		/* and to avoid possible endless loops... */
		(indexer_field != field) &&
		(INDEXED_FE_FIELD != get_FE_field_FE_field_type(indexer_field)))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_values = field->number_of_components*number_of_indexed_values;
		if (NULL != (values_storage = make_value_storage_array(field->value_type,
			(struct FE_time_sequence *)NULL, number_of_values)))
		{
			/* 2. free current type-specific data */
			if (field->values_storage)
			{
				free_value_storage_array(field->values_storage, field->value_type,
					(struct FE_time_sequence *)NULL, field->number_of_values);
				DEALLOCATE(field->values_storage);
			}
			/* 3. establish the new type */
			field->fe_field_type = INDEXED_FE_FIELD;
			REACCESS(FE_field)(&(field->indexer_field), indexer_field);
			field->number_of_indexed_values = number_of_indexed_values;
			field->values_storage = values_storage;
			field->number_of_values = number_of_values;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_type_indexed.  Could not allocate values_storage");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_type_indexed.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_type_indexed */

int FE_field_set_indexer_field(struct FE_field *fe_field,
	struct FE_field *indexer_fe_field)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
If <fe_field> is already indexed, substitutes <indexer_fe_field>.
Does not change any of the values currently stored in <fe_field>
Used to merge indexed fields into different FE_regions; should not be used for
any other purpose.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_set_indexer_field);
	return_code = 0;
	if (fe_field && (INDEXED_FE_FIELD == fe_field->fe_field_type) &&
		indexer_fe_field &&
		(1 == get_FE_field_number_of_components(indexer_fe_field)) &&
		(INT_VALUE == get_FE_field_value_type(indexer_fe_field)) &&
		/* and to avoid possible endless loops... */
		(INDEXED_FE_FIELD != get_FE_field_FE_field_type(indexer_fe_field)))
	{
		REACCESS(FE_field)(&(fe_field->indexer_field), indexer_fe_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_set_indexer_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_field_set_indexer_field */

int FE_field_log_FE_field_change(struct FE_field *fe_field,
	void *fe_field_change_log_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
Logs the field in <fe_field> as RELATED_OBJECT_CHANGED in the
struct CHANGE_LOG(FE_field) pointed to by <fe_field_change_log_void>.
???RC Later may wish to allow more than just RELATED_OBJECT_CHANGED, or have
separate functions for each type.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_log_FE_field_change);
	/*???RC try to make this as efficient as possible so no argument checking */
	return_code = CHANGE_LOG_OBJECT_CHANGE(FE_field)(
		(struct CHANGE_LOG(FE_field) *)fe_field_change_log_void,
		fe_field, CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field));
	LEAVE;

	return (return_code);
} /* FE_field_log_FE_field_change */

enum Value_type get_FE_field_value_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the value_type of the <field>.
==============================================================================*/
{
	enum Value_type value_type;

	ENTER(get_FE_field_value_type);
	if (field)
	{
		value_type = field->value_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_value_type.  Invalid field");
		value_type = UNKNOWN_VALUE;
	}
	LEAVE;

	return (value_type);
} /* get_FE_field_value_type */

int set_FE_field_value_type(struct FE_field *field, enum Value_type value_type)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the value_type of the <field>. Clears/reallocates the values_storage for
FE_field_types that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but
only if the value_type changes. If function fails the field is left exactly as
it was. Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/
{
	int number_of_values, return_code;
	Value_storage *values_storage;

	ENTER(set_FE_field_value_type);
	if (field && ((1 >= field->number_of_components) ||
		((value_type != ELEMENT_XI_VALUE) && (value_type != STRING_VALUE) &&
		(value_type != URL_VALUE))))
	{
		return_code = 1;
		if (value_type != field->value_type)
		{
			/* 1. make dynamic allocations for value_type-specific data */
			values_storage = (Value_storage *)NULL;
			number_of_values = field->number_of_values;
			if (0!=number_of_values)
			{
				if (!(values_storage = make_value_storage_array(value_type,
					(struct FE_time_sequence *)NULL, number_of_values)))
				{
					return_code = 0;
				}
			}
			if (return_code)
			{
				/* 2. free current value_type-specific data */
				if (field->values_storage)
				{
					free_value_storage_array(field->values_storage, field->value_type,
						(struct FE_time_sequence *)NULL, field->number_of_values);
					DEALLOCATE(field->values_storage);
				}
				/* 3. establish the new value_type and associated data */
				field->value_type = value_type;
				field->values_storage = values_storage;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_value_type.  Not enough memory");
				DEALLOCATE(values_storage);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_value_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}/* set_FE_field_value_type */

const FE_mesh *FE_field_get_element_xi_host_mesh(struct FE_field *field)
{
	if (field)
		return field->element_xi_host_mesh;
	return 0;
}

int FE_field_set_element_xi_host_mesh(struct FE_field *field,
	const FE_mesh *hostMesh)
{
	if (!((field) && (field->value_type == ELEMENT_XI_VALUE) && (hostMesh)
		&& (hostMesh->get_FE_region() == field->info->fe_region))) // Current limitation of same region can be removed later
	{
		display_message(ERROR_MESSAGE, "FE_field_set_element_xi_host_mesh.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	if (field->element_xi_host_mesh)
	{
		display_message(ERROR_MESSAGE, "FE_field_set_element_xi_host_mesh.  Host mesh is already set");
		return CMZN_ERROR_ALREADY_EXISTS;
	}
	field->element_xi_host_mesh = hostMesh->access();
	return CMZN_OK;
}

enum Value_type get_FE_field_time_value_type(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the time_value_type of the <field>.
==============================================================================*/
{
	enum Value_type time_value_type;

	ENTER(get_FE_field_time_value_type);
	if (field)
	{
		time_value_type = field->time_value_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_time_value_type.  Invalid field");
		time_value_type = UNKNOWN_VALUE;
	}
	LEAVE;

	return (time_value_type);
} /*get_FE_field_time_value_type */

int set_FE_field_time_value_type(struct FE_field *field, enum Value_type time_value_type)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the time_value_type of the <field>.
Should only call this function for unmanaged fields.
=========================================================================*/
{
	int return_code;

	ENTER(set_FE_field_time_value_type);
	if (field)
	{
		field->time_value_type = time_value_type;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_field_time_value_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}/*set_FE_field_time_value_type */

int get_FE_field_max_array_size(struct FE_field *field, int *max_number_of_array_values,
	enum Value_type *value_type)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Given the field, search vaules_storage  for the largest array, and return it in
max_number_of_array_values. Return the field value_type.
====================================================================================*/
{
	int return_code, size, i, number_of_array_values;

	Value_storage *values_storage;

	ENTER(get_FE_field_max_array_size);
	if (field)
	{
		if (field->number_of_values)
		{
			return_code = 1;
			*value_type = field->value_type;
			switch (field->value_type)
			{
			case DOUBLE_ARRAY_VALUE:
			case FE_VALUE_ARRAY_VALUE:
			case FLT_ARRAY_VALUE:
			case SHORT_ARRAY_VALUE:
			case INT_ARRAY_VALUE:
			case UNSIGNED_ARRAY_VALUE:
			case STRING_VALUE:
			{
				*max_number_of_array_values = 0;
				size = get_Value_storage_size(*value_type,
					(struct FE_time_sequence *)NULL);
				values_storage = field->values_storage;
				for (i = 0; i<field->number_of_values; i++)
				{
					if (field->value_type == STRING_VALUE)
					{
						char *the_string, **str_address;
						/* get the string's length*/
						str_address = (char **)(values_storage);
						the_string = *str_address;
						number_of_array_values = static_cast<int>(strlen(the_string)) + 1;/* +1 for null termination*/
						if (number_of_array_values > *max_number_of_array_values)
						{
							*max_number_of_array_values = number_of_array_values;
						}
					}
					else
					{
						/* get the number of array values  for the specified array in vaules_storage */
						number_of_array_values = *((int *)values_storage);
						if (number_of_array_values > *max_number_of_array_values)
						{
							*max_number_of_array_values = number_of_array_values;
						}
					}
					values_storage += (i*size);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, " get_FE_field_max_array_size. Not an array type)");
				number_of_array_values = 0;
				return_code = 0;
			} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, " get_FE_field_max_array_size. No values at field");

			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, " get_FE_field_max_array_size. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* get_FE_field_max_array_size */

int get_FE_field_array_attributes(struct FE_field *field, int value_number,
	int *number_of_array_values, enum Value_type *value_type)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Get the value_type and the number of array values for the array in field->values_storage
specified by value_number.
Give an error if field->values_storage isn't storing array types.
====================================================================================*/
{

	int return_code, size;

	Value_storage *values_storage;

	ENTER(get_FE_field_array_attributes);
	if (field&&(0<=value_number)&&(value_number<=field->number_of_values))
	{
		if (field->number_of_values)
		{
			return_code = 1;
			*value_type = field->value_type;
			switch (field->value_type)
			{
			case DOUBLE_ARRAY_VALUE:
			case FE_VALUE_ARRAY_VALUE:
			case FLT_ARRAY_VALUE:
			case SHORT_ARRAY_VALUE:
			case INT_ARRAY_VALUE:
			case UNSIGNED_ARRAY_VALUE:
			{
				/* get the correct offset*/
				size = get_Value_storage_size(*value_type,
					(struct FE_time_sequence *)NULL);
				values_storage = field->values_storage+(value_number*size);
				/* get the number of array values  for the specified array in vaules_storage */
				*number_of_array_values = *((int *)values_storage);
			} break;
			case STRING_VALUE:
			{
				char *the_string, **str_address;
				/* get the correct offset*/
				size = get_Value_storage_size(*value_type,
					(struct FE_time_sequence *)NULL);
				values_storage = field->values_storage+(value_number*size);
				/* get the string*/
				str_address = (char **)(values_storage);
				the_string = *str_address;
				*number_of_array_values = static_cast<int>(strlen(the_string)) + 1;/* +1 for null termination*/
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "get_FE_field_array_attributes. Not an array type)");
				number_of_array_values = 0;
				return_code = 0;
			} break;
			}
		}
		else
		{
			return_code = 0;
			display_message(ERROR_MESSAGE, "get_FE_field_array_attributes. No values at the field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "get_FE_field_array_attributes. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* get_FE_field_array_attributes */

int get_FE_field_string_value(struct FE_field *field, int value_number,
	char **string)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Returns a copy of the string stored at <value_number> in the <field>.
Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the field.
==============================================================================*/
{
	int return_code, size;
	char *the_string, **string_address;

	ENTER(get_FE_field_string_value);
	return_code = 0;
	if (field&&(0<=value_number)&&(value_number<field->number_of_values)&&string)
	{
		/* get the pointer to the stored string */
		size = get_Value_storage_size(STRING_VALUE, (struct FE_time_sequence *)NULL);
		string_address = (char **)(field->values_storage+value_number*size);
		if (NULL != (the_string = *string_address))
		{
			if (ALLOCATE(*string, char, strlen(the_string)+1))
			{
				strcpy(*string, the_string);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_field_string_value.  Not enough memory");
			}
		}
		else
		{
			/* no string, so successfully return NULL */
			*string = (char *)NULL;
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_string_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_string_value */

int set_FE_field_string_value(struct FE_field *field, int value_number,
	char *string)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Copies and sets the <string> stored at <value_number> in the <field>.
<string> may be NULL.
==============================================================================*/
{
	int return_code, size;
	char *the_string, **string_address;

	ENTER(set_FE_field_string_value);
	return_code = 0;
	if (field&&(0<=value_number)&&(value_number<field->number_of_values))
	{
		/* get the pointer to the stored string */
		size = get_Value_storage_size(STRING_VALUE, (struct FE_time_sequence *)NULL);
		string_address = (char **)(field->values_storage+value_number*size);
		if (string)
		{
			/* reallocate the string currently there */
			if (REALLOCATE(the_string, *string_address, char, strlen(string)+1))
			{
				strcpy(the_string, string);
				*string_address = the_string;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"set_FE_field_string_value.  Not enough memory");
			}
		}
		else
		{
			/* NULL string; free the existing string */
			if (*string_address)
			{
				DEALLOCATE(*string_address);
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_string_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_string_value */

int get_FE_field_element_xi_value(struct FE_field *field, int number,
	cmzn_element **element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Gets the specified global value for the <field>.
==============================================================================*/
{
	int i, number_of_xi_dimensions, return_code;
	Value_storage *values_storage;

	ENTER(get_FE_field_element_xi_value);
	if (field&&(0<=number)&&(number<field->number_of_values)
		&&(field->value_type==ELEMENT_XI_VALUE))
	{
		if (field->number_of_values)
		{
			return_code = 1;

			/* get the correct offset*/
			values_storage = field->values_storage + (number*(sizeof(cmzn_element *) +
				MAXIMUM_ELEMENT_XI_DIMENSIONS * sizeof(FE_value)));

			/* copy the element and xi out */
			*element = *((cmzn_element **)values_storage);
			values_storage += sizeof(cmzn_element *);
			number_of_xi_dimensions = (*element) ? (*element)->getDimension() : 0;
			if (number_of_xi_dimensions <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
			{
				/* Extract the xi values */
				for (i = 0; i < number_of_xi_dimensions; i++)
				{
					xi[i] = *((FE_value *)values_storage);
					values_storage += sizeof(FE_value);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"get_FE_field_element_xi_value.  Number of xi dimensions of element exceeds maximum");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_field_element_xi_value. no values at field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_element_xi_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}/* get_FE_field_element_xi_value */

int set_FE_field_element_xi_value(struct FE_field *field, int number,
	cmzn_element *element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 14 September 1999

DESCRIPTION :
Sets the specified global value for the <field>, to the passed Element and xi.
The <field> must be of the correct FE_field_type to have such values and
<number> must be within the range valid for that type.
==============================================================================*/
{
	int i, number_of_xi_dimensions, return_code;
	Value_storage  *values_storage;

	ENTER(set_FE_field_element_xi_value);
	if (field&&(0<=number)&&(number<=field->number_of_values)
		&&(field->value_type==ELEMENT_XI_VALUE) && element && xi)
	{
		return_code = 1;
		number_of_xi_dimensions = element->getDimension();
		if (number_of_xi_dimensions <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
		{

			/* get the correct offset*/
			values_storage = field->values_storage + (number*(sizeof(cmzn_element *) +
				MAXIMUM_ELEMENT_XI_DIMENSIONS * sizeof(FE_value)));

			/* copy the element in ensuring correct accessing */
			REACCESS(cmzn_element)(((cmzn_element **)values_storage), element);
			values_storage += sizeof(struct Element *);
			/* Write in the xi values */
			for (i = 0; i<MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
			{
				if (i<number_of_xi_dimensions)
				{
					*((FE_value *)values_storage) = xi[i];
				}
				else
				{
					/* set spare xi values to 0 */
					*((FE_value *)values_storage) = 0.0;
				}
				values_storage += sizeof(FE_value);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_FE_field_element_xi_value.  Number of xi dimensions of element exceeds maximum");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, " set_FE_field_element_xi_value. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* set_FE_field_element_xi_value */

int get_FE_field_FE_value_value(struct FE_field *field, int number,
	FE_value *value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global FE_value <value> from <field>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_FE_value_value);
	if (field&&(FE_VALUE_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values)&&value)
	{
		*value = *((FE_value *)(field->values_storage+(number*sizeof(FE_value))));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_FE_value_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_FE_value_value */

int set_FE_field_FE_value_value(struct FE_field *field, int number,
	FE_value value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global FE_value <value> in <field>.
The <field> must be of type FE_VALUE_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_FE_value_value);
	if (field&&(FE_VALUE_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values))
	{
		*((FE_value *)(field->values_storage+(number*sizeof(FE_value)))) = value;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_FE_value_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_FE_value_value */

int get_FE_field_int_value(struct FE_field *field, int number, int *value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global int <value> from <field>.
==============================================================================*/
{
	int return_code;

	ENTER(get_FE_field_int_value);
	if (field&&(INT_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values)&&value)
	{
		*value = *((int *)(field->values_storage+(number*sizeof(int))));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_int_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_FE_field_int_value */

int set_FE_field_int_value(struct FE_field *field, int number, int value)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global int <value> in <field>.
The <field> must be of type INT_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/
{
	int return_code;

	ENTER(set_FE_field_int_value);
	if (field&&(INT_VALUE==field->value_type)&&field->values_storage&&
		(0<=number)&&(number<=field->number_of_values))
	{
		*((int *)(field->values_storage+(number*sizeof(int)))) = value;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_int_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_int_value */

int get_FE_field_time_FE_value(struct FE_field *field, int number, FE_value *value)
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Gets the specified global time value for the <field>.
==============================================================================*/
{
	int return_code;
	Value_storage *times;

	ENTER(get_FE_field_time_FE_value);
	if (field&&(0<=number)&&(number<field->number_of_times))
	{
		if (field->number_of_times)
		{
			return_code = 1;
			/* get the correct offset*/
			times = field->times+(number*sizeof(FE_value));
			/* copy the value in*/
			*value = *((FE_value *)times);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"get_FE_field_time_FE_value.  no times at field");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_time_FE_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}/*get_FE_field_time_FE_value */

int set_FE_field_time_FE_value(struct FE_field *field, int number, FE_value value)
/*******************************************************************************
LAST MODIFIED : l0 June 1999

DESCRIPTION :
Sets the specified global time value for the <field>, to the passed FE_value
The field value MUST have been previously allocated with set_FE_field_number_of_times
==============================================================================*/
{

	int return_code;
	FE_value *times;

	ENTER(set_FE_field_time_FE_value);
	if (field&&(0<=number)&&(number<=field->number_of_times))
	{
		return_code = 1;
		if (field->time_value_type!=FE_VALUE_VALUE)
		{
			display_message(ERROR_MESSAGE, " set_FE_field_time_FE_value. "
				" value type doesn't match");
			return_code = 0;
		}
		/* get the correct offset*/
		times = (FE_value *)(field->times+(number*sizeof(FE_value)));
		/* copy the value in*/
		*times = value;
	}
	else
	{
		display_message(ERROR_MESSAGE, " set_FE_field_time_FE_value. Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* set_FE_field_time_FE_value */

const char *get_FE_field_name(struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns a pointer to the name for the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/
{
	const char *name;

	ENTER(get_FE_field_name);
	if (field)
	{
		name = field->name;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_FE_field_name.  Invalid argument(s)");
		name = (const char *)NULL;
	}
	LEAVE;

	return (name);
}/*get_FE_field_name */

int set_FE_field_name(struct FE_field *field, const char *name)
{
	char *temp;
	int return_code;

	ENTER(set_FE_field_name);
	if (field&&name)
	{
		temp = duplicate_string(name);
		if (temp)
		{
			DEALLOCATE(field->name);
			field->name = temp;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "set_FE_field_name.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_field_name.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_name */

int FE_field_is_1_component_integer(struct FE_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if <field> has exactly 1 component and a
value type of integer.
This type of field is used for storing eg. grid_point_number.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_1_component_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(INT_VALUE==field->value_type)&&
			(1==field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_1_component_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_1_component_integer */

int FE_field_is_coordinate_field(struct FE_field *field,void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	if (field && field->isTypeCoordinate())
		return 1;
	return 0;
}

int FE_field_is_anatomical_fibre_field(struct FE_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 2 September 2001

DESCRIPTION :
Conditional function returning true if the <field> is a anatomical field
(defined by having a CM_field_type of anatomical), has a Value_type of
FE_VALUE_VALUE, has from 1 to 3 components, and has a FIBRE coordinate system.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_anatomical_fibre_field);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(CM_ANATOMICAL_FIELD==field->cm_field_type)&&
			(FE_VALUE_VALUE==field->value_type)&&
			(1<=field->number_of_components)&&
			(3>=field->number_of_components)&&
			(FIBRE==field->coordinate_system.type);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_is_anatomical_fibre_field.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_anatomical_fibre_field */

int FE_field_is_embedded(struct FE_field *field, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 5 June 2003

DESCRIPTION :
Returns true if the values returned by <field> are a location in an FE_region,
either an element_xi value, or eventually a node.
==============================================================================*/
{
	int return_code;

	ENTER(FE_field_is_embedded);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code = (ELEMENT_XI_VALUE == field->value_type);
	}
	else
	{
		display_message(ERROR_MESSAGE, "FE_field_is_embedded.  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_is_embedded */
