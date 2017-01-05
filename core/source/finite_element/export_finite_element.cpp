/*******************************************************************************
FILE : export_finite_element.c

LAST MODIFIED : 16 May 2003

DESCRIPTION :
Functions for exporting finite element data to a file.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "opencmiss/zinc/element.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldgroup.h"
#include "opencmiss/zinc/fieldmodule.h"
#include "opencmiss/zinc/fieldsubobjectgroup.h"
#include "opencmiss/zinc/node.h"
#include "datastore/labels.hpp"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_region.h"
#include "finite_element/export_finite_element.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/list.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "region/cmiss_region_write_info.h"
#include "general/message.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>
using namespace std;

/*
Module types
------------
*/

namespace {

/** Stores offset into packed nodes for each EFT, allowing nodes for several EFTs to
  * be output together iff they share the same nodes in the element
  */
class ElementNodePacking
{
	class EftNodes
	{
	public:
		const int offset;
		const int count;
		std::vector<const FE_element_field_template *> efts;
		const DsLabelIndex *nodeIndexes;

		EftNodes(int offsetIn, const FE_element_field_template *eft, const DsLabelIndex *nodeIndexesIn) :
			offset(offsetIn),
			count(eft->getNumberOfLocalNodes()),
			nodeIndexes(nodeIndexesIn)
		{
			efts.push_back(eft);
		}
	};

	std::vector<EftNodes> eftNodes;

public:

	/** Assumes eft has nodes, and arguments are valid. Don't call if not! */
	void packEftNodes(const FE_element_field_template *eft, const DsLabelIndex *nodeIndexesIn)
	{
		const size_t eftNodesCount = this->eftNodes.size();
		int offset = 0;
		for (size_t i = 0; i < eftNodesCount; ++i)
		{
			EftNodes& thisEftNodes = this->eftNodes[i];
			if (eft->getNumberOfLocalNodes() == thisEftNodes.count)
			{
				const size_t eftCount = thisEftNodes.efts.size();
				for (size_t e = 0; e < eftCount; ++e)
				{
					if (thisEftNodes.efts[e] == eft)
						return;  // already packed this EFT's nodes
				}
				if (0 == memcmp(thisEftNodes.nodeIndexes, nodeIndexesIn, sizeof(DsLabelIndex)*thisEftNodes.count))
				{
					thisEftNodes.efts.push_back(eft);
					return;
				}
			}
			offset += thisEftNodes.count;
		}
		EftNodes newEftNodes(offset, eft, nodeIndexesIn);
		this->eftNodes.push_back(newEftNodes);
	}

	/** @return  Offset starting at 0 into packed nodes array where nodes for EFT
	  * are stored, or -1 if EFT nodes not packed. */
	int getEftNodeOffset(const FE_element_field_template *eft) const
	{
		const size_t eftNodesCount = this->eftNodes.size();
		for (size_t i = 0; i < eftNodesCount; ++i)
		{
			const EftNodes& thisEftNodes = this->eftNodes[i];
			if (eft->getNumberOfLocalNodes() == thisEftNodes.count)
			{
				const size_t eftCount = thisEftNodes.efts.size();
				for (size_t e = 0; e < eftCount; ++e)
				{
					if (thisEftNodes.efts[e] == eft)
						return thisEftNodes.offset;
				}
			}
		}
		return -1;
	}

	/** @return  True if other ElementNodePacking matches the same offsets for all the same EFTs. */
	bool matches(const ElementNodePacking& other) const
	{
		const size_t eftNodesCount = this->eftNodes.size();
		if (other.eftNodes.size() != eftNodesCount)
			return false;
		for (size_t i = 0; i < eftNodesCount; ++i)
		{
			const EftNodes& thisEftNodes = this->eftNodes[i];
			const EftNodes& otherEftNodes = other.eftNodes[i];
			const size_t eftCount = thisEftNodes.efts.size();
			if (otherEftNodes.efts.size() != eftCount)
				return false;
			for (size_t e = 0; e < eftCount; ++e)
			{
				if (otherEftNodes.efts[e] != thisEftNodes.efts[e])
					return false;
			}
		}
		return true;
	}

	int getTotalNodeCount() const
	{
		int totalNodeCount = 0;
		const size_t eftNodesCount = this->eftNodes.size();
		for (size_t i = 0; i < eftNodesCount; ++i)
		{
			const EftNodes& thisEftNodes = this->eftNodes[i];
			totalNodeCount += thisEftNodes.count;
		}
		return totalNodeCount;
	}

	/** @return  First EFT for node block index; iterate from 0 until no EFT returned */
	const FE_element_field_template *getFirstEftAtIndex(size_t index) const
	{
		if (index < this->eftNodes.size())
			return this->eftNodes[index].efts[0];
		return 0;
	}
};

/** Add field to vector, but ensure indexer fields added before any fields they index */
int FE_field_add_to_vector_indexer_priority(struct FE_field *field, void *field_vector_void)
{
	std::vector<FE_field*> *field_vector = static_cast<std::vector<FE_field*> *>(field_vector_void);
	for (auto fieldIter = field_vector->begin(); fieldIter != field_vector->end(); ++fieldIter)
	{
		if (*fieldIter == field)
			return 1; // already present
	}
	if (INDEXED_FE_FIELD == get_FE_field_FE_field_type(field))
	{
		FE_field *indexer_field = 0;
		int number_of_indexed_values = 0;
		get_FE_field_type_indexed(field, &indexer_field, /*ignored*/&number_of_indexed_values);
		if (indexer_field)
			FE_field_add_to_vector_indexer_priority(indexer_field, field_vector_void);
	}
	field_vector->push_back(field);
	return 1;
}

} // anonymous namespace

class WriteEXElements
{
	ostream *output_file;
	int output_number_of_nodes, *output_node_indices,
		output_number_of_scale_factors, *output_scale_factor_indices;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	std::vector<FE_field*> writableFields; // fields to write, from field_order_info, otherwise field manager
	const FE_mesh *mesh;
	struct FE_region *fe_region;
	FE_element_template *elementtemplate;
	FE_value time;
	bool writeGroupOnly;
	// following cached to check whether last field header applies to subsequent elements
	FE_element *headerElement;
	std::vector<FE_field *> headerFields;
	ElementNodePacking *headerElementNodePacking;
	std::vector<const FE_element_field_template *> headerScalingEfts;

public:

	WriteEXElements(ostream *output_fileIn, FE_write_criterion write_criterionIn,
			FE_field_order_info *field_order_infoIn, FE_mesh *meshIn, FE_value timeIn) :
		output_file(output_fileIn),
		output_number_of_nodes(0),
		output_node_indices(0),
		output_number_of_scale_factors(0),
		output_scale_factor_indices(0),
		write_criterion(write_criterionIn),
		field_order_info(field_order_infoIn),
		mesh(meshIn),
		fe_region(this->mesh->get_FE_region()),
		elementtemplate(0),
		time(timeIn),
		writeGroupOnly(false),
		headerElement(0),
		headerElementNodePacking(0)
	{
		// make a list of all fields that should be written, with indexer fields before any indexed fields using them
		if (this->field_order_info)
		{
			const int number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
			for (int f = 0; f < number_of_fields; ++f)
				FE_field_add_to_vector_indexer_priority(get_FE_field_order_info_field(field_order_info, f), (void *)&this->writableFields);
		}
		else
		{
			FE_region_for_each_FE_field(this->fe_region, FE_field_add_to_vector_indexer_priority, (void *)&this->writableFields);
		}
	}

	~WriteEXElements()
	{
		DEALLOCATE(this->output_node_indices);
		DEALLOCATE(this->output_scale_factor_indices);
		delete this->headerElementNodePacking;
	}

	void setWriteGroupOnly()
	{
		this->writeGroupOnly = true;
	}

	bool writeElementExt(cmzn_element *element);

private:
	bool writeElementShape(FE_element_shape *elementShape);
	bool writeBasis(FE_basis *basis);
	bool elementIsToBeWritten(cmzn_element *element);
	bool elementHasFieldsToWrite(cmzn_element *element);
	bool elementFieldsMatchLastElement(cmzn_element *element);
	bool writeElementHeaderField(cmzn_element *element, int fieldIndex, FE_field *field);
	ElementNodePacking *createElementNodePacking(cmzn_element *element);
	bool writeElementHeader(cmzn_element *element);
	bool writeElementFieldValues(cmzn_element *element, FE_field *field, int componentNumber);
	bool writeElement(cmzn_element *element);

};

struct Write_FE_node_field_values
{
	ostream *output_file;
	/* store number of values for writing nodal values in columns */
	int number_of_values;
	FE_value time;
};

struct Write_FE_node_field_info_sub
{
	/* field_number and value_index are incremented by write_FE_node_field so
		 single and multiple field output can be handled appropriately. Both must be
		 initialised to 1 before the first time write_FE_node_field is called */
	int field_number,value_index;
	ostream *output_file;
}; /* Write_FE_node_field_info_sub */

struct Write_FE_region_node_data
{
	ostream *output_file;
	enum FE_write_criterion write_criterion;
	struct FE_field_order_info *field_order_info;
	struct FE_node *last_node;
	FE_value time;
}; /* struct Write_FE_region_node_data */

