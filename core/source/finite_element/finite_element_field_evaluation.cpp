/**
 * FILE : finite_element_field_evaluation.cpp
 *
 * Class for caching values and evaluating fields over elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <math.h>
#include <limits>

#include "opencmiss/zinc/result.h"
#include "finite_element/finite_element_field_evaluation.hpp"
#include "finite_element/finite_element_field_private.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_value_storage.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

// function from finite_element.cpp
int global_to_element_map_values(FE_field *field, int componentNumber,
	const FE_element_field_template *eft, cmzn_element *element, FE_value time,
	const FE_nodeset *nodeset, FE_value*& elementValues);

/*
Global functions
----------------
*/

FE_element_field_evaluation::FE_element_field_evaluation() :
	field(0),
	element(0),
	field_element(0),
	time_dependent(0),
	time(0.0),
	component_number_in_xi(0),
	destroy_standard_basis_arguments(false),
	number_of_components(0),
	component_number_of_values(0),
	component_grid_values_storage(0),
	component_base_grid_offset(0),
	component_grid_offset_in_xi(0),
	element_value_offsets(0),
	component_values(0),
	component_standard_basis_functions(0),
	component_standard_basis_function_arguments(0),
	access_count(1)
{
	this->last_grid_xi[0] = std::numeric_limits<FE_value>::quiet_NaN();  // to force initial grid xi to be invalid
}

int FE_element_field_evaluation::deaccess(FE_element_field_evaluation* &element_field_evaluation)
{
	if (!element_field_evaluation)
		return CMZN_RESULT_ERROR_ARGUMENT;
	--(element_field_evaluation->access_count);
	if (element_field_evaluation->access_count <= 0)
		delete element_field_evaluation;
	element_field_evaluation = 0;
	return CMZN_RESULT_OK;
}

void FE_element_field_evaluation::clear()
{
	if (this->component_number_in_xi)
	{
		if (this->field)
		{
			for (int i = 0; i < this->number_of_components; i++)
			{
				if (this->component_number_in_xi[i])
				{
					DEALLOCATE(this->component_number_in_xi[i]);
				}
			}
		}
		DEALLOCATE(this->component_number_in_xi);
	}
	DEACCESS(FE_field)(&(this->field));
	DEACCESS(FE_element)(&(this->element));
	DEACCESS(FE_element)(&(this->field_element));
	if (this->component_number_of_values)
	{
		DEALLOCATE(this->component_number_of_values);
	}
	if (this->component_grid_values_storage)
	{
		DEALLOCATE(this->component_grid_values_storage);
	}
	if (this->component_base_grid_offset)
	{
		DEALLOCATE(this->component_base_grid_offset);
	}
	if (this->component_grid_offset_in_xi)
	{
		for (int i = 0; i < this->number_of_components; i++)
		{
			DEALLOCATE(this->component_grid_offset_in_xi[i]);
		}
		DEALLOCATE(this->component_grid_offset_in_xi);
	}
	if (this->element_value_offsets)
	{
		DEALLOCATE(this->element_value_offsets);
	}
	if (this->component_values)
	{
		FE_value **tmp_values = this->component_values;
		for (int i = this->number_of_components; i>0; i--)
		{
			DEALLOCATE(*tmp_values);
			tmp_values++;
		}
		DEALLOCATE(this->component_values);
	}
	if (this->component_standard_basis_function_arguments)
	{
		if (this->destroy_standard_basis_arguments)
		{
			int **tmp_arguments = this->component_standard_basis_function_arguments;
			for (int i = this->number_of_components; i > 0; --i)
			{
				if (*tmp_arguments && ((1 == i) || (*tmp_arguments != tmp_arguments[1])))
				{
					DEALLOCATE(*tmp_arguments);
				}
				++tmp_arguments;
			}
		}
		DEALLOCATE(this->
			component_standard_basis_function_arguments);
	}
	if (this->component_standard_basis_functions)
	{
		DEALLOCATE(this->component_standard_basis_functions);
	}
}

