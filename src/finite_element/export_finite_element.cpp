/**
 * FILE : export_finite_element.cpp
 *
 * Functions for exporting finite element data to EX format.
 */
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
#include "opencmiss/zinc/mesh.h"
#include "opencmiss/zinc/node.h"
#include "opencmiss/zinc/nodeset.h"
#include "opencmiss/zinc/region.h"
#include "datastore/labels.hpp"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_nodeset.hpp"
#include "finite_element/finite_element_private.h"
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
		int offset;  // actually constant, but can't make const to use default assignment and copy constructor
		int count;  // actually constant, but can't make const to use default assignment and copy constructor
		std::vector<const FE_element_field_template *> efts;
		const DsLabelIndex *nodeIndexes;  // can be NULL if no eft nodes set yet

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

	ElementNodePacking(cmzn_element *element, vector<FE_field*>& writableFields)
	{
		const size_t fieldCount = writableFields.size();
		for (size_t f = 0; f < fieldCount; ++f)
		{
			FE_field *field = writableFields[f];
			const FE_mesh_field_data *meshFieldData = field->getMeshFieldData(element->getMesh());
			if (!meshFieldData)
			{
				continue;  // field not defined on this mesh
			}
			const int componentCount = get_FE_field_number_of_components(field);
			const FE_element_field_template *lastEft = nullptr;
			for (int c = 0; c < componentCount; ++c)
			{
				const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
				const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
				if (!eft)
				{
					break;  // field not defined on this element
				}
				if (eft != lastEft)
				{
					if (eft->getNumberOfLocalNodes() > 0)
					{
						const FE_mesh_element_field_template_data *meshEftData = element->getMesh()->getElementfieldtemplateData(eft);
						const DsLabelIndex *nodeIndexes = meshEftData->getElementNodeIndexes(element->getIndex());
						if (!nodeIndexes)
						{
							display_message(WARNING_MESSAGE, "EXWriter::createElementNodePacking.  Missing node indexes");
						}
						this->packEftNodes(eft, nodeIndexes);
					}
					lastEft = eft;
				}
			}
		}
	}

	/** @param  nodeIndexesIn  The node indexes for eft; can be NULL if not set */
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
				if ((nodeIndexesIn) && (thisEftNodes.nodeIndexes)
					&& (0 == memcmp(thisEftNodes.nodeIndexes, nodeIndexesIn, sizeof(DsLabelIndex)*thisEftNodes.count)))
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

/** Class for writing region/field data to EX format */
class EXWriter
{
	class TimeSequence
	{
	public:
		std::string name;
		FE_time_sequence *feTimeSequence;
		int size;
		const bool singleTimeSet;
		const FE_value singleTime;

		TimeSequence(const char *nameIn, FE_time_sequence *feTimeSequenceIn, bool singleTimeSetIn, FE_value singleTimeIn) :
			name(nameIn),
			feTimeSequence(ACCESS(FE_time_sequence)(feTimeSequenceIn)),
			size(FE_time_sequence_get_number_of_times(feTimeSequenceIn)),
			singleTimeSet(singleTimeSetIn),
			singleTime(singleTimeIn)
		{
			if ((singleTimeSetIn) && (size > 0))
			{
				size = 1;
			}
		}

		~TimeSequence()
		{
			DEACCESS(FE_time_sequence)(&this->feTimeSequence);
		}

		/** @return  Non-accessed FE_time_sequence */
		FE_time_sequence *getFeTimeSequence() const
		{
			return this->feTimeSequence;
		}

		const char *getName() const
		{
			return this->name.c_str();
		}

		int getSize() const
		{
			return this->size;
		}
	};

	class NodeTemplate
	{
	public:
		std::string name;
		cmzn_node *headerNode;
		std::vector<FE_field *> headerFields;

		NodeTemplate(const char *nameIn, cmzn_node *headerNodeIn) :
			name(nameIn),
			headerNode(headerNodeIn->access())
		{
		}

		~NodeTemplate()
		{
			cmzn_node::deaccess(this->headerNode);
			this->headerFields.clear();
		}

		/**
		 * Returns true if node has matching field definition to this template.
		 * Limited to matching writable fields defined on this node.
		 */
		bool matches(cmzn_node *node, FE_write_fields_mode writeFieldsMode, vector<FE_field*>& writableFields)
		{
			if (writeFieldsMode == FE_WRITE_ALL_FIELDS)
			{
				return (0 != equivalent_FE_fields_at_nodes(node, this->headerNode));
			}
			const size_t fieldsCount = writableFields.size();
			for (size_t i = 0; i < fieldsCount; ++i)
			{
				if (!equivalent_FE_field_at_nodes(writableFields[i], node, this->headerNode))
				{
					return false;
				}
			}
			return true;
		}

	};
	
	class ElementTemplate
	{
	public:
		std::string name;
		cmzn_element *headerElement;
		std::vector<FE_field *> headerFields;
		ElementNodePacking headerElementNodePacking;
		std::vector<const FE_element_field_template *> headerScalingEfts;

		ElementTemplate(const char *nameIn, cmzn_element *headerElementIn, vector<FE_field*>& writableFields) :
			name(nameIn),
			headerElement(headerElementIn->access()),
			headerElementNodePacking(headerElement, writableFields)
		{
		}

		~ElementTemplate()
		{
			cmzn_element::deaccess(this->headerElement);
		}

		/**
		 * Returns true if element has matching shape and field definition to this template.
		 * Also matches node packing which is pessimistic; would only fail if say 2 efts
		 * have the same nodes for one element, but different for another, all else equal.
		 * Limited to matching writable fields defined on this element.
		 */
		bool matches(cmzn_element *element, FE_write_fields_mode writeFieldsMode, vector<FE_field*>& writableFields)
		{
			if (element->getElementShape() != this->headerElement->getElementShape())
			{
				return false;
			}
			if (writeFieldsMode == FE_WRITE_ALL_FIELDS)
			{
				return equivalent_FE_fields_in_elements(element, this->headerElement);
			}
			const size_t fieldsCount = writableFields.size();
			for (size_t i = 0; i < fieldsCount; ++i)
			{
				if (!equivalent_FE_field_in_elements(writableFields[i], element, this->headerElement))
				{
					return false;
				}
			}
			ElementNodePacking tmpElementNodePacking(element, writableFields);
			if (!tmpElementNodePacking.matches(this->headerElementNodePacking))
			{
				return false;
			}
			return true;
		}

	};

	ostream *outStream;
	cmzn_region *rootRegion;  // accessed
	const char * groupName;
	bool singleTimeSet;
	FE_value singleTime;
	cmzn_field_domain_types writeDomainTypes;  // sets which meshes, nodesets to write
	FE_write_fields_mode writeFieldsMode;  // sets whether all/no/listed fields are written
	std::vector<std::string> fieldNames;
	std::vector<int> fieldNamesCounters;  // number of times a named field is written
	FE_write_criterion writeCriterion;
	cmzn_streaminformation_region_recursion_mode recursionMode;

	cmzn_region *region;  // not accessed
	FE_region *feRegion;
	cmzn_fieldmodule *fieldmodule;
	std::vector<FE_field *> writableFields;  // fields in region requested to be written in indexer-priority order
	const FE_mesh *feMesh;
	const FE_nodeset *feNodeset;
	int timeSequenceNumber;  // incremented and used to make time sequence name unique across file
	std::vector<TimeSequence *> timeSequences;  // time sequences defined in current region
	int nodeTemplateNumber;  // incremented and used to make node template name unique across file
	std::vector<NodeTemplate*> nodeTemplates;
	NodeTemplate *nodeTemplate;
	int elementTemplateNumber;  // incremented and used to make element template name unique across file
	std::vector<ElementTemplate*> elementTemplates;
	ElementTemplate *elementTemplate;

public:

	/** Constructor for object for writing region/field data to EX format.
	 * @param outStreamIn  The stream to write region and field data to.
	 * @param rootRegionIn  The root region of any data to be written. Need not be
	 *   the true root of region hierarchy, but region paths in file are relative to
	 *   this region.
	 * @param groupNameIn  Optional name of group to limit output to. Actual
	 *   group found from name in each region.
	 * @param singleTimeSetIn  True if output single time, false to output all times.
	 * @param singleTimeIn  The time to output if single time.
	 * @param writeDomainTypesIn  Bitwise OR of cmzn_field_domain_type flags
	 *   setting which meshes or nodesets to write.
	 * @param writeFieldsModeIn  Controls which fields are written to file.
	 *   If mode is FE_WRITE_LISTED_FIELDS then :
	 * - Number/list of field_names must be supplied;
	 * - Field names not used in a region are ignored;
	 * - Warnings are given for any field names not used in any output region.
	 * @param fieldNamesCountIn  The number of names in the field_names array.
	 * @param fieldNamesIn  Array of field names.
	 * @param writeCriterionIn  Controls which objects are written.Some modes
	 *   limit output to nodes or objects with any or all listed fields defined.
	 * @param recursionModeIn  Controls whether sub-regions and sub-groups are
	 *   recursively written.
	 */
	EXWriter(ostream *outStreamIn, cmzn_region *rootRegionIn,
			const char *groupNameIn, bool singleTimeSetIn, FE_value singleTimeIn,
			cmzn_field_domain_types writeDomainTypesIn,
			FE_write_fields_mode writeFieldsModeIn,
			int fieldNamesCountIn, const char * const *fieldNamesIn,
			FE_write_criterion writeCriterionIn,
			cmzn_streaminformation_region_recursion_mode recursionModeIn) :
		outStream(outStreamIn),
		rootRegion(rootRegionIn->access()),
		groupName((groupNameIn) ? duplicate_string(groupNameIn) : nullptr),
		singleTimeSet(singleTimeSetIn),
		singleTime(singleTimeIn),
		writeDomainTypes(writeDomainTypesIn),
		writeFieldsMode(writeFieldsModeIn),
		fieldNames(),
		fieldNamesCounters(fieldNamesCountIn, 0),
		writeCriterion(writeCriterionIn),
		recursionMode(recursionModeIn),
		region(nullptr),
		feRegion(nullptr),
		fieldmodule(nullptr),
		feMesh(nullptr),
		feNodeset(nullptr),
		timeSequenceNumber(0),
		nodeTemplateNumber(0),
		nodeTemplate(nullptr),
		elementTemplateNumber(0),
		elementTemplate(nullptr)
	{
		if (fieldNamesIn)
		{
			for (int i = 0; i < fieldNamesCountIn; ++i)
			{
				this->fieldNames.push_back(fieldNamesIn[i]);
			}
		}
	}