/*
Module functions
----------------
*/

static int write_element_xi_value(ostream *output_file,struct FE_element *element,
	FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
Writes to <output_file> the element_xi position in the format:
E<lement>/F<ace>/L<ine> ELEMENT_NUMBER DIMENSION xi1 xi2... xiDIMENSION
==============================================================================*/
{
	char element_char;
	int i, return_code;

	ENTER(write_element_xi_value);
	int dimension = get_FE_element_dimension(element);
	if (output_file && (0 < dimension))
	{
		int identifier = get_FE_element_identifier(element);
		if (dimension == 2)
			element_char = 'F';
		else if (dimension == 1)
			element_char = 'L';
		else
			element_char = 'E';
		(*output_file) << " " << element_char << " " <<  identifier << " " << dimension;
		for (i = 0; i < dimension; i++)
		{
			char num_string[100];
			sprintf(num_string, " %" FE_VALUE_STRING, xi[i]);
			(*output_file) << num_string;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_element_xi_value.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_element_xi_value */

static int write_FE_field_header(ostream *output_file,int field_number,
	struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Writes the part of the field header that is common to exnode and exelem files.
Examples:
1) coordinates, coordinate, rectangular cartesian, #Components=3
2) variable, field, indexed, Index_field=bob, #Values=3, real, #Components=1
3) fixed, field, constant, integer, #Components=3
4) an_array, field, real, #Values=10, #Components=1
Value_type ELEMENT_XI_VALUE has optional Mesh Dimension=#.
==============================================================================*/
{
	char *name;
	enum FE_field_type fe_field_type;
	enum Value_type value_type;
	int number_of_components,return_code;
	struct Coordinate_system *coordinate_system;

	ENTER(write_FE_field_header);
	if (output_file&&field)
	{
		(*output_file) << " " << field_number << ") ";
		/* write the field name */
		if (GET_NAME(FE_field)(field,&name))
		{
			(*output_file) << name;
			DEALLOCATE(name);
		}
		else
		{
			(*output_file) << "unknown";
		}
		(*output_file) << ", " << ENUMERATOR_STRING(CM_field_type)(get_FE_field_CM_field_type(field));
		/* optional constant/indexed, Index_field=~, #Values=# */
		fe_field_type=get_FE_field_FE_field_type(field);
		switch (fe_field_type)
		{
			case CONSTANT_FE_FIELD:
			{
				(*output_file) << ", constant";
			} break;
			case GENERAL_FE_FIELD:
			{
				/* default; nothing to write */
			} break;
			case INDEXED_FE_FIELD:
			{
				struct FE_field *indexer_field;
				int number_of_indexed_values;

				(*output_file) << ", indexed, Index_field=";
				if (get_FE_field_type_indexed(field,&indexer_field,
					&number_of_indexed_values))
				{
					if (GET_NAME(FE_field)(indexer_field,&name))
					{
						(*output_file) << name;
						DEALLOCATE(name);
					}
					else
					{
						(*output_file) << "unknown";
					}
					(*output_file) << ", #Values=" << number_of_indexed_values;
				}
				else
				{
					(*output_file) << "unknown, #Values=0";
					display_message(ERROR_MESSAGE,
						"write_FE_field_header.  Invalid indexed field");
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"write_FE_field_header.  Invalid FE_field_type");
			} break;
		}
		if (NULL != (coordinate_system=get_FE_field_coordinate_system(field)))
		{
			switch (coordinate_system->type)
			{
				case CYLINDRICAL_POLAR:
				{
					(*output_file) << ", cylindrical polar";
				} break;
				case FIBRE:
				{
					(*output_file) << ", fibre";
				} break;
				case OBLATE_SPHEROIDAL:
				{
					char num_string[100];
					sprintf(num_string, "%" FE_VALUE_STRING, coordinate_system->parameters.focus);
					(*output_file) << ", oblate spheroidal, focus=" << num_string;
				} break;
				case PROLATE_SPHEROIDAL:
				{
					char num_string[100];
					sprintf(num_string, "%" FE_VALUE_STRING, coordinate_system->parameters.focus);
					(*output_file) << ", prolate spheroidal, focus=" << num_string;
				} break;
				case RECTANGULAR_CARTESIAN:
				{
					(*output_file) << ", rectangular cartesian";
				} break;
				case SPHERICAL_POLAR:
				{
					(*output_file) << ", spherical polar";
				} break;
				default:
				{
					/* write nothing */
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_FE_element_field.  Missing field coordinate system");
		}
		value_type=get_FE_field_value_type(field);
		/* for backwards compatibility with old file formats, if there is a valid
			 coordinate system only write value_type if it is not FE_VALUE_VALUE */
		if ((FE_VALUE_VALUE!=value_type) || (!coordinate_system) ||
			(NOT_APPLICABLE==coordinate_system->type))
		{
			(*output_file) << ", " << Value_type_string(value_type);
		}
		number_of_components=get_FE_field_number_of_components(field);
		(*output_file) << ", #Components=" << number_of_components;
		if (ELEMENT_XI_VALUE == value_type)
		{
			int element_xi_mesh_dimension = FE_field_get_element_xi_mesh_dimension(field);
			if (element_xi_mesh_dimension != 0)
			{
				(*output_file) << "; mesh dimension=" << element_xi_mesh_dimension;
			}
		}
		(*output_file) << "\n";
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_field_header.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_field_header */

static int write_FE_field_values(ostream *output_file,struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Writes the field values to <output_file> - if field is of type constant or
indexed. Each component or version starts on a new line.
==============================================================================*/
{
	enum Value_type value_type;
	int k,number_of_values,return_code;

	ENTER(write_FE_field_values);
	if (output_file&&field)
	{
		return_code=1;
		number_of_values=get_FE_field_number_of_values(field);
		/* only output values for fields with them; ie. not for GENERAL_FE_FIELD */
		if (0<number_of_values)
		{
			value_type=get_FE_field_value_type(field);
			switch (value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
					struct FE_element *element;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_element_xi_value(field,k,&element,xi))
						{
							write_element_xi_value(output_file,element,xi);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting element_xi value");
						}
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value value;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_FE_value_value(field,k,&value))
						{
							char num_string[100];
							sprintf(num_string, "%" FE_VALUE_STRING, value);
							(*output_file) << " " << num_string;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting FE_value");
						}
					}
				} break;
				case INT_VALUE:
				{
					int value;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_int_value(field,k,&value))
						{
							(*output_file) << " " << value;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_FE_field_values.  Error getting int");
						}
					}
				} break;
				case STRING_VALUE:
				{
					char *the_string;

					for (k=0;k<number_of_values;k++)
					{
						if (get_FE_field_string_value(field,k,&the_string))
						{
							if (the_string)
							{
								make_valid_token(&the_string);
								(*output_file) << " " << the_string;
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								(*output_file) << " \"\"";
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_field_values.  Could not get string");
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_FE_field_values.  Value type %s not supported",
						Value_type_string(value_type));
				} break;
			}
			(*output_file) << "\n";
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_field_values */

static int write_FE_node_field_FE_field_values(struct FE_node *node,
	struct FE_field *field,void *output_file_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Iterator function for fields defined at a node. Wrapper for call to
write_FE_field_values.
==============================================================================*/
{
	int return_code;

	ENTER(write_FE_node_field_FE_field_values);
	USE_PARAMETER(node);
	return_code=write_FE_field_values((ostream *)output_file_void,field);
	LEAVE;

	return (return_code);
} /* write_FE_node_field_FE_field_values */

/**
 * Writes out the element shape to stream.
 */
bool WriteEXElements::writeElementShape(FE_element_shape *elementShape)
{
	if (!elementShape)
	{
		display_message(ERROR_MESSAGE,
			"WriteEXElements::writeElementShape.  Invalid argument(s)");
		return false;
	}
	const int dimension = get_FE_element_shape_dimension(elementShape);
	(*this->output_file) << " Shape. Dimension=" << dimension << ", ";
	char *shape_description = FE_element_shape_get_EX_description(elementShape);
	if (!shape_description)
	{
		display_message(ERROR_MESSAGE, "WriteEXElements::writeElementShape.  Invalid shape");
		return false;
	}
	(*this->output_file) << shape_description << "\n";
	DEALLOCATE(shape_description);
	return true;
}

/** Writes out the <basis> to <output_file>. */
bool WriteEXElements::writeBasis(FE_basis *basis)
{
	char *basis_string = FE_basis_get_description_string(basis);
	if (!basis_string)
	{
		display_message(ERROR_MESSAGE, "WriteEXElements::writeBasis.  Invalid basis");
		return false;
	}
	(*this->output_file) << basis_string;
	DEALLOCATE(basis_string);
	return true;
}

/** Write template defining field on element */
bool WriteEXElements::writeElementHeaderField(cmzn_element *element, int fieldIndex, FE_field *field)
{
	if (!write_FE_field_header(this->output_file, fieldIndex, field))
		return false;
	const FE_mesh_field_data *meshFieldData = FE_field_getMeshFieldData(field, this->mesh);
	if (!meshFieldData)
		return false;

	FE_field_type fe_field_type = get_FE_field_FE_field_type(field);
	const int componentCount = get_FE_field_number_of_components(field);
	for (int c = 0; c < componentCount; ++c)
	{
		char *componentName = get_FE_field_component_name(field, c);
		if (componentName)
		{
			(*this->output_file) << " " << componentName << ". ";
			DEALLOCATE(componentName);
		}
		else
		{
			(*this->output_file) << "  " << c + 1 << ".";
		}
		if (fe_field_type != GENERAL_FE_FIELD)
		{
			// constant and indexed fields: no further component information
			(*this->output_file) << "\n";
			continue;
		}

		const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
		const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
		if (!eft)
			return false;
		FE_basis *basis = eft->getBasis();
		this->writeBasis(basis);

		const FE_basis_modify_theta_mode modifyThetaMode = eft->getLegacyModifyThetaMode();
		switch (modifyThetaMode)
		{
		case FE_BASIS_MODIFY_THETA_MODE_INVALID:
			(*this->output_file) << ", no modify";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_CLOSEST_IN_XI1:
			(*this->output_file) << ", closest in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_DECREASING_IN_XI1:
			(*this->output_file) << ", decreasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_INCREASING_IN_XI1:
			(*this->output_file) << ", increasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_NON_DECREASING_IN_XI1:
			(*this->output_file) << ", non-decreasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_NON_INCREASING_IN_XI1:
			(*this->output_file) << ", non-increasing in xi1";
			break;
		}

		const cmzn_element_parameter_mapping_mode parameterMappingMode = eft->getElementParameterMappingMode();
		switch (parameterMappingMode)
		{
		case CMZN_ELEMENT_PARAMETER_MAPPING_MODE_ELEMENT:
		{
			// GRC: split into grid based and element based?
			(*this->output_file) << ", grid based.\n";
			const int *gridNumberInXi = eft->getLegacyGridNumberInXi();
			const int unitGridNumberInXi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 1, 1, 1 };
			if (!gridNumberInXi)
				gridNumberInXi = unitGridNumberInXi;
			(*this->output_file) << " ";
			const int dimension = this->mesh->getDimension();
			for (int d = 0; d < dimension; ++d)
			{
				if (0 < d)
					(*this->output_file) << ", ";
				(*this->output_file) << "#xi" << d + 1 << "=" << gridNumberInXi[d];
			}
			(*this->output_file) << "\n";
			break;
		}
		case CMZN_ELEMENT_PARAMETER_MAPPING_MODE_FIELD:
		{
			(*this->output_file) << ", field based.\n";
			break;
		}
		case CMZN_ELEMENT_PARAMETER_MAPPING_MODE_NODE:
		{
			(*this->output_file) << ", standard node based.";
			// previously the scale factor set was identified by the basis, now it is explicitly named
			if (eft->getNumberOfLocalScaleFactors() > 0)
			{
				int scaleFactorSetIndex = 0;
				for (auto eftIter = this->headerScalingEfts.begin(); eftIter != this->headerScalingEfts.end(); ++eftIter)
				{
					++scaleFactorSetIndex;
					if (*eftIter == eft)
						break;
				}
				(*this->output_file) << " scale factor set=scaling" << scaleFactorSetIndex;
			}
			(*this->output_file) << "\n";

			const int nodeCount = eft->getNumberOfLocalNodes();
			const int packedNodeOffset = this->headerElementNodePacking->getEftNodeOffset(eft);
			(*this->output_file) << "   #Nodes=" << nodeCount << ", offset=" << packedNodeOffset << "\n";
			// previously there was an entry for each basis node with the parameters extracted from it
			// now there is a separate entry for each block of parameters mapped from
			// the same nodes with the same number of terms
			// New: local node indexes are for the EFT, not the packed list output for each element
			const int functionCount = eft->getNumberOfFunctions();
			int f = 0;
			while (f < functionCount)
			{
				(*this->output_file) << "   ";
				const int termCount = eft->getFunctionNumberOfTerms(f);
				int valueCount = 1;
				for (int f2 = f + 1; f2 < functionCount; ++f2)
				{
					if (eft->getFunctionNumberOfTerms(f2) != termCount)
						break;
					int t = 0;
					for (; t < termCount; ++t)
					{
						if (eft->getTermLocalNodeIndex(f2, t) != eft->getTermLocalNodeIndex(f, t))
							break;
					}
					if (t < termCount)
						break;
					++valueCount;
				}
				for (int t = 0; t < termCount; ++t)
				{
					if (t > 0)
						(*this->output_file) << "+";
					(*this->output_file) << eft->getTermLocalNodeIndex(f, t) + 1;
				}
				(*this->output_file) << ". #Values=" << valueCount << "\n";
				// nodal value labels(versions) e.g. d/ds1(2), or the special zero for no terms
				// multi term example: d/ds1(2)+d/ds2
				(*this->output_file) << "     Value labels:";
				for (int v = 0; v < valueCount; ++v)
				{
					++f;
					(*this->output_file) << " ";
					if (termCount == 0)
					{
						(*this->output_file) << "zero";
					}
					else
					{
						for (int t = 0; t < termCount; ++t)
						{
							if (t > 0)
								(*this->output_file) << "+";
							const cmzn_node_value_label nodeValueLabel = eft->getTermNodeValueLabel(f, t);
							const char *valueTypeString = ENUMERATOR_STRING(FE_nodal_value_type)(
								cmzn_node_value_label_to_FE_nodal_value_type(nodeValueLabel));
							(*this->output_file) << valueTypeString;
							const int version = eft->getTermNodeVersion(f, t);
							if (version > 0)
								(*this->output_file) << "(" << version + 1 << ")";
						}
					}
				}
				// New: scale factors only output if there is any scaling for this EFT
				// Also, indexes are for the EFT, not the full list output for each element
				if (eft->getNumberOfLocalScaleFactors() > 0)
				{
					(*this->output_file) << "     Scale factor indices:";
					for (int v = 0; v < valueCount; ++v)
					{
						(*this->output_file) << " ";
						if (termCount == 0)
						{
							(*this->output_file) << "0"; // 0 means unscaled, still required for zero terms
						}
						else
						{
							for (int t = 0; t < termCount; ++t)
							{
								if (t > 0)
									(*this->output_file) << "+";
								const int termScaleFactorCount = eft->getTermScalingCount(f, t);
								if (0 == termScaleFactorCount)
								{
									(*this->output_file) << "0"; // 0 means unscaled
								}
								else
								{
									for (int s = 0; s < termScaleFactorCount; ++s)
									{
										if (s > 0)
											(*this->output_file) << "*";
										const int scaleFactorIndex = eft->getTermScaleFactorIndex(f, t, s);
										(*this->output_file) << scaleFactorIndex + 1;
									}
								}
							}
						}
					}
					(*this->output_file) << "\n";
				}
			}
		}	break;
		case CMZN_ELEMENT_PARAMETER_MAPPING_MODE_INVALID:
		{
			display_message(ERROR_MESSAGE, "WriteEXElements::writeElementHeaderField.  Invalid parameter mapping mode");
			return false;
			break;
		}
		}
	}
	return true;
}

ElementNodePacking *WriteEXElements::createElementNodePacking(cmzn_element *element)
{
	ElementNodePacking *elementNodePacking = new ElementNodePacking();
	const size_t fieldCount = this->writableFields.size();
	for (size_t f = 0; f < fieldCount; ++f)
	{
		FE_field *field = this->writableFields[f];
		const FE_mesh_field_data *meshFieldData = FE_field_getMeshFieldData(field, this->mesh);
		if (!meshFieldData)
			continue;
		const int componentCount = get_FE_field_number_of_components(field);
		const FE_element_field_template *lastEft = 0;
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
			const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
			if (!eft)
				break; // field not defined on this element
			if (eft != lastEft)
			{
				if (eft->getNumberOfLocalNodes() > 0)
				{
					const FE_mesh_element_field_template_data *meshEftData = this->mesh->getElementfieldtemplateData(eft);
					const DsLabelIndex *nodeIndexes = meshEftData->getElementNodeIndexes(element->getIndex());
					if (!nodeIndexes)
					{
						display_message(ERROR_MESSAGE, "WriteEXElements::createElementNodePacking.  Missing node indexes");
						delete elementNodePacking;
						return 0;
					}
					elementNodePacking->packEftNodes(eft, nodeIndexes);
				}
				lastEft = eft;
			}
		}
	}
	return elementNodePacking;
}

/**
 * Writes the element field information header for element. If the
 * <field_order_info> is supplied the header is restricted to include only
 * components/fields/bases for it. To handle both this and the all-field case, the
 * function builds up and returns the following:
 * - output_number_of_nodes and output_node_indices for reducing the number of
 *   nodes output if not all nodes in element are used for the field. In the array
 *   of indices, -1 indicates that the node is not used, otherwise the new index
 *   into the output node list is recorded.
 * - output_number_of_scale_factors and output_scale_factor_indices for reducing
 *   the number of scale_factors output. In the array of indices, -1 indicates
 *   that the scale_factor is not used, otherwise the new index into the output
 *   scale_factor list is recorded.
 * The indices arrays are reallocated here, so should be NULL the first time they
 * are passed to this function.
 */
bool WriteEXElements::writeElementHeader(cmzn_element *element)
{
	if (!element)
	{
		display_message(ERROR_MESSAGE,
			"WriteEXElements::writeElementHeader.  Invalid argument(s)");
		return false;
	}

	this->headerElement = element;
	this->headerFields.clear();
	delete this->headerElementNodePacking;
	this->headerElementNodePacking = new ElementNodePacking();
	this->headerScalingEfts.clear();

	// 1. Make list of fields to output, order of EFT scale factor sets, node packing
	bool writeFieldValues = false;
	for (auto fieldIter = this->writableFields.begin(); fieldIter != this->writableFields.end(); ++fieldIter)
	{
		FE_field *field = *fieldIter;
		const FE_mesh_field_data *meshFieldData = FE_field_getMeshFieldData(field, this->mesh);
		if (!meshFieldData)
			continue; // field not defined on mesh
		const int componentCount = get_FE_field_number_of_components(field);
		const FE_element_field_template *lastEft = 0;
		int c = 0;
		for (; c < componentCount; ++c)
		{
			const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
			const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
			if (!eft)
				break; // field not defined on this element
			if (eft != lastEft)
			{
				if (eft->getNumberOfLocalNodes() > 0)
				{
					const FE_mesh_element_field_template_data *meshEftData = this->mesh->getElementfieldtemplateData(eft);
					const DsLabelIndex *nodeIndexes = meshEftData->getElementNodeIndexes(element->getIndex());
					if (!nodeIndexes)
					{
						display_message(ERROR_MESSAGE, "WriteEXElements::writeElementHeader.  Missing node indexes");
						return false;
					}
					this->headerElementNodePacking->packEftNodes(eft, nodeIndexes);
				}
				if (eft->getNumberOfLocalScaleFactors() > 0)
				{
					lastEft = eft;
					auto eftIter = this->headerScalingEfts.begin();
					for (; eftIter != this->headerScalingEfts.end(); ++eftIter)
					{
						if (*eftIter == eft)
							break;
					}
					if (eftIter == this->headerScalingEfts.end())
						this->headerScalingEfts.push_back(eft);
				}
			}
		}
		if (c == componentCount)
		{
			this->headerFields.push_back(field);
			// in future: this should be per-component:
			if (CONSTANT_FE_FIELD == get_FE_field_FE_field_type(field))
				writeFieldValues = true;
		}
	}

	(*this->output_file) << " #Scale factor sets=" << this->headerScalingEfts.size() << "\n";
	int scaleFactorSetIndex = 0;
	for (auto eftIter = this->headerScalingEfts.begin(); eftIter != this->headerScalingEfts.end(); ++eftIter)
	{
		++scaleFactorSetIndex;
		(*this->output_file) << "  ";
		(*this->output_file) << "scaling" << scaleFactorSetIndex << ", #Scale factors=" << (*eftIter)->getNumberOfLocalScaleFactors() << "\n";
	}
	(*this->output_file) << " #Nodes=" << this->headerElementNodePacking->getTotalNodeCount() << "\n";
	(*this->output_file) << " #Fields=" << this->headerFields.size() << "\n";
	int fieldIndex = 0;
	for (auto fieldIter = this->headerFields.begin(); fieldIter != this->headerFields.end(); ++fieldIter)
	{
		++fieldIndex;
		FE_field *field = *fieldIter;
		if (!this->writeElementHeaderField(element, fieldIndex, field))
			return false;
	}

	// write globally constant field values:
	// future: change to write only values for constant components
	if (writeFieldValues)
	{
		(*this->output_file) << " Values :\n";
		for (auto fieldIter = this->headerFields.begin(); fieldIter != this->headerFields.end(); ++fieldIter)
		{
			FE_field *field = *fieldIter;
			if (CONSTANT_FE_FIELD == get_FE_field_FE_field_type(field))
			{
				if (!write_FE_field_values(this->output_file, field))
					return false;
			}
		}
	}
	return true;
}

bool WriteEXElements::writeElementFieldValues(cmzn_element *element,
	FE_field *field, int componentNumber)
{
	const FE_mesh_field_data *meshFieldData = FE_field_getMeshFieldData(field, this->mesh);
	const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(componentNumber);
	const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
	const int valueCount = eft->getNumberOfElementDOFs();
	FE_mesh_field_data::ComponentBase *componentBase = meshFieldData->getComponentBase(componentNumber);
	Value_type valueType = get_FE_field_value_type(field);

	const int *gridNumberInXi = eft->getLegacyGridNumberInXi();
	// have new line every number-of-grid-points-in-xi1
	const int columnCount = (gridNumberInXi) ? gridNumberInXi[0] + 1 : valueCount;

	switch (valueType)
	{
	case FE_VALUE_VALUE:
	{
		auto component = static_cast<FE_mesh_field_data::Component<FE_value>*>(componentBase);
		const FE_value *values = component->getElementValues(element->getIndex(), valueCount);
		if (!values)
		{
			display_message(ERROR_MESSAGE, "WriteEXElements::writeElementFieldValues.  Missing real values");
			return false;
		}
		char tmpString[100];
		for (int v = 0; v < valueCount; ++v)
		{
			sprintf(tmpString, " %" FE_VALUE_STRING, values[v]);
			(*this->output_file) << tmpString;
			if (0 == ((v + 1) % columnCount))
				(*this->output_file) << "\n";
		}
		// extra newline if not multiple of number_of_columns
		if (0 != (valueCount % columnCount))
			(*this->output_file) << "\n";
		break;
	}
	case INT_VALUE:
	{
		auto component = static_cast<FE_mesh_field_data::Component<int>*>(componentBase);
		const int *values = component->getElementValues(element->getIndex(), valueCount);
		if (!values)
		{
			display_message(ERROR_MESSAGE, "WriteEXElements::writeElementFieldValues.  Missing int values");
			return false;
		}
		for (int v = 0; v < valueCount; ++v)
		{
			(*this->output_file) << values[v];
			if (0 == ((v + 1) % columnCount))
				(*this->output_file) << "\n";
		}
		// extra newline if not multiple of number_of_columns
		if (0 != (valueCount % columnCount))
			(*this->output_file) << "\n";
		break;
	}
	default:
	{
		display_message(ERROR_MESSAGE,
			"write_FE_element_field_values.  Unsupported value type %s", Value_type_string(valueType));
		return false;
	} break;
	}
	return true;
}

/**
 * Writes out an element to stream in format:

Element: 1
  Faces:
  1 2 3 4 5 6
  Values:
  1 2 4 16 32 64 128 6 7 8 7 8 9 8 9 10 9 10 11 10 11 12 7 8 9 8 9 10 9 10 11 10
  11 12 11 12 13 8 9 10 9 10 11 10 11 12 11 12 13 12 13 14 9 10 11 10 11 12 11 12
  13 12 13 14 13 14 15
  Nodes:
  1 2 3 4 5 6 7 8
  Scale factors:
  0.100000E+01  0.100000E+01  0.100000E+01  0.100000E+01  0.100000E+01
  0.100000E+01  0.100000E+01  0.100000E+01

 * Notes:
 * Faces are only output for elements with the faces array allocated. Missing
 * faces are given the identifier -1 as expected by read_FE_element.
 * Values, Nodes and Scale Factors are only output if present for element fields.
 */
bool WriteEXElements::writeElement(cmzn_element *element)
{
	(*this->output_file) << "Element: " << element->getIdentifier() << "\n";

	if (this->writeGroupOnly)
		return true;

	// Faces: if defined
	const FE_mesh::ElementShapeFaces *elementShapeFaces = this->mesh->getElementShapeFacesConst(element->getIndex());
	if (!elementShapeFaces)
	{
		display_message(ERROR_MESSAGE, "WriteEXElements::writeElement.  Missing ElementShapeFaces");
		return false;
	}
	FE_mesh *faceMesh = this->mesh->getFaceMesh();
	if (faceMesh)
	{
		const int faceCount = elementShapeFaces->getFaceCount();
		const DsLabelIndex *faceIndexes;
		if ((0 < faceCount) && (faceIndexes = elementShapeFaces->getElementFaces(element->getIndex())))
		{
			(*this->output_file) << " Faces:\n";
			for (int i = 0; i < faceCount; ++i)
			{
				// now writing -1 if face not defined, since 0 is a valid identifier
				DsLabelIdentifier elementIdentifier = (faceIndexes[i] >= 0) ?
					faceMesh->getElementIdentifier(faceIndexes[i]) : -1;
				(*this->output_file) << " " << elementIdentifier;
			}
			(*this->output_file) << "\n";
		}
	}

	// Values: if writing any element-based fields
	bool firstElementBasedField = true;
	for (auto fieldIter = this->headerFields.begin(); fieldIter != this->headerFields.end(); ++fieldIter)
	{
		FE_field *field = *fieldIter;
		if (GENERAL_FE_FIELD != get_FE_field_FE_field_type(field))
			continue;
		const FE_mesh_field_data *meshFieldData = FE_field_getMeshFieldData(field, this->mesh);
		const int componentCount = get_FE_field_number_of_components(field);
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
			const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
			if (eft->getNumberOfElementDOFs() > 0)
			{
				if (firstElementBasedField)
				{
					(*this->output_file) << " Values :\n";
					firstElementBasedField = false;
				}
				if (!this->writeElementFieldValues(element, field, c))
					return false;
			}
		}
	}

	// Nodes: if any
	if (this->headerElementNodePacking->getTotalNodeCount() > 0)
	{
		FE_nodeset *nodeset = this->mesh->getNodeset();
		(*this->output_file) << " Nodes:\n";
		int index = 0;
		const FE_element_field_template *eft;
		while (0 != (eft = this->headerElementNodePacking->getFirstEftAtIndex(index)))
		{
			const FE_mesh_element_field_template_data *meshEftData = this->mesh->getElementfieldtemplateData(eft);
			const DsLabelIndex *nodeIndexes = meshEftData->getElementNodeIndexes(element->getIndex());
			const int nodeCount = eft->getNumberOfLocalNodes();
			for (int n = 0; n < nodeCount; ++n)
				(*this->output_file) << " " << nodeset->getNodeIdentifier(nodeIndexes[n]);
			++index;
		}
		(*this->output_file) << "\n";
	}

	// Scale factors: if any scale factor sets being output
	if (this->headerScalingEfts.size() > 0)
	{
		int scaleFactorNumber = 0;
		char tmpString[100];
		(*this->output_file) << " Scale factors:\n";
		for (auto eftIter = this->headerScalingEfts.begin(); eftIter != this->headerScalingEfts.end(); ++eftIter)
		{
			const FE_element_field_template *eft = *eftIter;
			const FE_mesh_element_field_template_data *meshEftData = this->mesh->getElementfieldtemplateData(eft);
			const FE_value *values = meshEftData->getElementScaleFactors(element->getIndex());
			if (!values)
			{
				display_message(ERROR_MESSAGE, "WriteEXElements::writeElement.  Missing scale factors");
				return false;
			}
			const int scaleFactorCount = eft->getNumberOfLocalScaleFactors();
			for (int s = 0; s < scaleFactorCount; ++s)
			{
				++scaleFactorNumber;
				sprintf(tmpString, "%" FE_VALUE_STRING, values[s]);
				(*this->output_file) << " " << tmpString;
				if ((0 < FE_VALUE_MAX_OUTPUT_COLUMNS)
					&& (0 == (scaleFactorNumber % FE_VALUE_MAX_OUTPUT_COLUMNS)))
				{
					(*this->output_file) << "\n";
				}
			}
			// extra new line if not multiple of FE_VALUE_MAX_OUTPUT_COLUMNS values
			if ((FE_VALUE_MAX_OUTPUT_COLUMNS <= 0)
				|| (0 != (scaleFactorNumber % FE_VALUE_MAX_OUTPUT_COLUMNS)))
			{
				(*this->output_file) << "\n";
			}
		}
	}
	return true;
}

/**
 * @return  True if specification of what to output, optionally including the
 * field order info, indicates the <element> is to be written.
 */
bool WriteEXElements::elementIsToBeWritten(cmzn_element *element)
{
	if (!((element) && ((this->write_criterion == FE_WRITE_COMPLETE_GROUP) || (this->field_order_info))))
	{
		display_message(ERROR_MESSAGE,
			"WriteEXElements::elementIsToBeWritten.  Invalid argument(s)");
		return false;
	}
	switch (this->write_criterion)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			const int number_of_fields = get_FE_field_order_info_number_of_fields(this->field_order_info);
			for (int i = 0; i < number_of_fields; ++i)
			{
				struct FE_field *field = get_FE_field_order_info_field(this->field_order_info, i);
				if (!FE_field_is_defined_in_element(field, element))
					return false;
			}
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			const int number_of_fields = get_FE_field_order_info_number_of_fields(this->field_order_info);
			for (int i = 0; i < number_of_fields; ++i)
			{
				struct FE_field *field = get_FE_field_order_info_field(this->field_order_info, i);
				if (FE_field_is_defined_in_element(field, element))
					return true;
			}
			return false;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"WriteEXElements::elementIsToBeWritten.  Unknown write_criterion");
			return false;
		} break;
	}
	return true;
}