int FE_element_field_evaluation::calculate_values(FE_field *field,
	cmzn_element *element, FE_value time, cmzn_element *top_level_element)
{
	FE_value *blending_matrix, *inherited_value, *inherited_values,
		*transformation, *value, **values_address;
	int cn, **component_number_in_xi, *component_base_grid_offset,
		*element_value_offsets, i, j, number_of_inherited_values,
		*number_of_values_address, row_size,
		**standard_basis_arguments_address;
	Standard_basis_function **standard_basis_address;

	if (!((element) && (field)))
	{
		display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  Invalid argument(s)");
		return 0;
	}
	FE_value coordinate_transformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_element *fieldElement = field->getOrInheritOnElement(element,
		/*inherit_face_number*/-1, top_level_element, coordinate_transformation);
	if (!fieldElement)
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::calculate_values.  Field %s not defined on %d-D element %d",
			field->name, element->getDimension(), element->getIdentifier());
		return 0;
	}
	const FE_mesh *mesh = element->getMesh();
	if (!mesh)
	{
		display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  Invalid element");
		return 0;
	}
	const FE_nodeset *nodeset = mesh->getNodeset();
	if (!nodeset)
	{
		display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  No nodeset, invalid mesh");
		return 0;
	}

	int return_code = 1;
	const int elementDimension = element->getDimension();
	const int fieldElementDimension = fieldElement->getDimension();
	const int number_of_components = field->number_of_components;
	switch (field->fe_field_type)
	{
		case CONSTANT_FE_FIELD:
		{
			/* constant fields do not use the values except to remember the
					element and field they are for */
			this->field=ACCESS(FE_field)(field);
			this->element = element->access();
			// store field_element since we are now able to suggest through the
			// top_level_element clue which one we get. Must compare element
			// and field_element to ensure field values are still valid for
			// a given line or face. */
			this->field_element = fieldElement->access();
			this->component_number_in_xi=(int **)NULL;
			this->destroy_standard_basis_arguments = false;
			this->number_of_components=number_of_components;
			this->component_number_of_values=(int *)NULL;
			this->component_grid_values_storage=
				(const Value_storage **)NULL;
			this->component_base_grid_offset=(int *)NULL;
			this->component_grid_offset_in_xi=(int **)NULL;
			this->element_value_offsets=(int *)NULL;
			/* clear arrays not used for grid-based fields */
			this->component_values=(FE_value **)NULL;
			this->component_standard_basis_functions=
				(Standard_basis_function **)NULL;
			this->component_standard_basis_function_arguments=
				(int **)NULL;
			this->time_dependent = 0;
			this->time = time;
		} break;
		case INDEXED_FE_FIELD:
		{
			if (this->calculate_values(field->indexer_field, element,
				time, top_level_element))
			{
				/* restore pointer to original field - has the indexer_field in
						it anyway */
				REACCESS(FE_field)(&(this->field),field);
			}
			else
			{
				display_message(ERROR_MESSAGE,"FE_element_field_evaluation::calculate_values.  "
					"Cannot calculate element field values for indexer field");
				return_code=0;
			}
		} break;
		case GENERAL_FE_FIELD:
		{
			ALLOCATE(number_of_values_address, int, number_of_components);
			ALLOCATE(values_address, FE_value *, number_of_components);
			ALLOCATE(standard_basis_address, Standard_basis_function *, number_of_components);
			ALLOCATE(standard_basis_arguments_address, int *, number_of_components);
			blending_matrix = (FE_value *)NULL;
			ALLOCATE(component_number_in_xi, int *, number_of_components);
			if (number_of_values_address&&values_address&&
				standard_basis_address&&standard_basis_arguments_address&&
				component_number_in_xi)
			{
				for (i=0;i<number_of_components;i++)
				{
					number_of_values_address[i] = 0;
					values_address[i] = (FE_value *)NULL;
					standard_basis_address[i] = (Standard_basis_function *)NULL;
					standard_basis_arguments_address[i]=(int *)NULL;
					/* following is non-NULL only for grid-based components */
					component_number_in_xi[i] = NULL;
				}
				this->component_number_in_xi = component_number_in_xi;
				this->field = field->access();
				this->element = element->access();
				this->field_element = fieldElement->access();
				this->time_dependent = FE_field_has_multiple_times(this->field);
				this->time = time;
				this->destroy_standard_basis_arguments = fieldElementDimension > elementDimension;
				this->number_of_components = number_of_components;
				this->component_number_of_values = number_of_values_address;
				/* clear arrays only used for grid-based fields */
				this->component_grid_values_storage = 0;
				this->component_grid_offset_in_xi = 0;
				this->element_value_offsets = 0;

				this->component_values = values_address;
				this->component_standard_basis_functions = standard_basis_address;
				this->component_standard_basis_function_arguments = standard_basis_arguments_address;
				int grid_maximum_number_of_values = elementDimension + 1;
				for (i = elementDimension; i > 0; i--)
				{
					grid_maximum_number_of_values *= 2;
				}

				const FE_mesh_field_data *meshFieldData = field->meshFieldData[fieldElementDimension - 1];
				FE_basis *previous_basis = 0;
				return_code=1;
				int **component_grid_offset_in_xi = 0;
				const DsLabelIndex fieldElementIndex = fieldElement->getIndex();
				for (int component_number = 0; component_number < number_of_components; ++component_number)
				{
					const FE_element_field_template *componentEFT =
						meshFieldData->getComponentMeshfieldtemplate(component_number)->getElementfieldtemplate(fieldElementIndex);
					if (!componentEFT)
					{
						return_code = 0;
						break;
					}
					if ((componentEFT->getParameterMappingMode() == CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_ELEMENT)
						&& (0 != componentEFT->getLegacyGridNumberInXi()))
					{
						/* legacy grid-based */
						// always uses linear Lagrange for the element dimension, monomial has same number of functions
						int basisFunctionCount = 2;
						for (int d = 1; d < elementDimension; ++d)
							basisFunctionCount *= 2;
						ALLOCATE(*values_address, FE_value, basisFunctionCount);
						*number_of_values_address = basisFunctionCount;
						if (!(*values_address))
						{
							return_code = 0;
							break;
						}
						const int *top_level_component_number_in_xi = componentEFT->getLegacyGridNumberInXi();
						/* one-off allocation of arrays only needed for grid-based components */
						if (NULL == this->component_grid_values_storage)
						{
							const Value_storage **component_grid_values_storage;
							ALLOCATE(component_grid_values_storage, const Value_storage *, number_of_components);
							ALLOCATE(component_base_grid_offset, int, number_of_components);
							ALLOCATE(component_grid_offset_in_xi, int*, number_of_components);
							ALLOCATE(element_value_offsets, int, grid_maximum_number_of_values);
							if (component_grid_values_storage && component_base_grid_offset &&
								component_grid_offset_in_xi && element_value_offsets)
							{
								for (cn = 0; (cn < number_of_components); cn++)
								{
									component_grid_values_storage[cn]=NULL;
									component_base_grid_offset[cn]=0;
									component_grid_offset_in_xi[cn]=NULL;
								}
								this->component_grid_values_storage = component_grid_values_storage;
								this->component_base_grid_offset = component_base_grid_offset;
								this->component_grid_offset_in_xi = component_grid_offset_in_xi;
								this->element_value_offsets = element_value_offsets;
							}
							else
							{
								return_code = 0;
								break;
							}
						}
						// GRC risky to cache pointers into per-element data
						FE_mesh_field_data::ComponentBase *componentBase = meshFieldData->getComponentBase(component_number);
						const int valuesCount = componentEFT->getNumberOfElementDOFs();
						// following could be done as a virtual function
						switch (field->value_type)
						{
						case FE_VALUE_VALUE:
							{
								auto component = static_cast<FE_mesh_field_data::Component<FE_value>*>(componentBase);
								this->component_grid_values_storage[component_number] =
									reinterpret_cast<const Value_storage*>(component->getElementValues(fieldElementIndex, valuesCount));
							} break;
						case INT_VALUE:
							{
								auto component = static_cast<FE_mesh_field_data::Component<int>*>(componentBase);
								this->component_grid_values_storage[component_number] =
									reinterpret_cast<const Value_storage*>(component->getElementValues(fieldElementIndex, valuesCount));
							} break;
						default:
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::calculate_values.  Invalid value type for grid field");
								return_code = 0;
							} break;
						}
						if (!return_code)
							break;
						ALLOCATE(component_number_in_xi[component_number], int, elementDimension);
						ALLOCATE(component_grid_offset_in_xi[component_number], int, elementDimension);
						if ((NULL != component_number_in_xi[component_number]) &&
							(NULL != component_grid_offset_in_xi[component_number]))
						{
							this->component_base_grid_offset[component_number] = 0;
							if (!calculate_grid_field_offsets(elementDimension,
								fieldElementDimension, top_level_component_number_in_xi,
								coordinate_transformation, component_number_in_xi[component_number],
								&(this->component_base_grid_offset[component_number]),
								component_grid_offset_in_xi[component_number]))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::calculate_values.  "
									"Could not calculate grid field offsets");
								return_code=0;
								break;
							}
						}
						else
						{
							return_code = 0;
							break;
						}
					}
					else /* not grid-based; includes non-grid element-based */
					{
						// calculate element values for component on the fieldElement
						const int basisFunctionCount = componentEFT->getNumberOfFunctions();
						ALLOCATE(*values_address, FE_value, basisFunctionCount);
						if (!(*values_address))
						{
							return_code = 0;
							break;
						}
						switch (componentEFT->getParameterMappingMode())
						{
						case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_NODE:
						{
							if (0 == (*number_of_values_address = global_to_element_map_values(field, component_number,
								componentEFT, fieldElement, time, nodeset, *values_address)))
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
									"Could not calculate node-based values for field %s in %d-D element %d",
									field->name, fieldElementDimension, fieldElement->getIdentifier());
								return_code = 0;
								break;
							}
						} break;
						case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_ELEMENT:
						{
							// element-based mapping stores the parameters ready for use in the element
							if (field->value_type != FE_VALUE_VALUE)
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
									"Element-based non-grid field %s only implemented for real values", field->name);
								return_code = 0;
								break;
							}
							auto component = static_cast<FE_mesh_field_data::Component<FE_value> *>(meshFieldData->getComponentBase(component_number));
							const FE_value *values = component->getElementValues(fieldElementIndex, basisFunctionCount);
							if (!values)
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
									"Element-based field %s has no values at %d-D element %d",
									field->name, fieldElementDimension, fieldElement->getIdentifier());
								return_code = 0;
								break;
							}
							// transform values to standard basis functions if needed
							FE_basis *basis = componentEFT->getBasis();
							const int blendedElementValuesCount = FE_basis_get_number_of_blended_functions(basis);
							if (blendedElementValuesCount > 0)
							{
								FE_value *blendedElementValues = FE_basis_get_blended_element_values(basis, values);
								if (!blendedElementValues)
								{
									display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
										"Could not allocate memory for blended values");
									return_code = 0;
									break;
								}
								DEALLOCATE(*values_address); // future: avoid allocating this above
								*values_address = blendedElementValues;
								*number_of_values_address = blendedElementValuesCount;
							}
							else
							{
								memcpy(*values_address, values, basisFunctionCount*sizeof(FE_value));
								*number_of_values_address = basisFunctionCount;
							}
						} break;
						case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_FIELD:
						{
							if (field->value_type != FE_VALUE_VALUE)
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
									"Field-based field %s only implemented for real values", field->name);
								return_code = 0;
								break;
							}
							if (!get_FE_field_FE_value_value(field, component_number, *values_address))
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
									"Field-based field %s has no values at %d-D element %d",
									field->name, fieldElementDimension, fieldElement->getIdentifier());
								return_code = 0;
								break;
							}
							*number_of_values_address = 1;
						} break;
						case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_INVALID:
						{
							display_message(ERROR_MESSAGE, "FE_element_field_evaluation::calculate_values.  "
								"Invalid parameter mapping mode for field %s in %d-D element %d",
								field->name, fieldElementDimension, fieldElement->getIdentifier());
							return_code = 0;
						} break;
						}
						if (!return_code)
							break;
						if (previous_basis == componentEFT->getBasis())
						{
							*standard_basis_address = *(standard_basis_address - 1);
							*standard_basis_arguments_address = *(standard_basis_arguments_address - 1);
						}
						else
						{
							previous_basis = componentEFT->getBasis();
							if (blending_matrix)
							{
								DEALLOCATE(blending_matrix);
								blending_matrix = 0;
							}
							*standard_basis_address = FE_basis_get_standard_basis_function(previous_basis);
							if (fieldElementDimension > elementDimension)
							{
								if (!calculate_standard_basis_transformation(
									previous_basis, coordinate_transformation,
									elementDimension, standard_basis_arguments_address,
									&number_of_inherited_values, standard_basis_address,
									&blending_matrix))
								{
									return_code = 0;
									break;
								}
							}
							else
							{
								/* standard basis transformation is just a big identity matrix, so don't compute */
								/* also use the real basis arguments */
								*standard_basis_arguments_address =
									const_cast<int *>(FE_basis_get_standard_basis_function_arguments(previous_basis));
							}
						}
						if (fieldElement == element)
						{
							/* values already correct regardless of basis */
						}
						else if ((monomial_basis_functions== *standard_basis_address)||
							(polygon_basis_functions== *standard_basis_address))
						{
							/* project the fieldElement values onto the lower-dimension element
									using the affine transformation */
							/* allocate memory for the element values */
							ALLOCATE(inherited_values,FE_value, number_of_inherited_values);
							if (inherited_values)
							{
								row_size= *number_of_values_address;
								inherited_value=inherited_values;
								for (j=0;j<number_of_inherited_values;j++)
								{
									FE_value sum = 0.0;
									value= *values_address;
									transformation=blending_matrix+j;
									for (i=row_size;i>0;i--)
									{
										sum += (*transformation)*(*value);
										value++;
										transformation += number_of_inherited_values;
									}
									*inherited_value=(FE_value)sum;
									inherited_value++;
								}
								DEALLOCATE(*values_address);
								*values_address=inherited_values;
								*number_of_values_address=number_of_inherited_values;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::calculate_values.  "
									"Insufficient memory for inherited_values");
								DEALLOCATE(*values_address);
								return_code = 0;
								break;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::calculate_values.  Invalid basis");
							return_code = 0;
							break;
						}
						standard_basis_address++;
						standard_basis_arguments_address++;
					}
					number_of_values_address++;
					values_address++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::calculate_values.  Insufficient memory");
				DEALLOCATE(number_of_values_address);
				DEALLOCATE(values_address);
				DEALLOCATE(standard_basis_address);
				DEALLOCATE(standard_basis_arguments_address);
				return_code=0;
			}
			if (blending_matrix)
			{
				DEALLOCATE(blending_matrix);
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::calculate_values.  Unknown field type");
			return_code=0;
		} break;
	} /* switch (field->fe_field_type) */
	return (return_code);
}

