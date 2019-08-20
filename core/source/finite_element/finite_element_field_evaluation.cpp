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

#include "opencmiss/zinc/result.h"
#include "finite_element/finite_element_field_evaluation.hpp"
#include "finite_element/finite_element_field_private.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_value_storage.hpp"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/message.h"

#define DOUBLE_FOR_DOT_PRODUCT

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
	derivatives_calculated(false),
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
	grid_basis_function_values(0),
	access_count(1)
{
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
	if (this->grid_basis_function_values)
	{
		DEALLOCATE(this->grid_basis_function_values);
	}
}

int FE_element_field_evaluation::calculate_values(FE_field *field, cmzn_element *element,
	FE_value time, bool calculate_derivatives, cmzn_element *top_level_element)
{
	FE_value *blending_matrix,
		*derivative_value,*inherited_value,*inherited_values,scalar,
		*second_derivative_value,*transformation,*value,**values_address;
	int cn,**component_number_in_xi,
		*component_base_grid_offset, *element_value_offsets, i, j, k,
		number_of_grid_based_components,
		number_of_inherited_values,number_of_polygon_verticies,number_of_values,
		*number_of_values_address,offset,order,*orders,polygon_offset,power,
		row_size,**standard_basis_arguments_address;
	Standard_basis_function **standard_basis_address;
	// this had used DOUBLE_FOR_DOT_PRODUCT, but FE_value will be at least double precision now
	FE_value sum;

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
			/* derivatives will be calculated in calculate_FE_element_field */
			this->derivatives_calculated = true;
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
			this->grid_basis_function_values=(FE_value *)NULL;
			this->time_dependent = 0;
			this->time = time;
		} break;
		case INDEXED_FE_FIELD:
		{
			if (this->calculate_values(field->indexer_field, element,
				time, calculate_derivatives, top_level_element))
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
			ALLOCATE(number_of_values_address,int,number_of_components);
			ALLOCATE(values_address,FE_value *,number_of_components);
			ALLOCATE(standard_basis_address,Standard_basis_function *,
				number_of_components);
			ALLOCATE(standard_basis_arguments_address,int *,
				number_of_components);
			blending_matrix=(FE_value *)NULL;
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
				this->time_dependent =
					FE_field_has_multiple_times(this->field);
				this->time = time;
				this->derivatives_calculated=calculate_derivatives;
				this->destroy_standard_basis_arguments = fieldElementDimension > elementDimension;
				this->number_of_components=number_of_components;
				this->component_number_of_values=
					number_of_values_address;
				/* clear arrays only used for grid-based fields */
				this->component_grid_values_storage=
					(const Value_storage **)NULL;
				this->component_grid_offset_in_xi=(int **)NULL;
				this->element_value_offsets=(int *)NULL;

				this->component_values=values_address;
				this->component_standard_basis_functions=
					standard_basis_address;
				this->component_standard_basis_function_arguments=
					standard_basis_arguments_address;
				int grid_maximum_number_of_values = elementDimension + 1;
				for (i = elementDimension; i > 0; i--)
				{
					grid_maximum_number_of_values *= 2;
				}

				const FE_mesh_field_data *meshFieldData = field->meshFieldData[fieldElementDimension - 1];
				FE_basis *previous_basis = 0;
				return_code=1;
				number_of_grid_based_components = 0;
				int **component_grid_offset_in_xi = 0;
				const DsLabelIndex fieldElementIndex = fieldElement->getIndex();
				for (int component_number = 0; return_code && (component_number < number_of_components); ++component_number)
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
						if (0 == this->grid_basis_function_values)
						{
							// allocate enough space for linear basis plus first derivatives
							if (!(ALLOCATE(this->grid_basis_function_values, FE_value, grid_maximum_number_of_values)))
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::calculate_values.  "
									"Could not allocate grid_basis_function_values");
								return_code = 0;
								break;
							}
						}
						const int *top_level_component_number_in_xi = componentEFT->getLegacyGridNumberInXi();
						if (!top_level_component_number_in_xi)
						{
							display_message(ERROR_MESSAGE,
								"FE_element_field_evaluation::calculate_values.  Element parameter mapping only implemented for legacy grid");
							return_code = 0;
							break;
						}
						++number_of_grid_based_components;
						if (number_of_grid_based_components == number_of_components)
						{
							this->derivatives_calculated = true;
						}
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
							*standard_basis_address= *(standard_basis_address-1);
							*standard_basis_arguments_address=
								*(standard_basis_arguments_address-1);
						}
						else
						{
							previous_basis = componentEFT->getBasis();
							if (blending_matrix)
							{
								/* SAB We don't want to keep the old one */
								DEALLOCATE(blending_matrix);
								blending_matrix=NULL;
							}
							*standard_basis_address = FE_basis_get_standard_basis_function(previous_basis);
							if (fieldElementDimension > elementDimension)
							{
								return_code=calculate_standard_basis_transformation(
									previous_basis,coordinate_transformation,
									elementDimension,standard_basis_arguments_address,
									&number_of_inherited_values,standard_basis_address,
									&blending_matrix);
							}
							else
							{
								/* standard basis transformation is just a big identity matrix, so don't compute */
								/* also use the real basis arguments */
								*standard_basis_arguments_address =
									const_cast<int *>(FE_basis_get_standard_basis_function_arguments(previous_basis));
							}
						}
						if (return_code)
						{
							if (fieldElement == element)
							{
								/* values already correct regardless of basis, but must make space for derivatives if needed */
								if (calculate_derivatives)
								{
									if (REALLOCATE(inherited_values,*values_address,FE_value,
										(elementDimension+1)*(*number_of_values_address)))
									{
										*values_address=inherited_values;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"FE_element_field_evaluation::calculate_values.  Could not reallocate values");
										return_code=0;
									}
								}
							}
							else if ((monomial_basis_functions== *standard_basis_address)||
								(polygon_basis_functions== *standard_basis_address))
							{
								/* project the fieldElement values onto the lower-dimension element
										using the affine transformation */
								/* allocate memory for the element values */
								if (calculate_derivatives)
								{
									ALLOCATE(inherited_values,FE_value,
										(elementDimension+1)*number_of_inherited_values);
								}
								else
								{
									ALLOCATE(inherited_values,FE_value,
										number_of_inherited_values);
								}
								if (inherited_values)
								{
									row_size= *number_of_values_address;
									inherited_value=inherited_values;
									for (j=0;j<number_of_inherited_values;j++)
									{
										sum=0;
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
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"FE_element_field_evaluation::calculate_values.  Invalid basis");
								return_code=0;
							}
							if (return_code)
							{
								if (calculate_derivatives)
								{
									/* calculate the derivatives with respect to the xi
											coordinates */
									if (monomial_basis_functions==
										*standard_basis_address)
									{
										number_of_values= *number_of_values_address;
										value= *values_address;
										derivative_value=value+number_of_values;
										orders= *standard_basis_arguments_address;
										offset=1;
										for (i=elementDimension;i>0;i--)
										{
											orders++;
											order= *orders;
											for (j=0;j<number_of_values;j++)
											{
												/* calculate derivative value */
												power=(j/offset)%(order+1);
												if (order==power)
												{
													*derivative_value=0;
												}
												else
												{
													*derivative_value=
														(FE_value)(power+1)*value[j+offset];
												}
												/* move to the next derivative value */
												derivative_value++;
											}
											offset *= (order+1);
										}
									}
									else if (polygon_basis_functions==
										*standard_basis_address)
									{
										number_of_values= *number_of_values_address;
										value= *values_address;
										derivative_value=value+number_of_values;
										orders= *standard_basis_arguments_address;
										offset=1;
										for (i=elementDimension;i>0;i--)
										{
											orders++;
											order= *orders;
											if (order<0)
											{
												/* polygon */
												order= -order;
												if (order%2)
												{
													/* calculate derivatives with respect to
														both polygon coordinates */
													order /= 2;
													polygon_offset=order%elementDimension;
													order /= elementDimension;
													number_of_polygon_verticies=
														(-orders[polygon_offset])/2;
													/* first polygon coordinate is
														circumferential */
													second_derivative_value=derivative_value+
														(polygon_offset*number_of_values);
													order=4*number_of_polygon_verticies;
													scalar=
														(FE_value)number_of_polygon_verticies;
													for (j=0;j<number_of_values;j++)
													{
														/* calculate derivative values */
														k=(j/offset)%order;
														switch (k/number_of_polygon_verticies)
														{
															case 0:
															{
																*derivative_value=scalar*value[j+
																	number_of_polygon_verticies*offset];
																*second_derivative_value=value[j+
																	2*number_of_polygon_verticies*
																	offset];
															} break;
															case 1:
															{
																*derivative_value=0;
																*second_derivative_value=value[j+
																	2*number_of_polygon_verticies*
																	offset];
															} break;
															case 2:
															{
																*derivative_value=scalar*value[j+
																	number_of_polygon_verticies*offset];
																*second_derivative_value=0;
															} break;
															case 3:
															{
																*derivative_value=0;
																*second_derivative_value=0;
															} break;
														}
														/* move to the next derivative value */
														derivative_value++;
														second_derivative_value++;
													}
													offset *= order;
												}
												else
												{
													/* second polgon xi.  Derivatives already
														calculated */
													derivative_value += number_of_values;
												}
											}
											else
											{
												/* not polygon */
												for (j=0;j<number_of_values;j++)
												{
													/* calculate derivative value */
													power=(j/offset)%(order+1);
													if (order==power)
													{
														*derivative_value=0;
													}
													else
													{
														*derivative_value=
															(FE_value)(power+1)*value[j+offset];
													}
													/* move to the next derivative value */
													derivative_value++;
												}
												offset *= (order+1);
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"FE_element_field_evaluation::calculate_values.  "
											"Invalid basis");
										DEALLOCATE(*values_address);
										return_code=0;
									}
								}
							}
						}
						number_of_values_address++;
						values_address++;
						standard_basis_address++;
						standard_basis_arguments_address++;
					}
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