/** @return true if any fields are to be written for the element. */
bool WriteEXElements::elementHasFieldsToWrite(cmzn_element *element)
{
	if (!this->field_order_info)
		return (0 < get_FE_element_number_of_fields(element));
	const int number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
	for (int i = 0; i < number_of_fields; ++i)
	{
		struct FE_field *field = get_FE_field_order_info_field(field_order_info, i);
		if (FE_field_is_defined_in_element_not_inherited(field, element))
			return true;
	}
	return false;
}

/**
 * Returns true if element has matching field definition to last element whose
 * field header was output, or if there was no last element.
 * If field_order_info is supplied only those fields listed in it are compared.
 * Note that even if this returns true, need to check element nodes are
 * packed identically for fields written together.
 */
bool WriteEXElements::elementFieldsMatchLastElement(cmzn_element *element)
{
	if (!this->headerElement)
		return false;
	if (!this->field_order_info)
		return (0 != equivalent_FE_fields_in_elements(element, this->headerElement));
	const int number_of_fields = get_FE_field_order_info_number_of_fields(field_order_info);
	for (int f = 0; f < number_of_fields; ++f)
	{
		struct FE_field *field = get_FE_field_order_info_field(field_order_info, f);
		if (!equivalent_FE_field_in_elements(field, element, this->headerElement))
			return false;
	}
	return true;
}