int FE_element_field_evaluation::evaluate_int(int component_number,
	const FE_value *xi_coordinates, int *values)
{
	int return_code = 0;
	int cn, comp_no, components_to_calculate, *element_int_values, i, *number_in_xi,
		offset, this_comp_no, xi_offset;
	FE_value xi_coordinate;
	if ((this->field) && xi_coordinates && values &&
		((INT_VALUE == this->field->value_type)))
	{
		if ((0<=component_number)&&(component_number<this->field->number_of_components))
		{
			comp_no = component_number;
				components_to_calculate = 1;
		}
		else
		{
			comp_no = 0;
				components_to_calculate = this->field->number_of_components;
		}
		switch (this->field->fe_field_type)
		{
		case CONSTANT_FE_FIELD:
		{
			return_code = 1;
			for (i = 0; (i<components_to_calculate)&&return_code; i++)
			{
				if (!get_FE_field_int_value(this->field, comp_no, &values[i]))
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_evaluation::evaluate_int.  "
						"Could not get values for constant field %s", this->field->name);
					return_code = 0;
				}
				comp_no++;
			}
		} break;
		case INDEXED_FE_FIELD:
		{
			int index;

			FE_field *indexed_field = this->field;
			REACCESS(FE_field)(&(this->field), indexed_field->indexer_field);
			if (this->evaluate_int(/*component_number*/0, xi_coordinates, &index))
			{
				/* index numbers start at 1 */
				if ((1<=index)&&(index<=indexed_field->number_of_indexed_values))
				{
					return_code = 1;
					int value_no = index-1 + comp_no*indexed_field->number_of_indexed_values;
					for (i = 0; (i<components_to_calculate)&&return_code; i++)
					{
						if (!get_FE_field_int_value(indexed_field, value_no, &values[i]))
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::evaluate_int.  "
								"Could not get values for constant field %s", indexed_field->name);
							return_code = 0;
						}
						value_no += indexed_field->number_of_indexed_values;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_evaluation::evaluate_int.  "
						"Index field %s gave out-of-range index %d in field %s",
						indexed_field->indexer_field->name, index, indexed_field->name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::evaluate_int.  "
					"Could not calculate indexer field %s for field %s at %d-D element %",
					indexed_field->indexer_field->name, indexed_field->name,
					this->element->getDimension(),
					this->element->getIdentifier());
			}
			REACCESS(FE_field)(&(this->field), indexed_field);
		} break;
		case GENERAL_FE_FIELD:
		{
			return_code = 1;
			const int number_of_xi_coordinates = this->element->getDimension();
			for (cn = 0; (cn<components_to_calculate)&&return_code; cn++)
			{
				this_comp_no = comp_no + cn;
				number_in_xi = this->component_number_in_xi[this_comp_no];
				if (number_in_xi)
				{
					/* for integer, get value at nearest grid point */
					return_code = 1;
					offset = this->component_base_grid_offset[this_comp_no];
					for (i = 0; (i<number_of_xi_coordinates)&&return_code; i++)
					{
						xi_coordinate = xi_coordinates[i];
						if (xi_coordinate < 0.0)
						{
							xi_coordinate = 0.0;
						}
						else if (xi_coordinate > 1.0)
						{
							xi_coordinate = 1.0;
						}
						/* get nearest xi_offset */
						xi_offset = (int)floor((double)number_in_xi[i]*(double)xi_coordinate+0.5);
						offset += xi_offset*this->component_grid_offset_in_xi[this_comp_no][i];
					}
					element_int_values = (int *)this->component_grid_values_storage[this_comp_no];
					values[cn] = element_int_values[offset];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_evaluation::evaluate_int.  "
						"Non-grid-based component for integer valued field");
					return_code = 0;
				}
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::evaluate_int.  Unknown field type");
		} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::evaluate_int.  Invalid argument(s)");
	}
	return (return_code);
}