int FE_element_field_evaluation::differentiate(int xi_index)
{
	FE_value *derivative_value, *value;
	int elementDimension, i, j, k, number_of_values, offset, order,
		*orders, power, return_code;

	if (this->derivatives_calculated)
	{
		return_code = 1;
		elementDimension = this->element->getDimension();
		for (k = 0 ; k < this->number_of_components ; k++)
		{
			if (monomial_basis_functions==
				this->component_standard_basis_functions[k])
			{
				number_of_values = this->component_number_of_values[k];
				value = this->component_values[k];

				/* Copy the specified derivative back into the values */
				derivative_value = value + number_of_values * (xi_index + 1);
				for (j=0;j<number_of_values;j++)
				{
					*value = *derivative_value;
					value++;
					derivative_value++;
				}

				/* Now differentiate the values monomial as we did to calculate them above */

				value = this->component_values[k];
				derivative_value = value + number_of_values;

				orders= this->component_standard_basis_function_arguments[k];
				offset = 1;

				for (i=elementDimension;i>0;i--)
				{
					orders++;
					order= *orders;
					for (j=0;j<number_of_values;j++)
					{
						/* calculate derivative value */
						power=(j/offset)%(order+1);
						if (order==power)
						{
							*derivative_value=0;
						}
						else
						{
							*derivative_value=
								(FE_value)(power+1)*value[j+offset];
						}
						/* move to the next derivative value */
						derivative_value++;
					}
					offset *= (order+1);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::differentiate.  Unsupported basis type");
				return_code=0;
				break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::differentiate.  Must have derivatives calculated");
		return_code=0;
	}
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
	const FE_value *xi_coordinates, Standard_basis_function_values &basis_function_values,
	FE_value *values, FE_value *jacobian)
{
	int cn,comp_no,*component_number_of_values,components_to_calculate,
		i,j,k,l,m, *last_number_in_xi,*number_in_xi,
		number_of_xi_coordinates,recompute_basis,
		return_code,size,this_comp_no,xi_offset;
	FE_value *calculated_value,
		**component_values,*derivative,*element_value,temp,xi_coordinate;
	Standard_basis_function *current_standard_basis_function,
		**component_standard_basis_function;
	int **component_standard_basis_function_arguments,
		*current_standard_basis_function_arguments;
	const Value_storage **component_grid_values_storage,*element_values_storage;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
	double sum;
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
	FE_value sum;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */

	return_code=0;
	if ((this->field) && (xi_coordinates) && (values) &&
		(!jacobian||(jacobian&&(this->derivatives_calculated))))
	{
		const int dimension = this->element->getDimension();
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
			case CONSTANT_FE_FIELD:
			{
				return_code=1;
				for (i=0;(i<components_to_calculate)&&return_code;i++)
				{
					if (!get_FE_field_FE_value_value(this->field,comp_no,&values[i]))
					{
						display_message(ERROR_MESSAGE,
							"FE_element_field_evaluation::evaluate_real.  "
							"Could not get values for constant field %s",this->field->name);
						return_code=0;
					}
					comp_no++;
				}
				if (jacobian)
				{
					/* derivatives are zero for constant fields */
					derivative=jacobian;
					for (i = (this->field->number_of_components)*dimension; 0 < i ; --i)
					{
						*derivative = 0.0;
						derivative++;
					}
				}
			} break;
			case INDEXED_FE_FIELD:
			{
				int index,value_no;

				FE_field *indexed_field = this->field;
				REACCESS(FE_field)(&(this->field), indexed_field->indexer_field);
				if (this->evaluate_int(/*component_number*/0, xi_coordinates, &index))
				{
					/* index numbers start at 1 */
					if ((1<=index)&&(index<=indexed_field->number_of_indexed_values))
					{
						return_code=1;
						value_no = index-1 + comp_no*indexed_field->number_of_indexed_values;
						for (i=0;(i<components_to_calculate)&&return_code;i++)
						{
							if (!get_FE_field_FE_value_value(indexed_field,value_no,&values[i]))
							{
								display_message(ERROR_MESSAGE,
									"calculate_FE_element_field.  "
									"Could not get values for constant field %s", indexed_field->name);
								return_code=0;
							}
							value_no += indexed_field->number_of_indexed_values;
						}
						if (jacobian)
						{
							/* derivatives are zero for indexed fields */
							derivative=jacobian;
							for (i = (indexed_field->number_of_components)*dimension; 0 < i ; --i)
							{
								*derivative = 0.0;
								derivative++;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"FE_element_field_evaluation::evaluate_real.  "
							"Index field %s gave out-of-range index %d in field %s",
							indexed_field->indexer_field->name,index, indexed_field->name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"FE_element_field_evaluation::evaluate_real.  "
						"Could not calculate indexer field %s for field %s at %d-D element %",
						indexed_field->indexer_field->name, indexed_field->name,
						this->element->getDimension(),
						this->element->getIdentifier());
				}
				REACCESS(FE_field)(&(this->field), indexed_field);
			} break;
			case GENERAL_FE_FIELD:
			{
				/* calculate the value for the element field */
				return_code=1;
				/* calculate a value for each component */
				current_standard_basis_function=(Standard_basis_function *)NULL;
				current_standard_basis_function_arguments=(int *)NULL;
				component_number_of_values=
					this->component_number_of_values;
				component_values=this->component_values;
				component_standard_basis_function=
					this->component_standard_basis_functions;
				component_standard_basis_function_arguments=
					this->component_standard_basis_function_arguments;
				calculated_value=values;
				if (this->derivatives_calculated)
				{
					derivative=jacobian;
				}
				else
				{
					derivative=(FE_value *)NULL;
				}
				component_number_of_values += comp_no;
				component_values += comp_no;
				component_standard_basis_function += comp_no;
				component_standard_basis_function_arguments += comp_no;
				number_of_xi_coordinates = dimension;
				int *element_value_offsets = 0;
				int *element_value_offset = 0;
				int number_of_values = 0;
				int offset = 0;
				for (cn=0;(cn<components_to_calculate)&&return_code;cn++)
				{
					this_comp_no = comp_no + cn;
					number_in_xi = this->component_number_in_xi[this_comp_no];
					if (number_in_xi)
					{
						/* grid based */
						FE_value *basis_values = this->grid_basis_function_values;
						current_standard_basis_function = NULL;
						current_standard_basis_function_arguments = NULL;
						/* optimisation: reuse basis from last component if same number_in_xi */
						recompute_basis = 1;
						if ((cn > 0) && (last_number_in_xi = this->component_number_in_xi[this_comp_no - 1]))
						{
							recompute_basis = 0;
							for (i = 0; i < number_of_xi_coordinates; i++)
							{
								if (number_in_xi[i] != last_number_in_xi[i])
								{
									recompute_basis = 1;
									break;
								}
							}
						}
						if (recompute_basis)
						{
							number_of_values=1;
							for (i=number_of_xi_coordinates;i>0;i--)
							{
								number_of_values *= 2;
							}
							return_code=1;
							i=0;
							offset=this->component_base_grid_offset[this_comp_no];
							*basis_values = 1;
							element_value_offsets = this->element_value_offsets;
							*element_value_offsets=0;
							m=1;
							while (return_code&&(i<number_of_xi_coordinates))
							{
								xi_coordinate=xi_coordinates[i];
								if (0.0 > xi_coordinate)
								{
									xi_coordinate = 0.0;
								}
								if (xi_coordinate > 1.0)
								{
									xi_coordinate = 1.0;
								}
								/* get xi_offset = lower grid number for cell in xi_coordinate
									 i, and xi_coordinate = fractional xi value in grid cell */
								if (1.0 == xi_coordinate)
								{
									if (number_in_xi[i] > 0)
									{
										xi_offset=number_in_xi[i]-1;
									}
									else
									{
										xi_offset=0;
									}
								}
								else
								{
									xi_coordinate *= (FE_value)(number_in_xi[i]);
									xi_offset=(int)floor((double)xi_coordinate);
									xi_coordinate -= (FE_value)xi_offset;
								}
								offset += xi_offset*this->component_grid_offset_in_xi[this_comp_no][i];
								/* add grid_offset in xi_coordinate i for neighbouring grid
									 points around the linear cell */
								element_value_offset=element_value_offsets;
								for (l=m;l>0;l--)
								{
									element_value_offset[m]=(*element_value_offset)+
										this->component_grid_offset_in_xi[this_comp_no][i];
									element_value_offset++;
								}
								temp = 1.0 - xi_coordinate;
								FE_value *basis_value = basis_values;
								if (derivative)
								{
									// derivatives are w.r.t. element xi, not the grid element xi
									FE_value grid_xi_scaling = static_cast<FE_value>(number_in_xi[i]);
									for (j=1;j<=i;j++)
									{
										basis_value = basis_values + (j*number_of_values + m);
										for (l=m;l>0;l--)
										{
											basis_value--;
											basis_value[m]=(*basis_value)*xi_coordinate;
											*basis_value *= temp;
										}
									}
									j=(i+1)*number_of_values;
									basis_value = basis_values + m;
									for (l=m;l>0;l--)
									{
										basis_value--;
										basis_value[j]= -(*basis_value)*grid_xi_scaling;
										basis_value[j+m]= (*basis_value)*grid_xi_scaling;
									}
								}
								basis_value = basis_values + m;
								for (l=m;l>0;l--)
								{
									basis_value--;
									basis_value[m]=(*basis_value)*xi_coordinate;
									*basis_value *= temp;
								}
								m *= 2;
								i++;
							}
						}
						if (return_code)
						{
							size=get_Value_storage_size(this->field->value_type,
								(struct FE_time_sequence *)NULL);
							component_grid_values_storage=
								this->component_grid_values_storage + this_comp_no;

							element_values_storage=
								(*component_grid_values_storage)+size*offset;
							const FE_value *basis_value = basis_values;
							sum=0;
							element_value_offset=element_value_offsets;
							for (j=number_of_values;j>0;j--)
							{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
								sum += (double)(*basis_value)*(double)(*((FE_value *)(
									element_values_storage+size*(*element_value_offset))));
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								sum += (*basis_value)*(*((FE_value *)(
									element_values_storage+size*(*element_value_offset))));
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
								element_value_offset++;
								basis_value++;
							}
							*calculated_value=(FE_value)sum;
							if (derivative)
							{
								for (k=number_of_xi_coordinates;k>0;k--)
								{
									sum=0;
									element_value_offset=element_value_offsets;
									for (j=number_of_values;j>0;j--)
									{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
										sum += (double)(*basis_value)*(double)(*((FE_value *)(
											element_values_storage+size*(*element_value_offset))));
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										sum += (*basis_value)*(*((FE_value *)(
											element_values_storage+size*(*element_value_offset))));
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
										element_value_offset++;
										basis_value++;
									}
									*derivative=(FE_value)sum;
									derivative++;
								}
							}
						}
					}
					else
					{
						/* standard interpolation */
						number_of_values = *component_number_of_values;
						const FE_value *basis_values = basis_function_values.evaluate(
							*component_standard_basis_function, *component_standard_basis_function_arguments, xi_coordinates);
						if (!basis_values)
						{
							display_message(ERROR_MESSAGE, "FE_element_field_evaluation::evaluate_real.  "
								"Error calculating standard basis");
							return_code = 0;
						}
						/* calculate the element field value as a dot product of the element
							 values and the basis function values */
						const FE_value *basis_value = basis_values;
						element_value= *component_values;
						sum=0;
						for (j=number_of_values;j>0;j--)
						{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
							sum += (double)(*element_value)*(double)(*basis_value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
							sum += (*element_value)*(*basis_value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
							basis_value++;
							element_value++;
						}
						*calculated_value=(FE_value)sum;
						if (derivative)
						{
							for (k=number_of_xi_coordinates;k>0;k--)
							{
								sum=0;
								basis_value = basis_values;
								for (j=number_of_values;j>0;j--)
								{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
									sum += (double)(*element_value)*(double)(*basis_value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									sum += (*element_value)*(*basis_value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
									basis_value++;
									element_value++;
								}
								*derivative=(FE_value)sum;
								derivative++;
							}
						}
					}
					component_number_of_values++;
					component_values++;
					component_standard_basis_function++;
					component_standard_basis_function_arguments++;
					calculated_value++;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"FE_element_field_evaluation::evaluate_real.  Unknown field type");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_field_evaluation::evaluate_real.  Invalid argument(s)\n"
			"xi_coordinates %p, values %p, jacobian %p",
			xi_coordinates,values,jacobian);
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
	const FE_value *xi_coordinates, Standard_basis_function_values &basis_function_values, char **out_string)
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
						basis_function_values, values, /*jacobian*/static_cast<FE_value *>(0)))
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