	~EXWriter()
	{
		this->clearTimeSequences();
		DEALLOCATE(this->groupName);
		this->clearTemplates();
		if (this->fieldmodule)
		{
			cmzn_fieldmodule_destroy(&this->fieldmodule);
		}
		cmzn_region::deaccess(this->rootRegion);
	}

	/** Write the region/tree to stream 
	 * @param regionIn  The base region to write, can be anywhere in tree under root region */
	int write(cmzn_region *regionIn);

private:

	void clearTimeSequences()
	{
		const size_t tsCount = this->timeSequences.size();
		for (size_t ts = 0; ts < tsCount; ++ts)
			delete this->timeSequences[ts];
		this->timeSequences.clear();
	}

	void clearTemplates()
	{
		const size_t ntCount = this->nodeTemplates.size();
		for (size_t nt = 0; nt < ntCount; ++nt)
		{
			delete this->nodeTemplates[nt];
		}
		this->nodeTemplates.clear();
		this->nodeTemplate = nullptr;
		const size_t etCount = this->elementTemplates.size();
		for (size_t et = 0; et < etCount; ++et)
		{
			delete this->elementTemplates[et];
		}
		this->elementTemplates.clear();
		this->elementTemplate = nullptr;
	}

	/** Switch to writing elements from mesh */
	void setMesh(const FE_mesh *feMeshIn)
	{
		this->feMesh = feMeshIn;
		this->feNodeset = nullptr;
		this->clearTemplates();
	}

	/** Switch to writing nodes from nodeset */
	void setNodeset(const FE_nodeset *nodesetIn)
	{
		this->feMesh = nullptr;
		this->feNodeset = nodesetIn;
		this->clearTemplates();
	}

	/** Output name, automatically quoting if contains special characters */
	void writeSafeName(const char *name) const
	{
		char *safeName = duplicate_string(name);
		make_valid_token(&safeName);
		(*this->outStream) << safeName;
		DEALLOCATE(safeName);
	}

	bool writeElementXiValue(const FE_mesh *hostMesh, DsLabelIndex elementIndex, const FE_value *xi);
	bool writeFieldHeader(int fieldIndex, struct FE_field *field, TimeSequence *timeSequence=nullptr);
	bool writeFieldValues(struct FE_field *field);
	bool writeOptionalFieldValues(vector<FE_field*>& headerFields);

	TimeSequence *findTimeSequence(FE_time_sequence *feTimeSequence);
	bool writeTimeSequence(FE_time_sequence *feTimeSequence);

	bool writeElementShape(FE_element_shape *elementShape);
	bool writeEftScaleFactorIdentifiers(const FE_element_field_template *eft);
	bool writeBasis(FE_basis *basis);
	bool writeElementHeaderField(cmzn_element *element, int fieldIndex, FE_field *field);
	bool writeElementTemplate(cmzn_element *element);
	bool writeElementFieldComponentValues(cmzn_element *element, FE_field *field, int componentNumber);
	bool writeElement(cmzn_element *element);
	bool elementIsToBeWritten(cmzn_element *element);
	bool writeFeMesh(FE_mesh *feMeshIn);
	bool writeMesh(int dimension, cmzn_field_group *group);
	bool writeElementGroup(int dimension, cmzn_field_group *group);

	bool writeNodeHeaderField(cmzn_node *node, int fieldIndex, FE_field *field);
	bool writeNodeTemplate(cmzn_node *node);
	bool writeNodeFieldValues(cmzn_node *node, FE_field *field);
	bool writeNode(cmzn_node *node);
	bool nodeIsToBeWritten(cmzn_node *node);
	bool writeFeNodeset(FE_nodeset *feNodesetIn);
	bool writeNodeset(cmzn_field_domain_type fieldDomainType, cmzn_field_group *group);
	bool writeNodeGroup(cmzn_field_domain_type fieldDomainType, cmzn_field_group *group);

	int writeRegionContent(cmzn_field_group *group);
	bool writeGroup(cmzn_field_group *group);
	int writeRegion(cmzn_region *regionIn);
};

/*
Module functions
----------------
*/

/**
 * Writes to output_file the element_xi position in the format:
 * ELEMENT_IDENTIFIER xi1 xi2... xi(DIMENSION)
 * If there is no element it writes:
 * -1 0 0... xi(DIMENSION)
 * This new format requires embedded locations be within one mesh set
 * with the element:xi field.
 */
bool EXWriter::writeElementXiValue(const FE_mesh *hostMesh, DsLabelIndex elementIndex, const FE_value *xi)
{
	if (!((hostMesh) && (xi)))
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeElementXiValue.  Invalid argument(s)");
		return false;
	}
	DsLabelIdentifier elementIdentifier = hostMesh->getElementIdentifier(elementIndex);
	(*this->outStream) << " " << elementIdentifier;
	for (int d = 0; d < hostMesh->getDimension(); ++d)
	{
		if (elementIdentifier < 0)
		{
			(*this->outStream) << " 0";
		}
		else
		{
			char num_string[100];
			sprintf(num_string, " %" FE_VALUE_STRING, xi[d]);
			(*this->outStream) << num_string;
		}
	}
	return true;
}

/**
 * Writes the part of the field header that is common for nodes and elements.
 * Examples:
 * 1) coordinates, coordinate, rectangular cartesian, #Components=3
 * 2) variable, field, indexed, Index_field=bob, #Values=3, real, #Components=1
 * 3) fixed, field, constant, integer, #Components=3
 * 4) an_array, field, real, #Values=10, #Components=1
 * Value_type ELEMENT_XI_VALUE has optional Mesh Dimension=#.
 * @param timeSequence  If supplied (not default nullptr) adds: , time sequence=NAME
 */
bool EXWriter::writeFieldHeader(int fieldIndex, struct FE_field *field, TimeSequence *timeSequence)
{
	(*this->outStream) << fieldIndex << ") " << get_FE_field_name(field);
	(*this->outStream) << ", " << ENUMERATOR_STRING(CM_field_type)(field->get_CM_field_type());
	/* optional constant/indexed, Index_field=~, #Values=# */
	FE_field_type fe_field_type = get_FE_field_FE_field_type(field);
	switch (fe_field_type)
	{
	case CONSTANT_FE_FIELD:
	{
		(*this->outStream) << ", constant";
	} break;
	case GENERAL_FE_FIELD:
	{
		/* default; nothing to write */
	} break;
	case INDEXED_FE_FIELD:
	{
		struct FE_field *indexer_field;
		int number_of_indexed_values;
		if (!get_FE_field_type_indexed(field, &indexer_field, &number_of_indexed_values))
		{
			display_message(ERROR_MESSAGE, "EXWriter::writeFieldHeader.  Invalid indexed field");
			return false;
		}
		(*this->outStream) <<  ", indexed, Index_field=" << get_FE_field_name(indexer_field) << ", #Values=" << number_of_indexed_values;
	} break;
	default:
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeFieldHeader.  Invalid FE_field_type");
		return false;
	} break;
	}
	const Coordinate_system& coordinate_system = field->getCoordinateSystem();
	switch (coordinate_system.type)
	{
		case CYLINDRICAL_POLAR:
		{
			(*this->outStream) << ", cylindrical polar";
		} break;
		case FIBRE:
		{
			(*this->outStream) << ", fibre";
		} break;
		case OBLATE_SPHEROIDAL:
		{
			char num_string[100];
			sprintf(num_string, "%" FE_VALUE_STRING, coordinate_system.parameters.focus);
			(*this->outStream) << ", oblate spheroidal, focus=" << num_string;
		} break;
		case PROLATE_SPHEROIDAL:
		{
			char num_string[100];
			sprintf(num_string, "%" FE_VALUE_STRING, coordinate_system.parameters.focus);
			(*this->outStream) << ", prolate spheroidal, focus=" << num_string;
		} break;
		case RECTANGULAR_CARTESIAN:
		{
			(*this->outStream) << ", rectangular cartesian";
		} break;
		case SPHERICAL_POLAR:
		{
			(*this->outStream) << ", spherical polar";
		} break;
		case NOT_APPLICABLE:
		{
			/* write nothing */
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"EXWriter::writeFieldHeader.  Unknown coordinate system type: ignoring");
			/* write nothing */
		} break;
	}

	// In EX Versions < 2, value type was optional if coordinate system output for field
	// Since reader always handled it if present or not, now write it always
	Value_type valueType = get_FE_field_value_type(field);
	(*this->outStream) << ", " << Value_type_string(valueType);

	const int componentCount = get_FE_field_number_of_components(field);
	(*this->outStream) << ", #Components=" << componentCount;
	if (ELEMENT_XI_VALUE == valueType)
	{
		const FE_mesh *hostMesh = field->getElementXiHostMesh();
		if (!hostMesh)
		{
			display_message(ERROR_MESSAGE, "EXWriter::writeFieldHeader.  Missing host mesh for element xi field");
			return false;
		}
		char *hostMeshName = duplicate_string(hostMesh->getName());
		make_valid_token(&hostMeshName);
		(*this->outStream) << ", host mesh=" << hostMeshName << ", host mesh dimension=" << hostMesh->getDimension();
		DEALLOCATE(hostMeshName);
	}

	if (timeSequence)
	{
		(*this->outStream) << ", time sequence=" << timeSequence->getName();
	}

	(*this->outStream) << "\n";
	return true;
}