int FE_element_field_evaluation::evaluate_real(int component_number,
	const FE_value *xi_coordinates, Standard_basis_function_evaluation &basis_function_evaluation,
	int derivative_order, FE_value *values)
{
	int return_code = 1;
	if ((this->field) && (xi_coordinates) && (values))
	{
		const int dimension = this->element->getDimension();
		const int number_of_derivatives = this->get_number_of_derivatives(derivative_order, dimension);
		int comp_no, components_to_calculate;
		if ((0<=component_number)&&(component_number<this->field->number_of_components))
		{
			comp_no=component_number;
			components_to_calculate=1;
		}
		else
		{
			comp_no=0;
			components_to_calculate=this->field->number_of_components;
		}
		switch (this->field->fe_field_type)
		{
			case GENERAL_FE_FIELD:
			{
				/* calculate a value for each component */
				Standard_basis_function *current_standard_basis_function = 0;
				int *current_standard_basis_function_arguments = 0;
				Standard_basis_function **standard_basis_function_ptr = this->component_standard_basis_functions;
				int **standard_basis_function_arguments_ptr = this->component_standard_basis_function_arguments;
				FE_value *calculated_value = values;
				int *number_of_element_values_ptr = this->component_number_of_values + comp_no;
				FE_value **element_values_ptr = this->component_values + comp_no;
				standard_basis_function_ptr += comp_no;
				standard_basis_function_arguments_ptr += comp_no;
				for (int cn = 0; cn < components_to_calculate; ++cn)
				{
					const int this_comp_no = comp_no + cn;
					const int *number_in_xi = this->component_number_in_xi[this_comp_no];
					if (number_in_xi)
					{
						/* legacy grid-based */
						// convert parent xi to grid base offset and grid xi
						int grid_base_offset = this->component_base_grid_offset[this_comp_no];
						FE_value grid_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
						bool different_grid_xi = false;
						FE_value number_in_xi_real[MAXIMUM_ELEMENT_XI_DIMENSIONS];  // used to convert derivatives to be w.r.t. parent xi
						for (int i = 0; i < dimension; ++i)
						{
							number_in_xi_real[i] = static_cast<FE_value>(number_in_xi[i]);
							FE_value xi_coordinate = xi_coordinates[i];
							if (xi_coordinate >= 1.0)
							{
								grid_xi[i] = 1.0 + (xi_coordinate - 1.0)*number_in_xi_real[i];
								if (number_in_xi[i] > 1)
									grid_base_offset += (number_in_xi[i] - 1)*this->component_grid_offset_in_xi[this_comp_no][i];
							}
							else if (xi_coordinate <= 0.0)
							{
								grid_xi[i] = xi_coordinate*number_in_xi_real[i];
							}
							else
							{
								xi_coordinate *= number_in_xi_real[i];
								const FE_value xi_offset_real = floor(xi_coordinate);
								grid_xi[i] = xi_coordinate - xi_offset_real;
								grid_base_offset += static_cast<int>(xi_offset_real)*this->component_grid_offset_in_xi[this_comp_no][i];
							}
							if (grid_xi[i] != this->last_grid_xi[i])
							{
								different_grid_xi = true;
								this->last_grid_xi[i] = grid_xi[i];
							}
						}
						if (different_grid_xi)
							this->grid_basis_function_evaluation.invalidate();

						// fill component values from grid parameters in this grid cell
						// these are for linear Lagrange interpolation for dimension
						const int size = get_Value_storage_size(this->field->value_type, (struct FE_time_sequence *)NULL);
						const Value_storage **component_grid_values_storage = this->component_grid_values_storage + this_comp_no;
						const int number_of_values = *number_of_element_values_ptr;
						FE_value *set_element_values = *element_values_ptr;
						for (int i = 0; i < number_of_values; ++i)
						{
							int offset = grid_base_offset;
							for (int d = 0; d < dimension; ++d)
								if (i & (1 << d))
									offset += this->component_grid_offset_in_xi[this_comp_no][d];
							set_element_values[i] = *(reinterpret_cast<const FE_value*>(*component_grid_values_storage + size*offset));
						}

						// convert linear Lagrange parameters to monomial: 1 x y xy z xz xy xyz
						// first parameter at xi = (0,0,0) is unchanged
						for (int i = 1; i < number_of_values; ++i)
						{
							FE_value delta = set_element_values[0];
							for (int j = 1; j < i; ++j)
								if ((i & j) == j)
									delta += set_element_values[j];
							set_element_values[i] -= delta;
						}

						// interpolate
						const int standard_basis_function_arguments[] = { dimension, 1, 1, 1 };
						const FE_value *basis_values = this->grid_basis_function_evaluation.evaluate(
							monomial_basis_functions, standard_basis_function_arguments, grid_xi, derivative_order);
						if (!basis_values)
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::evaluate_real.  Error calculating grid standard basis");
							return_code = 0;
							break;
						}
						else
						{
							// calculate field value as a dot product of the element and basis values
							const FE_value *basis_value = basis_values;
							for (int k = 0; k < number_of_derivatives; ++k)
							{
								const FE_value *element_value = *element_values_ptr;
								FE_value sum = 0.0;
								// keep this loop simple & let compiler optimise it
								for (int j = 0; j < number_of_values; ++j)
									sum += element_value[j]*basis_value[j];
								if (derivative_order)
								{
									// scale grid derivatives to be w.r.t. parent element xi
									int j = k;
									for (int o = 0; o < derivative_order; ++o)
									{
										sum *= number_in_xi_real[j % dimension];
										j /= dimension;
									}
								}
								*calculated_value = sum;
								++calculated_value;
								basis_value += number_of_values;
							}
						}
					}
					else
					{
						/* standard interpolation */
						const FE_value *basis_values = basis_function_evaluation.evaluate(
							*standard_basis_function_ptr, *standard_basis_function_arguments_ptr, xi_coordinates, derivative_order);
						if (!basis_values)
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::evaluate_real.  Error calculating standard basis");
							return_code = 0;
							break;
						}
						else
						{
							// calculate field value as a dot product of the element and basis values
							const int number_of_values = *number_of_element_values_ptr;
							const FE_value *basis_value = basis_values;
							for (int k = 0; k < number_of_derivatives; ++k)
							{
								const FE_value *element_value = *element_values_ptr;
								FE_value sum = 0.0;
								// keep this loop simple & let compiler optimise it
								for (int j = 0; j < number_of_values; ++j)
									sum += element_value[j]*basis_value[j];
								*calculated_value = sum;
								++calculated_value;
								basis_value += number_of_values;
							}
						}
					}
					++number_of_element_values_ptr;
					++element_values_ptr;
					++standard_basis_function_ptr;
					++standard_basis_function_arguments_ptr;
				}
			} break;
			case CONSTANT_FE_FIELD:
			case INDEXED_FE_FIELD:
			{
				if (derivative_order <= 0)
				{
					if (this->field->fe_field_type == CONSTANT_FE_FIELD)
					{
						for (int i = 0; i < components_to_calculate; ++i)
						{
							if (!get_FE_field_FE_value_value(this->field, comp_no, &values[i]))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::evaluate_real.  "
									"Could not get values for constant field %s", this->field->name);
								return_code = 0;
								break;
							}
							comp_no++;
						}
					}
					else // (this->field->fe_field_type == INDEXED_FE_FIELD)
					{
						int index;
						FE_field *indexed_field = this->field;
						REACCESS(FE_field)(&(this->field), indexed_field->indexer_field);
						if (this->evaluate_int(/*component_number*/0, xi_coordinates, &index))
						{
							/* index numbers start at 1 */
							if ((1<=index)&&(index<=indexed_field->number_of_indexed_values))
							{
								int value_no = index-1 + comp_no*indexed_field->number_of_indexed_values;
								for (int i = 0; i < components_to_calculate; ++i)
								{
									if (!get_FE_field_FE_value_value(indexed_field, value_no, &values[i]))
									{
										display_message(ERROR_MESSAGE,
											"calculate_FE_element_field.  "
											"Could not get values for indexed field %s", indexed_field->name);
										return_code = 0;
										break;
									}
									value_no += indexed_field->number_of_indexed_values;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE, "FE_element_field_evaluation::evaluate_real.  "
									"Index field %s gave out-of-range index %d in field %s",
									indexed_field->indexer_field->name, index, indexed_field->name);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE, "FE_element_field_evaluation::evaluate_real.  "
								"Could not calculate indexer field %s for field %s at %d-D element %",
								indexed_field->indexer_field->name, indexed_field->name,
								this->element->getDimension(), this->element->getIdentifier());
							return_code = 0;
						}
						REACCESS(FE_field)(&(this->field), indexed_field);
					}
				}
				else
				{
					/* derivatives are zero for constant and indexed fields */
					const int values_count = number_of_derivatives*components_to_calculate;
					for (int i = 0; 0 < values_count; ++i)
						values[i] = 0.0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::evaluate_real.  Unknown field type");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::evaluate_real.  Invalid argument(s)\n"
			"xi_coordinates %p, derivative_order %d, values %p",
			xi_coordinates, derivative_order, values);
		return_code = 0;
	}
	return (return_code);
}