/**
 * Writes element to the stream. If the element template is different from the
 * header element then a new header is written out. If the element
 * has no fields, only the shape is output in the header.
 */
bool WriteEXElements::writeElementExt(cmzn_element *element)
{
	if (!element)
	{
		display_message(ERROR_MESSAGE,
			"WriteEXElements::writeElementExt.  Invalid argument(s)");
		return false;
	}
	if (!this->elementIsToBeWritten(element))
		return true;
	// work out if shape or field header have changed from last element
	FE_element_shape *elementShape = element->getElementShape();
	if (!elementShape)
		return false;
	bool newShape = true;
	bool newFieldHeader = true;
	if (this->headerElement)
	{
		newShape = (elementShape != this->headerElement->getElementShape());
		if (newShape)
		{
			newFieldHeader = this->elementHasFieldsToWrite(element);
		}
		else
		{
			newFieldHeader = !this->WriteEXElements::elementFieldsMatchLastElement(element);
			if (!newFieldHeader)
			{
				ElementNodePacking *elementNodePacking = this->createElementNodePacking(element);
				newFieldHeader = !elementNodePacking->matches(*this->headerElementNodePacking);
				delete elementNodePacking;
			}
		}
	}
	if (newShape && (!this->writeElementShape(elementShape)))
		return false;
	if (newFieldHeader && !this->writeElementHeader(element))
		return false;
	if (!this->writeElement(element))
		return false;
	return true;
}