/**
  * Writes the field values to <output_file> - if field is of type constant or
  * indexed. Each component or version starts on a new line.
  */
bool EXWriter::writeFieldValues(struct FE_field *field)
{
	if (!field)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeFieldValues.  Missing field");
		return false;
	}
	
	const int number_of_values = field->getNumberOfValues();
	/* only output values for fields with them; ie. not for GENERAL_FE_FIELD */
	if (0<number_of_values)
	{
		Value_type valueType = get_FE_field_value_type(field);
		switch (valueType)
		{
			case FE_VALUE_VALUE:
			{
				char num_string[100];
				const FE_value *fieldValues = field->getRealValues();
				if (!fieldValues)
				{
					display_message(ERROR_MESSAGE, "EXWriter::writeFieldValues.  Error getting field FE_values");
					return false;
				}
				for (int k=0;k<number_of_values;k++)
				{
					sprintf(num_string, "%" FE_VALUE_STRING, fieldValues[k]);
					(*this->outStream) << " " << num_string;
				}
			} break;
			case INT_VALUE:
			{
				const int *fieldValues = field->getIntValues();
				if (!fieldValues)
				{
					display_message(ERROR_MESSAGE, "EXWriter::writeFieldValues.  Error getting field ints");
					return false;
				}
				for (int k=0;k<number_of_values;k++)
				{
					(*this->outStream) << " " << fieldValues[k];
				}
			} break;
			case STRING_VALUE:
			{
				const char **fieldValues = field->getStringValues();
				if (!fieldValues)
				{
					display_message(ERROR_MESSAGE, "EXWriter::writeFieldValues.  Could not get string values");
					return false;
				}
				for (int k=0;k<number_of_values;k++)
				{
					if (fieldValues[k])
					{
						char *s = duplicate_string(fieldValues[k]);
						make_valid_token(&s);
						(*this->outStream) << " " << s;
						DEALLOCATE(s);
					}
					else
					{
						/* output empty string */
						(*this->outStream) << " \"\"";
					}
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "EXWriter::writeFieldValues.  Value type %s not supported",
					Value_type_string(valueType));
				return false;
			} break;
		}
		(*this->outStream) << "\n";
	}
	return true;
}

/** If any fields have per-field parameters, e.g. is constant or indexed, writes a Values section
  * future: change to support having only some components constant; probably remove indexed */
bool EXWriter::writeOptionalFieldValues(vector<FE_field*>& headerFields)
{
	bool first = true;
	for (auto fieldIter = headerFields.begin(); fieldIter != headerFields.end(); ++fieldIter)
	{
		FE_field *field = *fieldIter;
		const FE_field_type feFieldType = get_FE_field_FE_field_type(field);
		if ((feFieldType == CONSTANT_FE_FIELD) || (feFieldType == INDEXED_FE_FIELD))
		{
			if (first)
			{
				(*this->outStream) << "Values:\n";
				first = false;
			}
			if (!this->writeFieldValues(field))
				return false;
		}
	}
	return true;
}

/** @return  Pointer to TimeSequence with FE_time_sequence, or nullptr if not found */
EXWriter::TimeSequence *EXWriter::findTimeSequence(FE_time_sequence *feTimeSequence)
{
	const size_t tsCount = this->timeSequences.size();
	if (this->singleTimeSet)
	{
		// only ever one time sequence when singleTimeSet
		if (tsCount > 0)
		{
			return this->timeSequences[0];
		}
		return nullptr;
	}
	for (size_t ts = 0; ts < tsCount; ++ts)
	{
		if (this->timeSequences[ts]->getFeTimeSequence() == feTimeSequence)
		{
			return this->timeSequences[ts];
		}
	}
	return nullptr;
}

/** Write/store TimeSequence with unique name. Must have called findTimeSequence unsuccessfully first */
bool EXWriter::writeTimeSequence(FE_time_sequence *feTimeSequence)
{
	++this->timeSequenceNumber;
	char name[20];
	sprintf(name, "times%d", this->timeSequenceNumber);
	TimeSequence *timeSequence = new TimeSequence(name, feTimeSequence, this->singleTimeSet, this->singleTime);
	if (!timeSequence)
	{
		display_message(ERROR_MESSAGE, "EX Writer.  Failed to write time sequence");
		return false;
	}
	this->timeSequences.push_back(timeSequence);
	const int size = timeSequence->getSize();
	(*this->outStream) << "Time sequence: " << name << ", size=" << size << "\n";
	if (size)
	{
		char tmpString[100];
		if (this->singleTimeSet)
		{
			sprintf(tmpString, "%" FE_VALUE_STRING, this->singleTime);
			(*this->outStream) << tmpString << "\n";
		}
		else
		{
			const int columnCount = 5;
			FE_value time;
			for (int i = 0; i < size; ++i)
			{
				FE_time_sequence_get_time_for_index(feTimeSequence, i, &time);
				sprintf(tmpString, " %" FE_VALUE_STRING, time);
				(*this->outStream) << tmpString;
				if (0 == ((i + 1) % columnCount))
					(*this->outStream) << "\n";
			}
			// extra newline if not multiple of columnCount
			if (0 != (size % columnCount))
				(*this->outStream) << "\n";
		}
	}
	return true;
}


/**
 * Writes out the element shape to stream.
 */
bool EXWriter::writeElementShape(FE_element_shape *elementShape)
{
	if (!elementShape)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeElementShape.  Missing element shape");
		return false;
	}
	char *shape_description = FE_element_shape_get_EX_description(elementShape);
	if (!shape_description)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeElementShape.  Invalid shape");
		return false;
	}
	const int dimension = get_FE_element_shape_dimension(elementShape);
	(*this->outStream) << "Shape. Dimension=" << dimension << ", " << shape_description << "\n";
	DEALLOCATE(shape_description);
	return true;
}

/**
 * Writes EFT scale factor types and identifiers. Example:
 * global_general(11,12) node_patch(1,2,3,4)
 */
bool EXWriter::writeEftScaleFactorIdentifiers(const FE_element_field_template *eft)
{
	if (!((eft) && (eft->getNumberOfLocalScaleFactors() > 0)))
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeEftScaleFactorIdentifiers.  Invalid element field template");
		return false;
	}
	cmzn_elementfieldtemplate_scale_factor_type lastScaleFactorType = CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_INVALID;
	const int scaleFactorCount = eft->getNumberOfLocalScaleFactors();
	for (int s = 0; s < scaleFactorCount; ++s)
	{
		const cmzn_elementfieldtemplate_scale_factor_type scaleFactorType = eft->getScaleFactorType(s);
		const int identifier = eft->getScaleFactorIdentifier(s);
		if (scaleFactorType != lastScaleFactorType)
		{
			if (s > 0)
			{
				(*this->outStream) << ") ";
			}
			const char *typeName = 0;
			switch (scaleFactorType)
			{
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_ELEMENT_GENERAL:
				typeName = "element_general";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_ELEMENT_PATCH:
				typeName = "element_patch";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_GLOBAL_GENERAL:
				typeName = "global_general";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_GLOBAL_PATCH:
				typeName = "global_patch";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_NODE_GENERAL:
				typeName = "node_general";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_NODE_PATCH:
				typeName = "node_patch";
				break;
			case CMZN_ELEMENTFIELDTEMPLATE_SCALE_FACTOR_TYPE_INVALID:
				break;
			}
			if (typeName == 0)
			{
				display_message(ERROR_MESSAGE, "EXWriter::writeEftScaleFactorIdentifiers.  Invalid element field template scale factor type");
				return false;
			}
			(*this->outStream) << typeName << "(" << identifier;
			lastScaleFactorType = scaleFactorType;
		}
		else
		{
			(*this->outStream) << "," << identifier;
		}
	}
	(*this->outStream) << ")";
	return true;
}

/** Writes out the <basis> to <output_file>. */
bool EXWriter::writeBasis(FE_basis *basis)
{
	char *basis_string = FE_basis_get_description_string(basis);
	if (!basis_string)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeBasis.  Invalid basis");
		return false;
	}
	(*this->outStream) << basis_string;
	DEALLOCATE(basis_string);
	return true;
}