int FE_element_field_evaluation::evaluate_string(int component_number,
	const FE_value *xi_coordinates, char **values)
{
	int comp_no, components_to_calculate, i, j, return_code;
	return_code = 0;
	if ((this->field) && (xi_coordinates) && (values) && (STRING_VALUE == this->field->value_type))
	{
		if ((0<=component_number)&&(component_number<field->number_of_components))
		{
			comp_no = component_number;
			components_to_calculate = 1;
		}
		else
		{
			comp_no = 0;
			components_to_calculate = field->number_of_components;
		}
		switch (field->fe_field_type)
		{
		case CONSTANT_FE_FIELD:
		{
			return_code = 1;
			for (i = 0; (i<components_to_calculate)&&return_code; i++)
			{
				if (!get_FE_field_string_value(field, comp_no, &values[i]))
				{
					display_message(ERROR_MESSAGE,
						"calculate_FE_element_field_string_values.  "
						"Could not get values for constant field %s", field->name);
					return_code = 0;
					for (j = 0; j<i; j++)
					{
						DEALLOCATE(values[j]);
					}
				}
				comp_no++;
			}
		} break;
		case INDEXED_FE_FIELD:
		{
			int index, value_no;

			FE_field *indexed_field = this->field;
			REACCESS(FE_field)(&(this->field), indexed_field->indexer_field);
			if (this->evaluate_int(/*component_number*/0, xi_coordinates, &index))
			{
				/* index numbers start at 1 */
				if ((1<=index)&&(index<=indexed_field->number_of_indexed_values))
				{
					return_code = 1;
					value_no = index-1 + comp_no*indexed_field->number_of_indexed_values;
					for (i = 0; (i<components_to_calculate)&&return_code; i++)
					{
						if (!get_FE_field_string_value(indexed_field, value_no, &values[i]))
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::evaluate_string.  "
								"Could not get values for indexed field %s", indexed_field->name);
							return_code = 0;
							for (j = 0; j<i; j++)
							{
								DEALLOCATE(values[j]);
							}
						}
						value_no += indexed_field->number_of_indexed_values;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"FE_element_field_evaluation::evaluate_string.  "
						"Index field %s gave out-of-range index %d in field %s",
						indexed_field->indexer_field->name, index, indexed_field->name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::evaluate_string.  "
					"Could not calculate index field %s for field %s at %d-D element %",
					indexed_field->indexer_field->name, indexed_field->name,
					this->element->getDimension(),
					this->element->getIdentifier());
			}
			REACCESS(FE_field)(&(this->field), indexed_field);
		} break;
		case GENERAL_FE_FIELD:
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::evaluate_string.  "
				"General fields not supported");
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::evaluate_string.  Unknown field type");
		} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::evaluate_string.  Invalid argument(s)");
	}
	return (return_code);
}