static int write_FE_node_field(ostream *output_file,int field_number,
	struct FE_node *node,struct FE_field *field,int *value_index)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Writes a node field to an <output_file>. After the field header is written with
write_FE_field_header, value index, derivative and version info are output on
following lines, one per component:
  COMPONENT_NAME.  Value_index=~, #Derivatives=~ (NAMES), #Versions=~
???RC Added value index - which should be initialised to 1 before the first
time this function is called - this function adds on
(#derivatives+1)*(#versions) to it for each component output.
==============================================================================*/
{
	char *component_name;
	enum FE_field_type fe_field_type;
	enum FE_nodal_value_type *nodal_value_types;
	int i,j,number_of_components,number_of_derivatives,number_of_versions,
		return_code;

	ENTER(write_FE_node_field);
	return_code=0;
	if (output_file&&node&&field&&value_index)
	{
		write_FE_field_header(output_file,field_number,field);
		fe_field_type=get_FE_field_FE_field_type(field);
		number_of_components=get_FE_field_number_of_components(field);
		for (i=0;i<number_of_components;i++)
		{
			if (NULL != (component_name=get_FE_field_component_name(field,i)))
			{
				(*output_file) << "  "<< component_name << ".";
				DEALLOCATE(component_name);
			}
			else
			{
				(*output_file) << "  "<< i+1 << ".";
			}
			if (GENERAL_FE_FIELD==fe_field_type)
			{
				number_of_derivatives=
				get_FE_node_field_component_number_of_derivatives(node,field,i);
				number_of_versions=
				get_FE_node_field_component_number_of_versions(node,field,i);
				(*output_file) << "  Value index=" << *value_index << ", #Derivatives=" << number_of_derivatives;
				if (0<number_of_derivatives)
				{
					if (NULL != (nodal_value_types=
						get_FE_node_field_component_nodal_value_types(node,field,i)))
					{
						(*output_file) << " (";
						for (j=1;j<1+number_of_derivatives;j++)
						{
							if (1!=j)
							{
								(*output_file) << ",";
							}
							(*output_file) << ENUMERATOR_STRING(FE_nodal_value_type)(nodal_value_types[j]);
						}
						(*output_file) << ")";
						DEALLOCATE(nodal_value_types);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"write_FE_node_field.  Could not get nodal value types");
					}
				}
				(*output_file) << ", #Versions=" << number_of_versions << "\n";
				(*value_index) += number_of_versions*(1+number_of_derivatives);
			}
			else
			{
				/* constant and indexed fields: 1 version, no derivatives */
				(*output_file) << "\n";
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_node_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field */

static int write_FE_node_field_info_sub(struct FE_node *node,
	struct FE_field *field,void *write_nodes_data_void)
/*******************************************************************************
LAST MODIFIED : 20 September 1999

DESCRIPTION :
Calls the write_FE_node_field routine for each FE_node_field
==============================================================================*/
{
	int return_code;
	struct Write_FE_node_field_info_sub *write_nodes_data;

	ENTER(write_FE_node_field_info_sub);
	if (NULL != (write_nodes_data=(struct Write_FE_node_field_info_sub *)write_nodes_data_void))
	{
		return_code=write_FE_node_field(write_nodes_data->output_file,
			write_nodes_data->field_number,node,field,&(write_nodes_data->value_index));
		write_nodes_data->field_number++;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field_info_sub */

static int write_FE_node_field_values(struct FE_node *node,
	struct FE_field *field,void *values_data_void)
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Writes out the nodal values. Each component or version starts on a new line.
==============================================================================*/
{
	enum FE_field_type fe_field_type;
	enum Value_type value_type;
	ostream *output_file;
	int i,j,k,number_of_components,number_of_derivatives,number_of_values,
		number_of_versions,return_code;
	struct Write_FE_node_field_values *values_data;

	ENTER(write_FE_node_field_values);
	values_data = (struct Write_FE_node_field_values *)values_data_void;
	output_file = values_data->output_file;
	if (node && field && (NULL != values_data) && (NULL != output_file))
	{
		fe_field_type=get_FE_field_FE_field_type(field);
		if (GENERAL_FE_FIELD==fe_field_type)
		{
			number_of_components=get_FE_field_number_of_components(field);
			value_type=get_FE_field_value_type(field);
			switch (value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
					struct FE_element *element;

					/* only allow as many values as components */
					for (i=0;i<number_of_components;i++)
					{
						if (get_FE_nodal_element_xi_value(node,field,/*component_number*/i,
							/*version*/0,FE_NODAL_VALUE,&element,xi))
						{
							write_element_xi_value(output_file,element,xi);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_node_field_values.  Could not get element_xi value");
						}
						(*output_file) << "\n";
					}
				} break;
				case FE_VALUE_VALUE:
				{
					FE_value *value,*values;
					if (get_FE_nodal_field_FE_value_values(field,node,&number_of_values,
							values_data->time, &values))
					{
						value=values;
						for (i=0;i<number_of_components;i++)
						{
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,field,i);
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,field,i);
							for (j=number_of_versions;0<j;j--)
							{
								for (k=0;k<=number_of_derivatives;k++)
								{
									char num_string[100];
									sprintf(num_string, "%" FE_VALUE_STRING, *value);

									(*output_file) << " " << num_string;
									value++;
								}
								(*output_file) << "\n";
							}
						}
						DEALLOCATE(values);
					}
				} break;
				case INT_VALUE:
				{
					int *value,*values;

					if (get_FE_nodal_field_int_values(field,node,&number_of_values,
							values_data->time, &values))
					{
						value=values;
						for (i=0;i<number_of_components;i++)
						{
							number_of_derivatives=
								get_FE_node_field_component_number_of_derivatives(node,field,i);
							number_of_versions=
								get_FE_node_field_component_number_of_versions(node,field,i);
							for (j=number_of_versions;0<j;j--)
							{
								for (k=0;k<=number_of_derivatives;k++)
								{
									(*output_file) << " " << *value;
									value++;
								}
								(*output_file) << "\n";
							}
						}
						DEALLOCATE(values);
					}
				} break;
				case STRING_VALUE:
				{
					char *the_string;

					/* only allow as many values as components */
					for (i=0;i<number_of_components;i++)
					{
						if (get_FE_nodal_string_value(node,field,/*component_number*/i,
							/*version*/0,FE_NODAL_VALUE,&the_string))
						{
							if (the_string)
							{
								make_valid_token(&the_string);
								(*output_file) << " " << the_string;
								DEALLOCATE(the_string);
							}
							else
							{
								/* empty string */
								(*output_file) << " \"\"";
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"write_FE_node_field_values.  Could not get string");
						}
						(*output_file) << "\n";
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"write_FE_node_field_values.  Value type %s not supported",
						Value_type_string(value_type));
				} break;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_node_field_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node_field_values */

static int write_FE_node(ostream *output_file,struct FE_node *node,
	struct FE_field_order_info *field_order_info, FE_value time)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Writes out a node to an <output_file>. Unless <field_order_info> is non-NULL and
contains the fields to be written if defined, all fields defined at the node are
written.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct Write_FE_node_field_values values_data;

	ENTER(write_FE_node);
	if (output_file && node)
	{
		(*output_file) << " Node: " << get_FE_node_identifier(node) << "\n";
		values_data.output_file = output_file;
		values_data.number_of_values = 0;
		values_data.time = time;
		if (field_order_info)
		{
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i = 0; i < number_of_fields; i++)
			{
				if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
					FE_field_is_defined_at_node(field, node))
				{
					for_FE_field_at_node(field, write_FE_node_field_values,
						(void *)&values_data, node);
				}
			}
		}
		else
		{
			for_each_FE_field_at_node_alphabetical_indexer_priority(write_FE_node_field_values,
				(void *)&values_data,node);
		}
		/* add extra carriage return for not multiple of
			 FE_VALUE_MAX_OUTPUT_COLUMNS values */
		if ((0 < values_data.number_of_values) &&
			((0 >= FE_VALUE_MAX_OUTPUT_COLUMNS) ||
				(0 != (values_data.number_of_values % FE_VALUE_MAX_OUTPUT_COLUMNS))))
		{
			(*output_file) << "\n";
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_FE_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_FE_node */

static int FE_node_passes_write_criterion(struct FE_node *node,
	enum FE_write_criterion write_criterion,
	struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 6 September 2001

DESCRIPTION :
Returns true if the <write_criterion> -- some options of which require the
<field_order_info> -- indicates the <node> is to be written.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_node_passes_write_criterion);
	switch (write_criterion)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			return_code = 1;
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			if (node && field_order_info && (0 < (number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 1;
				for (i = 0; (i < number_of_fields) && return_code; i++)
				{
					if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_field_is_defined_at_node(field, node)))
					{
						return_code = 0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			if (node && field_order_info && (0 < (number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info))))
			{
				return_code = 0;
				for (i = 0; (i < number_of_fields) && (!return_code); i++)
				{
					if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
						FE_field_is_defined_at_node(field, node))
					{
						return_code = 1;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_node_passes_write_criterion.  Invalid argument(s)");
				return_code = 0;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_node_passes_write_criterion.  Unknown write_criterion");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* FE_node_passes_write_criterion */

static int FE_nodes_have_same_header(struct FE_node *node_1,
	struct FE_node *node_2, struct FE_field_order_info *field_order_info)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if <node_1> and <node_2> can be written to file with the
same fields header. If <field_order_info> is supplied only those fields
listed in it are compared.
==============================================================================*/
{
	int i, number_of_fields, return_code;
	struct FE_field *field;

	ENTER(FE_nodes_have_same_header);
	if (node_1 && node_2)
	{
		if (field_order_info)
		{
			return_code = 1;
			number_of_fields =
				get_FE_field_order_info_number_of_fields(field_order_info);
			for (i = 0; (i < number_of_fields) && return_code; i++)
			{
				if (!((field = get_FE_field_order_info_field(field_order_info, i)) &&
					equivalent_FE_field_at_nodes(field, node_1, node_2)))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = equivalent_FE_fields_at_nodes(node_1, node_2);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_nodes_have_same_header.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_nodes_have_same_header */

static int write_FE_region_node(struct FE_node *node,
	Write_FE_region_node_data *write_nodes_data)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Writes a node to the given file.  If the fields defined at the node are
different from the last node (taking into account whether a selection of fields
has been selected for output) then the header is written out.
==============================================================================*/
{
	ostream *output_file;
	int i, number_of_fields = 0, number_of_fields_in_header, return_code,
		write_field_values;
	struct FE_field *field;
	struct FE_field_order_info *field_order_info;
	struct Write_FE_node_field_info_sub field_data;

	ENTER(write_FE_region_node);
	if (node && write_nodes_data && (0 != (output_file = write_nodes_data->output_file)))
	{
		return_code = 1;
		field_order_info = write_nodes_data->field_order_info;
		/* write this node? */
		if (FE_node_passes_write_criterion(node, write_nodes_data->write_criterion,
			field_order_info))
		{
			/* need to write new header? */
			if (((struct FE_node *)NULL == write_nodes_data->last_node) ||
				(!FE_nodes_have_same_header(node, write_nodes_data->last_node,
					field_order_info)))
			{
				/* get number of fields in header */
				if (field_order_info)
				{
					number_of_fields_in_header = 0;
					write_field_values = 0;
					number_of_fields = get_FE_field_order_info_number_of_fields(
						field_order_info);
					for (i = 0; i < number_of_fields; i++)
					{
						if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
							FE_field_is_defined_at_node(field, node))
						{
							number_of_fields_in_header++;
							if (0 < get_FE_field_number_of_values(field))
							{
								write_field_values = 1;
							}
						}
					}
				}
				else
				{
					number_of_fields_in_header = get_FE_node_number_of_fields(node);
					write_field_values = FE_node_has_FE_field_values(node);
				}
				(*output_file) << " #Fields=" << number_of_fields_in_header << "\n";
				field_data.field_number = 1;
				field_data.value_index = 1;
				field_data.output_file = output_file;
				if (field_order_info)
				{
					for (i = 0; i < number_of_fields; i++)
					{
						if ((field = get_FE_field_order_info_field(field_order_info, i)) &&
							FE_field_is_defined_at_node(field, node))
						{
							for_FE_field_at_node(field, write_FE_node_field_info_sub,
								&field_data, node);
						}
					}
				}
				else
				{
					for_each_FE_field_at_node_alphabetical_indexer_priority(write_FE_node_field_info_sub,
						&field_data, node);
				}
				if (write_field_values)
				{
					/* write values for constant and indexed fields */
					(*output_file) << " Values :\n";
					if (field_order_info)
					{
						for (i = 0; i < number_of_fields; i++)
						{
							if ((field = get_FE_field_order_info_field(field_order_info, i))
								&& FE_field_is_defined_at_node(field, node) &&
								(0 < get_FE_field_number_of_values(field)))
							{
								for_FE_field_at_node(field, write_FE_node_field_FE_field_values,
									(void *)output_file, node);
							}
						}
					}
					else
					{
						for_each_FE_field_at_node_alphabetical_indexer_priority(
							write_FE_node_field_FE_field_values, (void *)output_file, node);
					}
				}
			}
			write_FE_node(output_file, node, field_order_info, write_nodes_data->time);
			/* remember the last node to check if header needs to be re-output */
			write_nodes_data->last_node = node;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_FE_region_node.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static int write_cmzn_region_content(ostream *output_file,
	struct cmzn_region *region, cmzn_field_group_id group,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, int *field_names_counter,
	FE_value time, enum FE_write_criterion write_criterion,
	bool writeGroupOnly = false)
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Writes <base_fe_region> to the <output_file>.
If <field_order_info> is NULL, all element fields are written in the default,
alphabetical order.
If <field_order_info> is empty, only element identifiers are output.
If <field_order_info> contains fields, they are written in that order.
Additionally, the <write_criterion> controls output as follows:
FE_WRITE_COMPLETE_GROUP = write all elements in the group (the default);
FE_WRITE_WITH_ALL_LISTED_FIELDS =
  write only elements with all listed fields defined;
FE_WRITE_WITH_ANY_LISTED_FIELDS =
  write only elements with any listed fields defined.
==============================================================================*/
{
	int return_code;

	ENTER(write_cmzn_region_content);
	if (output_file && region)
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
		FE_region *fe_region = cmzn_region_get_FE_region(region);
		return_code = 1;
		FE_field_order_info *field_order_info = CREATE(FE_field_order_info)();

		if (write_fields_mode != FE_WRITE_NO_FIELDS)
		{
			if (write_fields_mode == FE_WRITE_ALL_FIELDS)
			{
				// get list of all fields in default alphabetical order
				return_code = FE_region_for_each_FE_field(fe_region,
					FE_field_add_to_FE_field_order_info, (void *)field_order_info);
				FE_field_order_info_prioritise_indexer_fields(field_order_info);
			}
			else if ((0 < number_of_field_names) && field_names && field_names_counter)
			{
				for (int i = 0; (i < number_of_field_names) && return_code; i++)
				{
					if (field_names[i])
					{
						struct FE_field *fe_field = FE_region_get_FE_field_from_name(fe_region, field_names[i]);
						if (fe_field)
						{
							++(field_names_counter[i]);
							return_code = add_FE_field_order_info_field(field_order_info, fe_field);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE, "write_cmzn_region_content.  NULL field name");
						return_code = 0;
					}
				}
			}
			else if (0 < number_of_field_names)
			{
				display_message(ERROR_MESSAGE, "write_cmzn_region_content.  Missing field names array");
				return_code = 0;
			}
		}

		if (return_code)
		{
			// write nodes then elements then data last since future plan is to remove the feature
			// where the same field can be defined simultaneously on nodes & elements and also data.
			// To migrate the first one will use the actual field name and the other will need to
			// qualify it, and we want the qualified one to be the datapoints.
			if (write_nodes)
			{
				Write_FE_region_node_data write_nodes_data;
				write_nodes_data.output_file = output_file;
				write_nodes_data.write_criterion = write_criterion;
				write_nodes_data.field_order_info = field_order_info;
				write_nodes_data.last_node = (struct FE_node *)NULL;
				write_nodes_data.time = time;
				write_nodes_data.last_node = (struct FE_node *)NULL;
				cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
					CMZN_FIELD_DOMAIN_TYPE_NODES);
				if (group)
				{
					cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
					cmzn_nodeset_destroy(&nodeset);
					nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
					cmzn_field_node_group_destroy(&node_group);
				}
				if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
				{
					(*output_file) << " !#nodeset nodes\n";
					cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
					cmzn_node_id node = 0;
					while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
					{
						if (!write_FE_region_node(node, &write_nodes_data))
						{
							return_code = 0;
							break;
						}
					}
					cmzn_nodeiterator_destroy(&iter);
					cmzn_nodeset_destroy(&nodeset);
				}
			}
			if (write_elements)
			{
				const int highest_dimension = FE_region_get_highest_dimension(fe_region);
				/* write 1-D, 2-D then 3-D so lines and faces precede elements */
				for (int dimension = 1; dimension <= highest_dimension; dimension++)
				{
					if ((dimension == 1 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH1D)) ||
						(dimension == 2 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH2D)) ||
						(dimension == 3 && (write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH3D)) ||
						(dimension == highest_dimension &&
						(write_elements & CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION)))
					{
						cmzn_mesh_id mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
						if (group)
						{
							cmzn_field_element_group_id element_group = cmzn_field_group_get_field_element_group(group, mesh);
							cmzn_mesh_destroy(&mesh);
							mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
							cmzn_field_element_group_destroy(&element_group);
						}
						if (mesh)
						{
							WriteEXElements writeElementsData(output_file, write_criterion, field_order_info,
								FE_region_find_FE_mesh_by_dimension(fe_region, dimension), time);
							if (writeGroupOnly)
								writeElementsData.setWriteGroupOnly();
							cmzn_elementiterator_id iter = cmzn_mesh_create_elementiterator(mesh);
							cmzn_element_id element = 0;
							while (0 != (element = cmzn_elementiterator_next_non_access(iter)))
							{
								if (!writeElementsData.writeElementExt(element))
								{
									return_code = 0;
									break;
								}
							}
							cmzn_elementiterator_destroy(&iter);
							cmzn_mesh_destroy(&mesh);
						}
					}
				}
			}
			if (write_data)
			{
				Write_FE_region_node_data write_nodes_data;
				write_nodes_data.output_file = output_file;
				write_nodes_data.write_criterion = write_criterion;
				write_nodes_data.field_order_info = field_order_info;
				write_nodes_data.last_node = (struct FE_node *)NULL;
				write_nodes_data.time = time;
				cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(field_module,
					CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS);
				if (group)
				{
					cmzn_field_node_group_id node_group = cmzn_field_group_get_field_node_group(group, nodeset);
					cmzn_nodeset_destroy(&nodeset);
					nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
					cmzn_field_node_group_destroy(&node_group);
				}
				if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
				{
					(*output_file) << " !#nodeset datapoints\n";
					cmzn_nodeiterator_id iter = cmzn_nodeset_create_nodeiterator(nodeset);
					cmzn_node_id node = 0;
					while (0 != (node = cmzn_nodeiterator_next_non_access(iter)))
					{
						if (!write_FE_region_node(node, &write_nodes_data))
						{
							return_code = 0;
							break;
						}
					}
					cmzn_nodeiterator_destroy(&iter);
					cmzn_nodeset_destroy(&nodeset);
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "write_cmzn_region_content.  Failed");
			return_code = 0;
		}
		if (field_order_info)
		{
			DESTROY(FE_field_order_info)(&field_order_info);
		}
		cmzn_fieldmodule_destroy(&field_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_cmzn_region_content.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Recursively writes the finite element fields in region tree to file.
 * Notes:
 * - the master region for a group must be the parent region.
 * - element_xi values currently restricted to being in the root_region.
 * 
 * @param output_file  The file to write region and field data to.
 * @param region  The region to write.
 * @param parent_region_output  Pointer to parent region if just output to same
 *   file. Caller should set to NULL on first call.
 * @param root_region  The root region of any data to be written. Need not be
 *   the true root of region hierarchy, but region paths in file are relative to
 *   this region.
 * @param write_elements  If set, write elements and element fields to file.
 * @param write_nodes  If set, write nodes and node fields to file.
 * @param write_data  If set, write data and data fields to file. May only use
 *   if write_elements and write_nodes are 0.
 * @param write_fields_mode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then:
 *   - Number/list of field_names must be supplied;
 *   - Field names not used in a region are ignored;
 *   - Warnings are given for any field names not used in any output region.
 * @param number_of_field_names  The number of names in the field_names array.
 * @param field_names  Array of field names.
 * @param field_names_counter  Array of integers of same length as field_names
 *   incremented whenever the respective field name is matched.
 * @param write_criterion  Controls which objects are written. Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param write_recursion  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
static int write_cmzn_region(ostream *output_file,
	struct cmzn_region *region, const char * group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, int *field_names_counter,
	FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	ENTER(write_cmzn_region);
	if (output_file && region && root_region)
	{
		return_code = 1;

		// write region path and/or group name */
		cmzn_field_group_id group = 0;
		if (group_name)
		{
			cmzn_fieldmodule_id fieldmodule =  cmzn_region_get_fieldmodule(region);
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(fieldmodule, group_name);
			if (field)
			{
				group = cmzn_field_cast_group(field);
				cmzn_field_destroy(&field);
			}
			cmzn_fieldmodule_destroy(&fieldmodule);
		}

		if (!group_name || group)
		{
			if (!group || (region != root_region))
			{
				char *region_path = cmzn_region_get_relative_path(region, root_region);
				size_t len = strlen(region_path);
				if ((1 < len) && (region_path[len - 1] == CMZN_REGION_PATH_SEPARATOR_CHAR))
				{
					region_path[len - 1] = '\0';
				}
				(*output_file) << "Region: " << region_path << "\n";
				DEALLOCATE(region_path);
			}
			if (group)
			{
				char *group_name = cmzn_field_get_name(cmzn_field_group_base_cast(group));
				(*output_file) << " Group name: " << group_name << "\n";
				DEALLOCATE(group_name);
			}

			// write finite element fields for this region
			if (return_code)
			{
				return_code = write_cmzn_region_content(output_file, region, group,
					write_elements, write_nodes, write_data,
					write_fields_mode, number_of_field_names, field_names,
					field_names_counter, time, write_criterion);
			}
		}

		if (return_code && !group_name && recursion_mode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)
		{
			// write group members
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fielditerator_id field_iter = cmzn_fieldmodule_create_fielditerator(field_module);
			cmzn_field_id field = 0;
			while ((0 != (field = cmzn_fielditerator_next_non_access(field_iter))) && return_code)
			{
				cmzn_field_group_id output_group = cmzn_field_cast_group(field);
				if (output_group)
				{
					char *group_name = cmzn_field_get_name(field);
					(*output_file) << " Group name: " << group_name << "\n";
					DEALLOCATE(group_name);
					return_code = write_cmzn_region_content(output_file, region, output_group,
						write_elements, write_nodes, write_data,
						FE_WRITE_NO_FIELDS, number_of_field_names, field_names,
						field_names_counter, time, write_criterion, /*writeGroupOnly*/true);
					cmzn_field_group_destroy(&output_group);
				}
			}
			cmzn_fielditerator_destroy(&field_iter);
			cmzn_fieldmodule_destroy(&field_module);
		}

		if (recursion_mode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)
		{
			// write child regions
			cmzn_region *child_region = cmzn_region_get_first_child(region);
			while (child_region)
			{
				return_code = write_cmzn_region(output_file,
					child_region, group_name, root_region,
					write_elements, write_nodes, write_data,
					write_fields_mode, number_of_field_names, field_names,
					field_names_counter, time, write_criterion, recursion_mode);
				if (!return_code)
				{
					cmzn_region_destroy(&child_region);
					break;
				}
				cmzn_region_reaccess_next_sibling(&child_region);
			}
		}
		if (group)
		{
			cmzn_field_group_destroy(&group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "write_cmzn_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_cmzn_region */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_write_criterion)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_write_criterion));
	switch (enumerator_value)
	{
		case FE_WRITE_COMPLETE_GROUP:
		{
			enumerator_string = "complete_group";
		} break;
		case FE_WRITE_WITH_ALL_LISTED_FIELDS:
		{
			enumerator_string = "with_all_listed_fields";
		} break;
		case FE_WRITE_WITH_ANY_LISTED_FIELDS:
		{
			enumerator_string = "with_any_listed_fields";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_write_criterion) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_write_criterion)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(FE_write_recursion)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(FE_write_recursion));
	switch (enumerator_value)
	{
		case FE_WRITE_RECURSIVE:
		{
			enumerator_string = "recursive";
		} break;
		case FE_WRITE_RECURSE_SUBGROUPS:
		{
			enumerator_string = "recurse_subgroups";
		} break;
		case FE_WRITE_NON_RECURSIVE:
		{
			enumerator_string = "non_recursive";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(FE_write_recursion) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(FE_write_recursion)

/***************************************************************************//**
 * Writes an EX file with supplied root_region at the top level of the file.
 *
 * @param output_file  The file to write region and field data to.
 * @param region  The region to output.
 * @param group  Optional subgroup to output.
 * @param root_region  The region which will become the root region in the EX
 *   file. Need not be the true root of region hierarchy, but must contain
 *   <region>.
 * @param write_elements  If set, write elements and element fields to file.
 * @param write_nodes  If set, write nodes and node fields to file.
 * @param write_data  If set, write data and data fields to file. May only use
 *   if write_elements and write_nodes are 0.
 * @param write_fields_mode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then:
 *   - Number/list of field_names must be supplied;
 *   - Field names not used in a region are ignored;
 *   - Warnings are given for any field names not used in any output region.
 * @param number_of_field_names  The number of names in the field_names array.
 * @param field_names  Array of field names.
 * @param time  Field values at <time> will be written out if field is time
 *    dependent. If fields are time dependent but <time> is out of range then
 *    the values at nearest time will be written out. If fields are not time
 *    dependent, this parameter is ignored.
 * @param write_criterion  Controls which objects are written. Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param write_recursion  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
int write_exregion_to_stream(ostream *output_file,
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	ENTER(write_exregion_to_stream);
	if (output_file && region && root_region &&
		((write_data || write_elements || write_nodes)) &&
		((write_fields_mode != FE_WRITE_LISTED_FIELDS) ||
			((0 < number_of_field_names) && field_names)))
	{
		if (cmzn_region_contains_subregion(root_region, region))
		{
			int *field_names_counter = NULL;
			if (0 < number_of_field_names)
			{
				/* count number of times each field name is matched for later warning */
				if (ALLOCATE(field_names_counter, int, number_of_field_names))
				{
					for (int i = 0; i < number_of_field_names; i++)
					{
						field_names_counter[i] = 0;
					}
				}
			}
			return_code = write_cmzn_region(output_file,
				region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, field_names_counter,
				time, write_criterion, recursion_mode);
			if (field_names_counter)
			{
				if (write_fields_mode == FE_WRITE_LISTED_FIELDS)
				{
					for (int i = 0; i < number_of_field_names; i++)
					{
						if (field_names_counter[i] == 0)
						{
							display_message(WARNING_MESSAGE,
								"No field named '%s' found in any region written to EX file",
								field_names[i]);
						}
					}
				}
				DEALLOCATE(field_names_counter);
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"write_exregion_to_stream.  Error writing region");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"write_exregion_to_stream.  Region is not within root region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_to_stream.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_exregion_to_stream */

int write_exregion_file_of_name(const char *file_name,
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region,
	int write_elements, int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode)
{
	int return_code;

	if (file_name)
	{
		ofstream output_file;
		output_file.open(file_name, ios::out);
		if (output_file.is_open())
		{
			return_code = write_exregion_to_stream(&output_file, region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, time,
				write_criterion, recursion_mode);
			output_file.close();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open for writing exregion file: %s", file_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_file_of_name.  Invalid arguments");
		return_code = 0;
	}

	return (return_code);
} /* write_exregion_file_of_name */

int write_exregion_file_to_memory_block(
	struct cmzn_region *region, const char *group_name,
	struct cmzn_region *root_region, int write_elements,
	int write_nodes, int write_data,
	enum FE_write_fields_mode write_fields_mode,
	int number_of_field_names, char **field_names, FE_value time,
	enum FE_write_criterion write_criterion,
	enum cmzn_streaminformation_region_recursion_mode recursion_mode,
	void **memory_block, unsigned int *memory_block_length)
{
	int return_code;

	ENTER(write_exregion_file_of_name);
	if (memory_block)
	{
		ostringstream stringStream;
		if (stringStream)
		{
			return_code = write_exregion_to_stream(&stringStream, region, group_name, root_region,
				write_elements, write_nodes, write_data,
				write_fields_mode, number_of_field_names, field_names, time,
				write_criterion, recursion_mode);
			string sstring = stringStream.str();
			*memory_block_length = static_cast<unsigned int>(sstring.size());
			*memory_block = duplicate_string(sstring.c_str());
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open for writing exregion into memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_file_of_name.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