/** Write template defining field on element */
bool EXWriter::writeElementHeaderField(cmzn_element *element, int fieldIndex, FE_field *field)
{
	if (!this->writeFieldHeader(fieldIndex, field))
		return false;
	const FE_mesh_field_data *meshFieldData = field->getMeshFieldData(this->feMesh);
	if (!meshFieldData)
		return false;

	FE_field_type fe_field_type = get_FE_field_FE_field_type(field);
	const int componentCount = get_FE_field_number_of_components(field);
	for (int c = 0; c < componentCount; ++c)
	{
		char *componentName = get_FE_field_component_name(field, c);
		if (componentName)
		{
			(*this->outStream) << " " << componentName << ". ";
			DEALLOCATE(componentName);
		}
		else
		{
			(*this->outStream) << " " << c + 1 << ".";
		}
		if (fe_field_type != GENERAL_FE_FIELD)
		{
			// constant and indexed fields: no further component information
			(*this->outStream) << "\n";
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
			(*this->outStream) << ", no modify";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_CLOSEST_IN_XI1:
			(*this->outStream) << ", closest in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_DECREASING_IN_XI1:
			(*this->outStream) << ", decreasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_INCREASING_IN_XI1:
			(*this->outStream) << ", increasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_NON_DECREASING_IN_XI1:
			(*this->outStream) << ", non-decreasing in xi1";
			break;
		case FE_BASIS_MODIFY_THETA_MODE_NON_INCREASING_IN_XI1:
			(*this->outStream) << ", non-increasing in xi1";
			break;
		}

		const cmzn_elementfieldtemplate_parameter_mapping_mode parameterMappingMode = eft->getParameterMappingMode();
		switch (parameterMappingMode)
		{
		case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_ELEMENT:
		{
			const int *gridNumberInXi = eft->getLegacyGridNumberInXi();
			if (0 == gridNumberInXi)
			{
				(*this->outStream) << ", element based.\n";
			}
			else
			{
				(*this->outStream) << ", grid based.\n";
				// Future: force output in old EX format
				//const int unitGridNumberInXi[MAXIMUM_ELEMENT_XI_DIMENSIONS] = { 1, 1, 1 };
				//if (!gridNumberInXi)
				//	gridNumberInXi = unitGridNumberInXi;
				(*this->outStream) << " ";
				const int dimension = this->feMesh->getDimension();
				for (int d = 0; d < dimension; ++d)
				{
					if (0 < d)
						(*this->outStream) << ", ";
					(*this->outStream) << "#xi" << d + 1 << "=" << gridNumberInXi[d];
				}
				(*this->outStream) << "\n";
			}
			break;
		}
		case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_FIELD:
		{
			(*this->outStream) << ", field based.\n";
			break;
		}
		case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_NODE:
		{
			(*this->outStream) << ", standard node based.";
			int scaleFactorOffset = 0;
			// previously the scale factor set was identified by the basis, now it is explicitly named
			if (eft->getNumberOfLocalScaleFactors() > 0)
			{
				int scaleFactorSetIndex = 0;
				for (auto eftIter = this->elementTemplate->headerScalingEfts.begin(); eftIter != this->elementTemplate->headerScalingEfts.end(); ++eftIter)
				{
					++scaleFactorSetIndex;
					if (*eftIter == eft)
						break;
					scaleFactorOffset += (*eftIter)->getNumberOfLocalScaleFactors();
				}
				(*this->outStream) << " scale factor set=scaling" << scaleFactorSetIndex;
			}
			(*this->outStream) << "\n";

			const int nodeCount = eft->getNumberOfLocalNodes();
			const int packedNodeOffset = this->elementTemplate->headerElementNodePacking.getEftNodeOffset(eft);
			(*this->outStream) << "  #Nodes=" << nodeCount << "\n";
			// previously there was an entry for each basis node with the parameters extracted from it
			// now there is a separate entry for each block of parameters mapped from
			// the same nodes with the same number of terms
			// Note local node numbers are for the element starting at packedNodeOffset, and
			// reader is expected to respect order in element when converting to EFT indexes
			const int functionCount = eft->getNumberOfFunctions();
			int f = 0;
			while (f < functionCount)
			{
				(*this->outStream) << "  ";
				const int termCount = eft->getFunctionNumberOfTerms(f);
				int valueCount = 1;
				// for compatibility with EX versions < 2 limit function values output to those
				// for current basis node, otherwise repeated nodes will be bunched together
				int f2basisNodeLimit = FE_basis_get_basis_node_function_number_limit(basis, f);
				for (int f2 = f + 1; f2 < f2basisNodeLimit; ++f2)
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
				if (0 == termCount)
				{
					// Use node index 0 if no terms
					(*this->outStream) << "0";
				}
				else
				{
					for (int t = 0; t < termCount; ++t)
					{
						if (t > 0)
							(*this->outStream) << "+";
						(*this->outStream) << eft->getTermLocalNodeIndex(f, t) + 1 + packedNodeOffset;
					}
				}
				(*this->outStream) << ". #Values=" << valueCount << "\n";
				// nodal value labels(versions) e.g. d/ds1(2), or the special zero for no terms
				// multi term example: d/ds1(2)+d/ds2
				(*this->outStream) << "   Value labels:";
				const int f2limit = f + valueCount;
				for (int f2 = f; f2 < f2limit; ++f2)
				{
					(*this->outStream) << " ";
					if (termCount == 0)
					{
						(*this->outStream) << "zero";
					}
					else
					{
						for (int t = 0; t < termCount; ++t)
						{
							if (t > 0)
								(*this->outStream) << "+";
							const cmzn_node_value_label nodeValueLabel = eft->getTermNodeValueLabel(f2, t);
							const char *valueLabelName = ENUMERATOR_STRING(cmzn_node_value_label)(nodeValueLabel);
							(*this->outStream) << valueLabelName;
							const int version = eft->getTermNodeVersion(f2, t);
							if (version > 0)
								(*this->outStream) << "(" << version + 1 << ")";
						}
					}
				}
				(*this->outStream) << "\n";
				// New: scale factor indexes only output if there is any scaling for this EFT
				// For compatibility with old EX Versions, output scale factor indexes are relative
				// to the whole element template, not per EFT, so must have the appropriate offset added.
				if (eft->getNumberOfLocalScaleFactors() > 0)
				{
					(*this->outStream) << "   Scale factor indices:";
					for (int f2 = f; f2 < f2limit; ++f2)
					{
						(*this->outStream) << " ";
						if (termCount == 0)
						{
							(*this->outStream) << "0"; // 0 means unscaled, still required for zero terms
						}
						else
						{
							for (int t = 0; t < termCount; ++t)
							{
								if (t > 0)
									(*this->outStream) << "+";
								const int termScaleFactorCount = eft->getTermScalingCount(f2, t);
								if (0 == termScaleFactorCount)
								{
									(*this->outStream) << "0"; // 0 means unscaled
								}
								else
								{
									for (int s = 0; s < termScaleFactorCount; ++s)
									{
										if (s > 0)
											(*this->outStream) << "*";
										const int scaleFactorIndex = eft->getTermScaleFactorIndex(f2, t, s);
										(*this->outStream) << scaleFactorOffset + scaleFactorIndex + 1;
									}
								}
							}
						}
					}
					(*this->outStream) << "\n";
				}
				f = f2limit;
			}
		}	break;
		case CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_INVALID:
		{
			display_message(ERROR_MESSAGE, "EXWriter::writeElementHeaderField.  Invalid parameter mapping mode");
			return false;
			break;
		}
		}
	}
	return true;
}

/**
 * If element has a different template from the last, write element template,
 * defining if this is the first use.
 */
bool EXWriter::writeElementTemplate(cmzn_element *element)
{
	if ((this->elementTemplate) && (this->elementTemplate->matches(element, this->writeFieldsMode, this->writableFields)))
	{
		return true;  // same template as last element
	}
	this->elementTemplate = nullptr;
	const size_t etCount = this->elementTemplates.size();
	for (int et = 0; et < etCount; ++et)
	{
		if (this->elementTemplates[et]->matches(element, this->writeFieldsMode, this->writableFields))
		{
			this->elementTemplate = this->elementTemplates[et];
			break;
		}
	}
	if (!this->elementTemplate)
	{
		// define element template
		char name[20];
		++this->elementTemplateNumber;
		sprintf(name, "element%d", this->elementTemplateNumber);
		this->elementTemplate = new ElementTemplate(name, element, this->writableFields);
		this->elementTemplates.push_back(this->elementTemplate);

		// make list of fields in header, order of EFT scale factor sets
		for (auto fieldIter = this->writableFields.begin(); fieldIter != this->writableFields.end(); ++fieldIter)
		{
			FE_field *field = *fieldIter;
			const FE_mesh_field_data *meshFieldData = field->getMeshFieldData(this->feMesh);
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
				{
					break; // field not defined on this element
				}
				if (eft != lastEft)
				{
					if (eft->getNumberOfLocalScaleFactors() > 0)
					{
						lastEft = eft;
						auto eftIter = this->elementTemplate->headerScalingEfts.begin();
						for (; eftIter != this->elementTemplate->headerScalingEfts.end(); ++eftIter)
						{
							if (*eftIter == eft)
							{
								break;
							}
						}
						if (eftIter == this->elementTemplate->headerScalingEfts.end())
						{
							this->elementTemplate->headerScalingEfts.push_back(eft);
						}
					}
				}
			}
			if (c == componentCount)
			{
				this->elementTemplate->headerFields.push_back(field);
			}
		}

		(*this->outStream) << "Define element template: " << name << "\n";
		if (!this->writeElementShape(element->getElementShape()))
		{
			return false;
		}
		(*this->outStream) << "#Scale factor sets=" << this->elementTemplate->headerScalingEfts.size() << "\n";
		int scaleFactorSetIndex = 0;
		for (auto eftIter = this->elementTemplate->headerScalingEfts.begin(); eftIter != this->elementTemplate->headerScalingEfts.end(); ++eftIter)
		{
			++scaleFactorSetIndex;
			(*this->outStream) << "  scaling" << scaleFactorSetIndex << ", #Scale factors=" << (*eftIter)->getNumberOfLocalScaleFactors()
				<< ", identifiers=\"";
			if (!this->writeEftScaleFactorIdentifiers(*eftIter))
				return false;
			(*this->outStream) << "\"\n";
		}
		(*this->outStream) << "#Nodes=" << this->elementTemplate->headerElementNodePacking.getTotalNodeCount() << "\n";
		(*this->outStream) << "#Fields=" << this->elementTemplate->headerFields.size() << "\n";
		int fieldIndex = 0;
		for (auto fieldIter = this->elementTemplate->headerFields.begin(); fieldIter != this->elementTemplate->headerFields.end(); ++fieldIter)
		{
			++fieldIndex;
			FE_field *field = *fieldIter;
			if (!this->writeElementHeaderField(element, fieldIndex, field))
			{
				return false;
			}
		}
		if (!this->writeOptionalFieldValues(this->elementTemplate->headerFields))
		{
			return false;
		}
	}
	// activate element template
	(*this->outStream) << "Element template: " << this->elementTemplate->name << "\n";
	return true;
}