int FE_element_field_evaluation::evaluate_as_string(int component_number,
	const FE_value *xi_coordinates, Standard_basis_function_evaluation &basis_function_evaluation, char **out_string)
{
	int return_code = 0;
	char temp_string[40];
	int components_to_calculate,error,i;

	if ((xi_coordinates) && (out_string) && (this->field))
	{
		(*out_string) = 0;
		if ((0<=component_number)&&(component_number<this->field->number_of_components))
		{
			components_to_calculate=1;
		}
		else
		{
			components_to_calculate=this->field->number_of_components;
		}
		switch (this->field->value_type)
		{
			case FE_VALUE_VALUE:
			{
				FE_value *values;

				if (ALLOCATE(values,FE_value,components_to_calculate))
				{
					if (this->evaluate_real(component_number, xi_coordinates,
						basis_function_evaluation, /*derivative_order*/0, values))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								sprintf(temp_string,",%g",values[i]);
							}
							else
							{
								sprintf(temp_string,"%g",values[i]);
							}
							append_string(out_string,temp_string,&error);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_element_field_evaluation::evaluate_as_string.  "
							"Could not calculate FE_value values");
					}
					DEALLOCATE(values);
				}
			} break;
			case INT_VALUE:
			{
				int *values;

				if (ALLOCATE(values,int,components_to_calculate))
				{
					if (this->evaluate_int(component_number, xi_coordinates, values))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								sprintf(temp_string,",%d",values[i]);
							}
							else
							{
								sprintf(temp_string,"%d",values[i]);
							}
							append_string(out_string,temp_string,&error);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_element_field_evaluation::evaluate_as_string.  "
							"Could not calculate int values");
					}
					DEALLOCATE(values);
				}
			} break;
			case STRING_VALUE:
			{
				char **values;

				if (ALLOCATE(values,char *,components_to_calculate))
				{
					if (this->evaluate_string(component_number, xi_coordinates, values))
					{
						error=0;
						for (i=0;i<components_to_calculate;i++)
						{
							if (0<i)
							{
								append_string(out_string,",",&error);
							}
							append_string(out_string,values[i],&error);
						}
						for (i=0;i<components_to_calculate;i++)
						{
							DEALLOCATE(values[i]);
						}
						return_code= !error;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"FE_element_field_evaluation::evaluate_as_string.  "
							"Could not calculate string values");
					}
					DEALLOCATE(values);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::evaluate_as_string.  Unknown value type %s",
					Value_type_string(this->field->value_type));
			} break;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::evaluate_as_string.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::evaluate_as_string.  Invalid argument(s)");
		if (out_string)
			(*out_string) = 0;
	}
	return (return_code);
}