bool EXWriter::writeElementFieldComponentValues(cmzn_element *element,
	FE_field *field, int componentNumber)
{
	const FE_mesh_field_data *meshFieldData = field->getMeshFieldData(this->feMesh);
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
			display_message(ERROR_MESSAGE, "EXWriter::writeElementFieldComponentValues.  Missing real values");
			return false;
		}
		char tmpString[100];
		for (int v = 0; v < valueCount; ++v)
		{
			sprintf(tmpString, " %" FE_VALUE_STRING, values[v]);
			(*this->outStream) << tmpString;
			if (0 == ((v + 1) % columnCount))
				(*this->outStream) << "\n";
		}
		// extra newline if not multiple of columnCount
		if (0 != (valueCount % columnCount))
			(*this->outStream) << "\n";
		break;
	}
	case INT_VALUE:
	{
		auto component = static_cast<FE_mesh_field_data::Component<int>*>(componentBase);
		const int *values = component->getElementValues(element->getIndex(), valueCount);
		if (!values)
		{
			display_message(ERROR_MESSAGE, "EXWriter::writeElementFieldComponentValues.  Missing int values");
			return false;
		}
		for (int v = 0; v < valueCount; ++v)
		{
			(*this->outStream) << " " << values[v];
			if (0 == ((v + 1) % columnCount))
				(*this->outStream) << "\n";
		}
		// extra newline if not multiple of columnCount
		if (0 != (valueCount % columnCount))
			(*this->outStream) << "\n";
		break;
	}
	default:
	{
		display_message(ERROR_MESSAGE,
			"EXWriter::writeElementFieldComponentValues.  Unsupported value type %s", Value_type_string(valueType));
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
bool EXWriter::writeElement(cmzn_element *element)
{
	(*this->outStream) << "Element: " << element->getIdentifier() << "\n";

	if (!this->elementTemplate)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeElement.  Missing element template");
		return false;
	}

	// Faces: if defined
	const FE_mesh::ElementShapeFaces *elementShapeFaces = this->feMesh->getElementShapeFacesConst(element->getIndex());
	if (!elementShapeFaces)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeElement.  Missing ElementShapeFaces");
		return false;
	}
	FE_mesh *faceMesh = this->feMesh->getFaceMesh();
	if (faceMesh)
	{
		const int faceCount = elementShapeFaces->getFaceCount();
		const DsLabelIndex *faceIndexes;
		if ((0 < faceCount) && (faceIndexes = elementShapeFaces->getElementFaces(element->getIndex())))
		{
			(*this->outStream) << " Faces:\n";
			for (int i = 0; i < faceCount; ++i)
			{
				if (faceIndexes[i] >= 0)
					(*this->outStream) << " " << faceMesh->getElementIdentifier(faceIndexes[i]);
				else
					(*this->outStream) << " -1"; // face not set; can't use 0 as it is a valid identifier
			}
			(*this->outStream) << "\n";
		}
	}

	// Values: if writing any element-based fields
	bool firstElementBasedField = true;
	for (auto fieldIter = this->elementTemplate->headerFields.begin(); fieldIter != this->elementTemplate->headerFields.end(); ++fieldIter)
	{
		FE_field *field = *fieldIter;
		if (GENERAL_FE_FIELD != get_FE_field_FE_field_type(field))
			continue;
		const FE_mesh_field_data *meshFieldData = field->getMeshFieldData(this->feMesh);
		const int componentCount = get_FE_field_number_of_components(field);
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
			const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
			if (eft->getNumberOfElementDOFs() > 0)
			{
				if (firstElementBasedField)
				{
					(*this->outStream) << " Values :\n";
					firstElementBasedField = false;
				}
				if (!this->writeElementFieldComponentValues(element, field, c))
					return false;
			}
		}
	}

	// Nodes: if any
	if (this->elementTemplate->headerElementNodePacking.getTotalNodeCount() > 0)
	{
		FE_nodeset *nodeset = this->feMesh->getNodeset();
		(*this->outStream) << " Nodes:\n";
		int index = 0;
		const FE_element_field_template *eft;
		while (0 != (eft = this->elementTemplate->headerElementNodePacking.getFirstEftAtIndex(index)))
		{
			const FE_mesh_element_field_template_data *meshEftData = this->feMesh->getElementfieldtemplateData(eft);
			const int nodeCount = eft->getNumberOfLocalNodes();
			const DsLabelIndex *nodeIndexes = meshEftData->getElementNodeIndexes(element->getIndex());
			if (nodeIndexes)
			{
				for (int n = 0; n < nodeCount; ++n)
				{
					(*this->outStream) << " " << nodeset->getNodeIdentifier(nodeIndexes[n]);
				}
			}
			else
			{
				for (int n = 0; n < nodeCount; ++n)
				{
					(*this->outStream) << " -1";
				}
			}
			++index;
		}
		(*this->outStream) << "\n";
	}

	// Scale factors: if any scale factor sets being output
	if (this->elementTemplate->headerScalingEfts.size() > 0)
	{
		int scaleFactorNumber = 0;
		char tmpString[100];
		(*this->outStream) << " Scale factors:\n";
		for (auto eftIter = this->elementTemplate->headerScalingEfts.begin(); eftIter != this->elementTemplate->headerScalingEfts.end(); ++eftIter)
		{
			const FE_element_field_template *eft = *eftIter;
			FE_mesh_element_field_template_data *meshEftData = this->feMesh->getElementfieldtemplateData(eft);
			const int scaleFactorCount = eft->getNumberOfLocalScaleFactors();
			int result = CMZN_OK;
			const DsLabelIndex *scaleFactorIndexes = meshEftData->getOrCreateElementScaleFactorIndexes(result, element->getIndex());
			if (!scaleFactorIndexes)
			{
				display_message(WARNING_MESSAGE, "EXWriter::writeElement.  Missing scale factors for element %d", element->getIdentifier());
			}
			for (int s = 0; s < scaleFactorCount; ++s)
			{
				++scaleFactorNumber;
				sprintf(tmpString, "%" FE_VALUE_STRING, (scaleFactorIndexes) ? this->feMesh->getScaleFactor(scaleFactorIndexes[s]) : 0.0);
				(*this->outStream) << " " << tmpString;
				if ((0 < FE_VALUE_MAX_OUTPUT_COLUMNS)
					&& (0 == (scaleFactorNumber % FE_VALUE_MAX_OUTPUT_COLUMNS)))
				{
					(*this->outStream) << "\n";
				}
			}
			// extra new line if not multiple of FE_VALUE_MAX_OUTPUT_COLUMNS values
			if ((FE_VALUE_MAX_OUTPUT_COLUMNS <= 0)
				|| (0 != (scaleFactorNumber % FE_VALUE_MAX_OUTPUT_COLUMNS)))
			{
				(*this->outStream) << "\n";
			}
		}
	}
	return true;
}

/**
 * @return  True if specification of what to output, optionally including the
 * field order info, indicates the <element> is to be written.
 */
bool EXWriter::elementIsToBeWritten(cmzn_element *element)
{
	switch (this->writeCriterion)
	{
	case FE_WRITE_COMPLETE_GROUP:
	{
	} break;
	case FE_WRITE_WITH_ALL_LISTED_FIELDS:
	{
		const size_t fieldsCount = this->writableFields.size();
		for (size_t i = 0; i < fieldsCount; ++i)
		{
			if (!FE_field_is_defined_in_element(this->writableFields[i], element))
			{
				return false;
			}
		}
	} break;
	case FE_WRITE_WITH_ANY_LISTED_FIELDS:
	{
		const size_t fieldsCount = this->writableFields.size();
		for (size_t i = 0; i < fieldsCount; ++i)
		{
			if (FE_field_is_defined_in_element(this->writableFields[i], element))
			{
				return true;
			}
		}
		return false;
	} break;
	default:
	{
		display_message(ERROR_MESSAGE,
			"EXWriter::elementIsToBeWritten.  Unknown write criterion");
		return false;
	} break;
	}
	return true;
}

bool EXWriter::writeFeMesh(FE_mesh *feMeshIn)
{
	(*this->outStream) << "!#mesh ";
	this->writeSafeName(feMeshIn->getName());
	(*this->outStream) << ", dimension=" << feMeshIn->getDimension();
	if (feMeshIn->getFaceMesh())
	{
		(*this->outStream) << ", face mesh=";
		this->writeSafeName(feMeshIn->getFaceMesh()->getName());
	}
	if (feMeshIn->getNodeset())
	{
		(*this->outStream) << ", nodeset=";
		this->writeSafeName(feMeshIn->getNodeset()->getName());
	}
	(*this->outStream) << "\n";
	return true;
}

bool EXWriter::writeMesh(int dimension, cmzn_field_group *group)
{
	bool result = true;
	cmzn_mesh *mesh = cmzn_fieldmodule_find_mesh_by_dimension(this->fieldmodule, dimension);
	if (group)
	{
		cmzn_field_element_group *element_group = cmzn_field_group_get_field_element_group(group, mesh);
		cmzn_mesh_destroy(&mesh);
		mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
		cmzn_field_element_group_destroy(&element_group);
	}
	if (mesh && (cmzn_mesh_get_size(mesh) > 0))
	{
		FE_mesh *tmpFeMesh = FE_region_find_FE_mesh_by_dimension(this->feRegion, dimension);
		this->setMesh(tmpFeMesh);
		this->writeFeMesh(tmpFeMesh);
		cmzn_elementiterator *iter = cmzn_mesh_create_elementiterator(mesh);
		cmzn_element *element = nullptr;
		while (nullptr != (element = iter->nextElement()))
		{
			if (this->elementIsToBeWritten(element))
			{
				if (!this->writeElementTemplate(element))
				{
					result = false;
					break;
				}
				if (!this->writeElement(element))
				{
					result = false;
					break;
				}
			}
		}
		cmzn_elementiterator_destroy(&iter);
		cmzn_mesh_destroy(&mesh);
	}
	return result;
}

/** Write mesh group element identifiers in compact form with ranges:
 * Element group:
 * 1,3..7,22..150,155,200..423
 */
bool EXWriter::writeElementGroup(int dimension, cmzn_field_group *group)
{
	bool result = true;
	cmzn_mesh *mesh = cmzn_fieldmodule_find_mesh_by_dimension(this->fieldmodule, dimension);
	if (group)
	{
		cmzn_field_element_group *element_group = cmzn_field_group_get_field_element_group(group, mesh);
		cmzn_mesh_destroy(&mesh);
		mesh = cmzn_mesh_group_base_cast(cmzn_field_element_group_get_mesh_group(element_group));
		cmzn_field_element_group_destroy(&element_group);
	}
	if (mesh && (cmzn_mesh_get_size(mesh) > 0))
	{
		FE_mesh *tmpFeMesh = FE_region_find_FE_mesh_by_dimension(this->feRegion, dimension);
		this->writeFeMesh(tmpFeMesh);
		(*this->outStream) << "Element group:\n";
		cmzn_elementiterator *iter = cmzn_mesh_create_elementiterator(mesh);
		cmzn_element *element = nullptr;
		int stopIdentifier = -2;  // forces starting a new range
		int startIdentifier = -2;
		const int columnCount = 10;  // limit of numbers written on each row, will get one more if range started
		int columns = 0;
		while (nullptr != (element = iter->nextElement()))
		{
			if (this->elementIsToBeWritten(element))
			{
				const int identifier = element->getIdentifier();
				if (identifier == (stopIdentifier + 1))
				{
					stopIdentifier = identifier;  // enlarge range
				}
				else
				{
					if (startIdentifier < stopIdentifier)
					{
						(*this->outStream) << ".." << stopIdentifier;
						++columns;
					}
					if (columns >= columnCount)
					{
						(*this->outStream) << ",\n";
						columns = 0;
					}
					else if (columns > 0)
					{
						(*this->outStream) << ",";
					}
					(*this->outStream) << identifier;
					++columns;
					startIdentifier = stopIdentifier = identifier;
				}
			}
		}
		if (startIdentifier < stopIdentifier)
		{
			(*this->outStream) << ".." << stopIdentifier;
			++columns;
		}
		if (columns > 0)
		{
			(*this->outStream) << "\n";
		}
		cmzn_elementiterator_destroy(&iter);
		cmzn_mesh_destroy(&mesh);
	}
	return result;
}

/**
 * Writes a node field to stream. After the field header is written with
 * write_FE_field_header, value index, derivative and version info are output on
 * following lines, one per component:
 * COMPONENT_NAME.  #Values=~ (valueType(versions) ...)
 */
bool EXWriter::writeNodeHeaderField(cmzn_node *node, int fieldIndex, FE_field *field)
{
	const FE_node_field *nodeField = node->getNodeField(field);
	if (!nodeField)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeNodeHeaderField.  Field is not defined at node");
		return false;
	}
	FE_time_sequence *feTimeSequence = nodeField->getTimeSequence();
	TimeSequence *timeSequence = (feTimeSequence) ? this->findTimeSequence(feTimeSequence) : nullptr;
	if (!this->writeFieldHeader(fieldIndex, field, timeSequence))
	{
		return false;
	}
	FE_field_type fe_field_type = get_FE_field_FE_field_type(field);
	const int componentCount = get_FE_field_number_of_components(field);
	for (int c = 0; c < componentCount; ++c)
	{
		char *componentName = get_FE_field_component_name(field, c);
		if (componentName)
		{
			(*this->outStream) << " " << componentName << ".";
			DEALLOCATE(componentName);
		}
		else
		{
			(*this->outStream) << " " << c + 1 << ".";
		}
		if (fe_field_type != GENERAL_FE_FIELD)
		{
			// constant and indexed fields: no further component information
			(*this->outStream) << "\n";
			continue;
		}
		const FE_node_field_template& nft = *(nodeField->getComponent(c));
		const int valuesCount = nft.getTotalValuesCount();
		(*this->outStream) << " #Values=" << valuesCount << " (";
		const int valueLabelsCount = nft.getValueLabelsCount();
		for (int d = 0; d < valueLabelsCount; ++d)
		{
			if (d > 0)
			{
				(*this->outStream) << ",";
			}
			const cmzn_node_value_label valueLabel = nft.getValueLabelAtIndex(d);
			const int versionsCount = nft.getVersionsCountAtIndex(d);
			(*this->outStream) << ENUMERATOR_STRING(cmzn_node_value_label)(valueLabel);
			if (versionsCount > 1)
			{
				(*this->outStream) << "(" << versionsCount << ")";
			}
		}
		(*this->outStream) << ")\n";
	}
	return true;
}

/**
 * If node has a different template from the last, write node template,
 * defining if this is the first use.
 */
bool EXWriter::writeNodeTemplate(cmzn_node *node)
{
	if ((this->nodeTemplate) && (this->nodeTemplate->matches(node, this->writeFieldsMode, this->writableFields)))
	{
		return true;  // same template as last node
	}
	this->nodeTemplate = nullptr;
	const size_t ntCount = this->nodeTemplates.size();
	for (int nt = 0; nt < ntCount; ++nt)
	{
		if (this->nodeTemplates[nt]->matches(node, this->writeFieldsMode, this->writableFields))
		{
			this->nodeTemplate = this->nodeTemplates[nt];
			break;
		}
	}
	if (!this->nodeTemplate)
	{
		// define node template
		char name[20];
		++this->nodeTemplateNumber;
		sprintf(name, "node%d", this->nodeTemplateNumber);
		this->nodeTemplate = new NodeTemplate(name, node);
		this->nodeTemplates.push_back(this->nodeTemplate);

		// make list of fields in header, write any time sequences in use before defining node template
		for (auto fieldIter = this->writableFields.begin(); fieldIter != this->writableFields.end(); ++fieldIter)
		{
			FE_field *field = *fieldIter;
			FE_node_field *nodeField = node->getNodeField(field);
			if (nodeField)
			{
				this->nodeTemplate->headerFields.push_back(field);
				FE_time_sequence *feTimeSequence = nodeField->getTimeSequence();
				if (feTimeSequence)
				{
					if (!this->findTimeSequence(feTimeSequence))
					{
						if (!this->writeTimeSequence(feTimeSequence))
						{
							return false;
						}
					}
				}
			}
		}
		(*this->outStream) << "Define node template: " << name << "\n";
		(*this->outStream) << "Shape. Dimension=0\n";
		(*this->outStream) << "#Fields=" << this->nodeTemplate->headerFields.size() << "\n";
		int fieldIndex = 0;
		for (auto fieldIter = this->nodeTemplate->headerFields.begin(); fieldIter != this->nodeTemplate->headerFields.end(); ++fieldIter)
		{
			++fieldIndex;
			FE_field *field = *fieldIter;
			if (!this->writeNodeHeaderField(node, fieldIndex, field))
				return false;
		}
		if (!this->writeOptionalFieldValues(this->nodeTemplate->headerFields))
		{
			return false;
		}
	}
	// activate node template
	(*this->outStream) << "Node template: " << this->nodeTemplate->name << "\n";
	return true;
}

/**
 * Writes out the nodal values. Each component starts on a new line.
 * In the new EX2 format, all versions for a given derivative are
 * consecutively output.
 * Only call for general field defined on node - this is not checked.
 */