int FE_element_field_evaluation::get_component_values(int component_number,
	int *number_of_component_values_address, FE_value **component_values_address) const
{
	int return_code = 0;
	if ((this->element)&&
		(0<=component_number)&&
		(component_number<this->number_of_components)&&
		number_of_component_values_address&&component_values_address)
	{
		if ((this->component_number_of_values)&&
			(0<(*number_of_component_values_address =
				this->component_number_of_values[component_number]))&&
				(this->component_values)&&
			this->component_values[component_number]&&
			ALLOCATE(*component_values_address, FE_value,
				*number_of_component_values_address))
		{
			memcpy(*component_values_address,
				this->component_values[component_number],
				(*number_of_component_values_address)*sizeof(FE_value));
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::get_component_values.  "
				"Component has no values");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::get_component_values.  "
			"Invalid argument(s).  %p %d %d %p %p",
			this->element, component_number,
			this->number_of_components,
			number_of_component_values_address, component_values_address);
	}
	return (return_code);
}

/** If component_number is monomial, integer values describing the monomial basis
 * are returned. The first number is the dimension, the following numbers are the
 * order of the monomial in each direction, where 3=cubic, for example.
 * <monomial_info> should point to a block of memory big enough to take
 * 1 + MAXIMUM_ELEMENT_XI_DIMENSIONS integers. */
int FE_element_field_evaluation::get_monomial_component_info(int component_number,
	int *monomial_info) const
{
	int return_code = 0;
	int i, *source_monomial_info;
	if ((this->element) && (0 <= component_number) &&
		(component_number < this->number_of_components) && (monomial_info))
	{
		if ((this->component_standard_basis_function_arguments)&&
			(source_monomial_info = this->
				component_standard_basis_function_arguments[component_number])&&
			standard_basis_function_is_monomial(this->
				component_standard_basis_functions[component_number],
				(void *)source_monomial_info))
		{
			*monomial_info = *source_monomial_info;
			for (i = 1; i<= *monomial_info; i++)
			{
				monomial_info[i] = source_monomial_info[i];
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_element_field_evaluation::get_monomial_component_info.  "
				"Component is not monomial");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::get_monomial_component_info.  "
			"Invalid argument(s).  %p %d %d %p",
			this->element, component_number,
			this->number_of_components, monomial_info);
	}
	return (return_code);
}