bool EXWriter::writeNodeFieldValues(cmzn_node *node, FE_field *field)
{
	const int componentCount = get_FE_field_number_of_components(field);
	const FE_node_field *nodeField = node->getNodeField(field);
	if (!nodeField)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeNodeFieldValues.  Field %s not defined at node %d",
			get_FE_field_name(field), node->getIdentifier());
		return false;
	}
	const int maximumValuesCount = nodeField->getMaximumComponentTotalValuesCount();
	FE_time_sequence *feTimeSequence = nodeField->getTimeSequence();
	TimeSequence *timeSequence = (feTimeSequence) ? this->findTimeSequence(feTimeSequence) : nullptr;
	const int timeCount = (timeSequence) ? timeSequence->getSize() : 1;
	const enum Value_type valueType = get_FE_field_value_type(field);
	switch (valueType)
	{
	case ELEMENT_XI_VALUE:
	{
		FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		cmzn_element *element;
		const FE_mesh *hostMesh = field->getElementXiHostMesh();
		if (!hostMesh)
		{
			display_message(ERROR_MESSAGE, "EXWriter::writeNodeFieldValues.  Missing host mesh for element xi field");
			return false;
		}
		// should only be one component, no derivatives and versions
		for (int c = 0; c < componentCount; ++c)
		{
			if (!get_FE_nodal_element_xi_value(node, field, /*component_number*/c, &element, xi))
			{
				display_message(ERROR_MESSAGE, "EXWriter::writeNodeFieldValues.  Could not get element_xi value");
				return false;
			}
			if (!this->writeElementXiValue(hostMesh, element ? element->getIndex() : DS_LABEL_IDENTIFIER_INVALID, xi))
			{
				return false;
			}
			(*this->outStream) << "\n";
		}
	} break;
	case FE_VALUE_VALUE:
	{
		char tmpString[100];
		std::vector<FE_value> valuesVector(maximumValuesCount);
		FE_value *values = valuesVector.data();
		for (int t = 0; t < timeCount; ++t)
		{
			FE_value time = this->singleTime;
			if ((timeSequence) && (!this->singleTimeSet))
			{
				FE_time_sequence_get_time_for_index(feTimeSequence, t, &time);
			}
			for (int c = 0; c < componentCount; ++c)
			{
				const FE_node_field_template *nft = nodeField->getComponent(c);
				const int valuesCount = nft->getTotalValuesCount();
				// EX2+ format matches internal storage with versions consecutive for each value label
				if (CMZN_OK != cmzn_node_get_field_component_FE_value_values(node, field, c, time, maximumValuesCount, values))
				{
					display_message(ERROR_MESSAGE, "EXWriter::writeNodeFieldValues.  "
						"Failed to get FE_value values for field %s component %d at node %d",
						get_FE_field_name(field), c + 1, node->getIdentifier());
					return false;
				}
				for (int v = 0; v < valuesCount; ++v)
				{
					sprintf(tmpString, "%" FE_VALUE_STRING, values[v]);
					(*this->outStream) << " " << tmpString;
				}
				if (valuesCount)
				{
					(*this->outStream) << "\n";
				}
			}
		}
	} break;
	case INT_VALUE:
	{
		std::vector<int> valuesVector(maximumValuesCount);
		int *values = valuesVector.data();
		for (int t = 0; t < timeCount; ++t)
		{
			FE_value time = this->singleTime;
			if ((timeSequence) && (!this->singleTimeSet))
			{
				FE_time_sequence_get_time_for_index(feTimeSequence, t, &time);
			}
			for (int c = 0; c < componentCount; ++c)
			{
				const FE_node_field_template *nft = nodeField->getComponent(c);
				const int valuesCount = nft->getTotalValuesCount();
				// EX2+ format matches internal storage with versions consecutive for each value label
				if (CMZN_OK != cmzn_node_get_field_component_int_values(node, field, c, time, maximumValuesCount, values))
				{
					display_message(ERROR_MESSAGE, "EXWriter::writeNodeFieldValues.  "
						"Failed to get FE_value values for field %s component %d at node %d",
						get_FE_field_name(field), c + 1, node->getIdentifier());
					return false;
				}
				for (int v = 0; v < valuesCount; ++v)
				{
					(*this->outStream) << " " << values[v];
				}
				if (valuesCount)
				{
					(*this->outStream) << "\n";
				}
			}
		}
	} break;
	case STRING_VALUE:
	{
		char *the_string;

		// should only be one component, no derivatives and versions
		for (int c = 0; c < componentCount; ++c)
		{
			if (get_FE_nodal_string_value(node, field, /*component_number*/c, &the_string))
			{
				if (the_string)
				{
					make_valid_token(&the_string);
					(*this->outStream) << " " << the_string;
					DEALLOCATE(the_string);
				}
				else
				{
					/* empty string */
					(*this->outStream) << " \"\"";
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"EXWriter::writeNodeFieldValues.  Could not get string");
			}
			(*this->outStream) << "\n";
		}
	} break;
	default:
	{
		display_message(ERROR_MESSAGE,
			"EXWriter::writeNodeFieldValues.  Value type %s not supported",
			Value_type_string(valueType));
	} break;
	}
	return true;
}

/** Writes out a node to stream */
bool EXWriter::writeNode(cmzn_node *node)
{
	(*this->outStream) << "Node: " << node->getIdentifier() << "\n";

	// values, if writing any general fields
	for (auto fieldIter = this->nodeTemplate->headerFields.begin(); fieldIter != this->nodeTemplate->headerFields.end(); ++fieldIter)
	{
		FE_field *field = *fieldIter;
		if ((GENERAL_FE_FIELD == get_FE_field_FE_field_type(field))
			&& !this->writeNodeFieldValues(node, field))
		{
			return false;
		}
	}
	return true;
}

/**
 * @return  True if specification of what to output, optionally including the
 * field order info, indicates the node is to be written.
 */
bool EXWriter::nodeIsToBeWritten(cmzn_node *node)
{
	switch (this->writeCriterion)
	{
	case FE_WRITE_COMPLETE_GROUP:
	{
	} break;
	case FE_WRITE_WITH_ALL_LISTED_FIELDS:
	{
		const size_t fieldsCount = this->writableFields.size();
		for (size_t i = 0; i < fieldsCount; ++i)
		{
			if (!(node->getNodeField(this->writableFields[i])))
			{
				return false;
			}
		}
	} break;
	case FE_WRITE_WITH_ANY_LISTED_FIELDS:
	{
		const size_t fieldsCount = this->writableFields.size();
		for (size_t i = 0; i < fieldsCount; ++i)
		{
			if (node->getNodeField(this->writableFields[i]))
			{
				return true;
			}
		}
		return false;
	} break;
	default:
	{
		display_message(ERROR_MESSAGE,
			"EXWriter::nodeIsToBeWritten.  Unknown writeCriterion");
		return false;
	} break;
	}
	return true;
}

bool EXWriter::writeFeNodeset(FE_nodeset *feNodesetIn)
{
	(*this->outStream) << "!#nodeset ";
	this->writeSafeName(feNodesetIn->getName());
	(*this->outStream) << "\n";
	return true;
}

bool EXWriter::writeNodeset(cmzn_field_domain_type fieldDomainType, cmzn_field_group *group)
{
	bool result = true;
	cmzn_nodeset *nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(this->fieldmodule,
		fieldDomainType);
	if (group)
	{
		cmzn_field_node_group *node_group = cmzn_field_group_get_field_node_group(group, nodeset);
		cmzn_nodeset_destroy(&nodeset);
		nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
		cmzn_field_node_group_destroy(&node_group);
	}
	if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
	{
		FE_nodeset *tmpFeNodeset = FE_region_find_FE_nodeset_by_field_domain_type(this->feRegion, fieldDomainType);
		this->setNodeset(tmpFeNodeset);
		this->writeFeNodeset(tmpFeNodeset);
		cmzn_nodeiterator *iter = cmzn_nodeset_create_nodeiterator(nodeset);
		cmzn_node *node = nullptr;
		while (0 != (node = iter->nextNode()))
		{
			if (this->nodeIsToBeWritten(node))
			{
				if (!this->writeNodeTemplate(node))
				{
					result = false;
					break;
				}
				if (!this->writeNode(node))
				{
					result = false;
					break;
				}
			}
		}
		cmzn_nodeiterator_destroy(&iter);
		cmzn_nodeset_destroy(&nodeset);
	}
	return result;
}

/** Write group node identifiers in compact form with ranges:
 * Node group:
 * 1,3..7,22..150,155,200..423
 */
bool EXWriter::writeNodeGroup(cmzn_field_domain_type fieldDomainType, cmzn_field_group *group)
{
	cmzn_nodeset *nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(this->fieldmodule,
		fieldDomainType);
	if (group)
	{
		cmzn_field_node_group *node_group = cmzn_field_group_get_field_node_group(group, nodeset);
		cmzn_nodeset_destroy(&nodeset);
		nodeset = cmzn_nodeset_group_base_cast(cmzn_field_node_group_get_nodeset_group(node_group));
		cmzn_field_node_group_destroy(&node_group);
	}
	if (nodeset && (cmzn_nodeset_get_size(nodeset) > 0))
	{
		FE_nodeset *tmpFeNodeset = FE_region_find_FE_nodeset_by_field_domain_type(this->feRegion, fieldDomainType);
		this->setNodeset(tmpFeNodeset);
		this->writeFeNodeset(tmpFeNodeset);
		(*this->outStream) << "Node group:\n";
		cmzn_nodeiterator *iter = cmzn_nodeset_create_nodeiterator(nodeset);
		cmzn_node *node = nullptr;
		int stopIdentifier = -2;  // forces starting a new range
		int startIdentifier = -2;
		const int columnCount = 10;  // limit of numbers written on each row, will get one more if range started
		int columns = 0;
		while (0 != (node = iter->nextNode()))
		{
			if (this->nodeIsToBeWritten(node))
			{
				const int identifier = node->getIdentifier();
				if (identifier == (stopIdentifier + 1))
				{
					stopIdentifier = identifier;  // enlarge range
				}
				else
				{
					if (startIdentifier < stopIdentifier)
					{
						(*this->outStream) << ".." << stopIdentifier;
						++columns;
					}
					if (columns >= columnCount)
					{
						(*this->outStream) << ",\n";
						columns = 0;
					}
					else if (columns > 0)
					{
						(*this->outStream) << ",";
					}
					(*this->outStream) << identifier;
					++columns;
					startIdentifier = stopIdentifier = identifier;
				}
			}
		}
		if (startIdentifier < stopIdentifier)
		{
			(*this->outStream) << ".." << stopIdentifier;
			++columns;
		}
		if (columns > 0)
		{
			(*this->outStream) << "\n";
		}
		cmzn_nodeiterator_destroy(&iter);
		cmzn_nodeset_destroy(&nodeset);
	}
	return true;
}

int EXWriter::writeRegionContent(cmzn_field_group *group)
{
	int return_code = 1;
	// write nodes then elements then data last since future plan is to remove the feature
	// where the same field can be defined simultaneously on nodes & elements and also data.
	// To migrate the first one will use the actual field name and the other will need to
	// qualify it, and we want the qualified one to be the datapoints.
	if (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_NODES)
	{
		if (!this->writeNodeset(CMZN_FIELD_DOMAIN_TYPE_NODES, group))
		{
			return_code = 0;
		}
	}
	if (return_code)
	{
		const int highestDimension = FE_region_get_highest_dimension(this->feRegion);
		/* write 1-D, 2-D then 3-D so lines and faces precede elements */
		for (int dimension = 1; dimension <= highestDimension; ++dimension)
		{
			if ((dimension == 1 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH1D)) ||
				(dimension == 2 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH2D)) ||
				(dimension == 3 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH3D)) ||
				(dimension == highestDimension &&
				(this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION)))
			{
				if (!this->writeMesh(dimension, group))
				{
					return_code = 0;
					break;
				}
			}
		}
	}
	if (return_code && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS))
	{
		if (!this->writeNodeset(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS, group))
		{
			return_code = 0;
		}
	}
	if (!return_code)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeRegionContent.  Failed");
	}
	return return_code;
}

bool EXWriter::writeGroup(cmzn_field_group *group)
{
	bool result = true;
	char *groupName = cmzn_field_get_name(cmzn_field_group_base_cast(group));
	(*this->outStream) << "Group name: " << groupName << "\n";
	DEALLOCATE(groupName);

	if (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_NODES)
	{
		result = this->writeNodeGroup(CMZN_FIELD_DOMAIN_TYPE_NODES, group);
	}
	const int highestDimension = FE_region_get_highest_dimension(this->feRegion);
	/* write 1-D, 2-D then 3-D so lines and faces precede elements */
	for (int dimension = 1; (dimension <= highestDimension) && result; ++dimension)
	{
		if ((dimension == 1 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH1D)) ||
			(dimension == 2 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH2D)) ||
			(dimension == 3 && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH3D)) ||
			(dimension == highestDimension &&
			(this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION)))
		{
			result = this->writeElementGroup(dimension, group);
		}
	}
	if (result && (this->writeDomainTypes & CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS))
	{
		result = this->writeNodeGroup(CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS, group);
	}
	if (!result)
	{
		display_message(ERROR_MESSAGE, "EXWriter::writeGroup.  Failed");
	}
	return result;
}

int EXWriter::writeRegion(cmzn_region *regionIn)
{
	this->region = regionIn;
	this->feRegion = this->region->get_FE_region();
	this->fieldmodule = cmzn_region_get_fieldmodule(regionIn);

	// make a list of all fields that may be written, with indexer fields before any indexed fields using them
	this->writableFields.clear();
	if (this->writeFieldsMode != FE_WRITE_NO_FIELDS)
	{
		if (this->writeFieldsMode == FE_WRITE_ALL_FIELDS)
		{
			FE_region_for_each_FE_field(this->feRegion, FE_field_add_to_vector_indexer_priority, (void *)&this->writableFields);
		}
		else if (this->fieldNames.size() > 0)
		{
			size_t fieldNamesCount = this->fieldNames.size();
			for (size_t i = 0; i < fieldNamesCount; ++i)
			{
				FE_field *feField = FE_region_get_FE_field_from_name(this->feRegion, this->fieldNames[i].c_str());
				if (feField)
				{
					++(this->fieldNamesCounters[i]);
					FE_field_add_to_vector_indexer_priority(feField, (void *)&this->writableFields);
				}
			}
		}
	}

	cmzn_field_group *group = nullptr;
	if (this->groupName)
	{
		cmzn_field *field = cmzn_fieldmodule_find_field_by_name(this->fieldmodule, this->groupName);
		if (field)
		{
			group = cmzn_field_cast_group(field);
			cmzn_field_destroy(&field);
		}
	}

	// write region path and define contents (limited to group if supplied)
	char *region_path = this->region->getRelativePath(this->rootRegion);
	int error = 0;
	// add leading '/' (required esp. for root region)
	append_string(&region_path, CMZN_REGION_PATH_SEPARATOR_STRING, &error, /*prefix*/true);
	(*this->outStream) << "Region: " << region_path << "\n";
	DEALLOCATE(region_path);

	int return_code = this->writeRegionContent(group);

	if (return_code && ((group) || (this->recursionMode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)))
	{
		if (group)
		{
			if (!this->writeGroup(group))
			{
				return_code = 0;
			}
		}
		else
		{
			// write all groups
			cmzn_fieldmodule *field_module = cmzn_region_get_fieldmodule(region);
			cmzn_fielditerator *field_iter = cmzn_fieldmodule_create_fielditerator(field_module);
			cmzn_field *field = nullptr;
			while ((0 != (field = cmzn_fielditerator_next_non_access(field_iter))) && return_code)
			{
				cmzn_field_group *output_group = cmzn_field_cast_group(field);
				if (output_group)
				{
					if (!this->writeGroup(output_group))
					{
						return_code = 0;
					}
					cmzn_field_group_destroy(&output_group);
				}
			}
			cmzn_fielditerator_destroy(&field_iter);
			cmzn_fieldmodule_destroy(&field_module);
		}
	}

	if (this->recursionMode == CMZN_STREAMINFORMATION_REGION_RECURSION_MODE_ON)
	{
		// write child regions
		cmzn_region *childRegion = region->getFirstChild();
		while (childRegion)
		{
			return_code = this->writeRegion(childRegion);
			if (!return_code)
			{
				break;
			}
			childRegion = childRegion->getNextSibling();
		}
	}
	if (group)
	{
		cmzn_field_group_destroy(&group);
	}

	this->writableFields.clear();
	cmzn_fieldmodule_destroy(&this->fieldmodule);
	return return_code;
}

int EXWriter::write(cmzn_region *regionIn)
{
	int return_code = 1;
	(*this->outStream) << "EX Version: 3\n";
	if (cmzn_region_contains_subregion(this->rootRegion, regionIn))
	{
		return_code = this->writeRegion(regionIn);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "EX write failed");
		}

		// warn about unused field names
		if (this->writeFieldsMode == FE_WRITE_LISTED_FIELDS)
		{
			for (size_t i = 0; i < this->fieldNames.size(); ++i)
			{
				if (this->fieldNamesCounters[i] == 0)
				{
					display_message(WARNING_MESSAGE,
						"No field named '%s' found in any region written to EX file",
						this->fieldNames[i].c_str());
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"EXWriter::write.  Region is not within root region");
		return_code = 0;
	}
	return return_code;
}

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

/**
 * Writes an EX file with supplied root_region at the top level of the file.
 *
 * @param outStream  The file/memory stream to write region and field data to.
 * @param rootRegion  The root region of any data to be written. Need not be
 *   the true root of region hierarchy, but region paths in file are relative to
 *   this region.
 * @param region  The base region to output, in tree under root region.
 * @param groupName  Optional name of group to limit output to.Actual
 *   group found from name in each region.
 * @param timeSet  True if output single time, false to output all times.
 * @param time  The time to output if single time.
 * @param writeDomainTypes  Bitwise OR of cmzn_field_domain_type flags
 *   setting which meshes or nodesets to write.
 * @param writeFieldsMode  Controls which fields are written to file.
 *   If mode is FE_WRITE_LISTED_FIELDS then :
 *-Number/list of field_names must be supplied;
 *-Field names not used in a region are ignored;
 *-Warnings are given for any field names not used in any output region.
 * @param fieldNamesCount  The number of names in the field_names array.
 * @param fieldNames  Array of field names.
 * @param writeCriterion  Controls which objects are written.Some modes
 *   limit output to nodes or objects with any or all listed fields defined.
 * @param recursionMode  Controls whether sub-regions and sub-groups are
 *   recursively written.
 */
int write_exregion_to_stream(ostream *outStream,
	struct cmzn_region *rootRegion,
	struct cmzn_region *region, const char *groupName,
	bool timeSet, FE_value time,
	cmzn_field_domain_types writeDomainTypes,
	FE_write_fields_mode writeFieldsMode,
	int fieldNamesCount, const char * const *fieldNames,
	FE_write_criterion writeCriterion,
	cmzn_streaminformation_region_recursion_mode recursionMode)
{
	int return_code = 1;
	if (outStream && rootRegion && region &&
		((writeFieldsMode != FE_WRITE_LISTED_FIELDS) ||
			((0 < fieldNamesCount) && fieldNames)))
	{
		EXWriter exWriter(outStream, rootRegion, groupName, timeSet, time,
			writeDomainTypes, writeFieldsMode, fieldNamesCount, fieldNames,
			writeCriterion, recursionMode);
		return_code = exWriter.write(region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_exregion_to_stream.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int write_exregion_file_of_name(
	const char *fileName,
	struct cmzn_region *rootRegion,
	struct cmzn_region *region, const char *groupName,
	bool timeSet, FE_value time,
	cmzn_field_domain_types writeDomainTypes,
	FE_write_fields_mode writeFieldsMode,
	int fieldNamesCount, const char * const *fieldNames,
	FE_write_criterion writeCriterion,
	cmzn_streaminformation_region_recursion_mode recursionMode)
{
	int return_code = 1;
	if (fileName)
	{
		ofstream fileStream;
		fileStream.open(fileName, ios::out);
		if (fileStream.is_open())
		{
			return_code = write_exregion_to_stream(&fileStream, rootRegion, region, groupName,
				timeSet, time, writeDomainTypes, writeFieldsMode, fieldNamesCount, fieldNames,
				writeCriterion, recursionMode);
			fileStream.close();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Could not open for writing exregion file: %s", fileName);
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
}

int write_exregion_file_to_memory_block(
	void **memoryBlock, unsigned int *memoryBlockLength,
	struct cmzn_region *rootRegion,
	struct cmzn_region *region, const char *groupName,
	bool timeSet, FE_value time,
	cmzn_field_domain_types writeDomainTypes,
	FE_write_fields_mode writeFieldsMode,
	int fieldNamesCount, const char * const *fieldNames,
	FE_write_criterion writeCriterion,
	cmzn_streaminformation_region_recursion_mode recursionMode)
{
	int return_code = 1;
	if (memoryBlock)
	{
		ostringstream stringStream;
		if (stringStream)
		{
			return_code = write_exregion_to_stream(&stringStream, rootRegion, region, groupName,
				timeSet, time, writeDomainTypes, writeFieldsMode, fieldNamesCount, fieldNames,
				writeCriterion, recursionMode);
			string sstring = stringStream.str();
			*memoryBlockLength = static_cast<unsigned int>(sstring.size());
			*memoryBlock = duplicate_string(sstring.c_str());
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
	return (return_code);
}
