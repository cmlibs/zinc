/*******************************************************************************
FILE : finite_element_private.h

LAST MODIFIED : 30 May 2003

DESCRIPTION :
Private interfaces to finite element objects to be included only by privileged
modules such as finite_element_region.c.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_PRIVATE_H)
#define FINITE_ELEMENT_PRIVATE_H

#include "opencmiss/zinc/types/nodeid.h"
#include "opencmiss/zinc/status.h"
#include "finite_element/finite_element.h"
#include "finite_element/node_field_template.hpp"
#include "general/indexed_list_private.h"
#include "general/indexed_list_stl_private.hpp"
#include "general/list.h"
#include "general/object.h"

/*
Global types
------------
*/

struct FE_node_field_info;
/*******************************************************************************
LAST MODIFIED : 4 October 2002

DESCRIPTION :
Structure describing how fields and their values and derivatives are stored in
cmzn_node structures.
==============================================================================*/

DECLARE_LIST_TYPES(FE_node_field_info);

struct FE_element_type_node_sequence_identifier;
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
See struct FE_element_type_node_sequence.
==============================================================================*/

struct FE_element_type_node_sequence;
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Structure for storing an element with its identifier being its cm_type and the
number and list - in ascending order - of the nodes referred to by the default
coordinate field of the element. Indexed lists, indexed using function
compare_FE_element_type_node_sequence_identifier ensure that recalling a line or
face with the same nodes is extremely rapid. FE_mesh
uses them to find faces and lines for elements without them, if they exist.
==============================================================================*/

DECLARE_LIST_TYPES(FE_element_type_node_sequence);

/*
Private functions
-----------------
*/

PROTOTYPE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(FE_field,name);

struct FE_field_info *CREATE(FE_field_info)(struct FE_region *fe_region);
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

int DESTROY(FE_field_info)(
	struct FE_field_info **fe_field_info_address);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Destroys the FE_field_info at *<field_info_address>. Frees the
memory for the information and sets <*field_info_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_field_info);

int FE_field_info_clear_FE_region(struct FE_field_info *field_info);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Clears the pointer to FE_region in <field_info>.
Private function only to be called by destroy_FE_region.
==============================================================================*/

int FE_field_set_FE_field_info(struct FE_field *fe_field,
	struct FE_field_info *fe_field_info);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Changes the FE_field_info at <fe_field> to <fe_field_info>.
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/

int FE_field_set_indexer_field(struct FE_field *fe_field,
	struct FE_field *indexer_fe_field);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
If <fe_field> is already indexed, substitutes <indexer_fe_field>.
Does not change any of the values currently stored in <fe_field>
Used to merge indexed fields into different FE_regions; should not be used for
any other purpose.
==============================================================================*/

int FE_field_log_FE_field_change(struct FE_field *fe_field,
	void *fe_field_change_log_void);
/*******************************************************************************
LAST MODIFIED : 30 May 2003

DESCRIPTION :
Logs the field in <fe_field> as RELATED_OBJECT_CHANGED in the
struct CHANGE_LOG(FE_field) pointed to by <fe_field_change_log_void>.
???RC Later may wish to allow more than just RELATED_OBJECT_CHANGED, or have
separate functions for each type.
==============================================================================*/

/**
 * Describes storage of global values and derivatives for a field at a node.
 * Public only to allow efficient conversion of legacy node DOF indexes.
 * @see FE_mesh::checkConvertLegacyNodeParameters
 */
struct FE_node_field
{
	/* the field which this accesses values and derivatives for */
	struct FE_field *field;
	/* an array with <number_of_components> node field templates */
	FE_node_field_template *components;
	/* the time dependence of all components below this point,
	   if it is non-NULL then every value storage must be an array of
	   values that matches this fe_time_sequence */
	struct FE_time_sequence *time_sequence;
	/* the number of structures that point to this node field.  The node field
		cannot be destroyed while this is greater than 0 */
	int access_count;

	const FE_node_field_template *getComponent(int componentIndex) const
	{
		return &this->components[componentIndex];
	}

	FE_node_field(struct FE_field *fieldIn);

	~FE_node_field();

	static FE_node_field *create(struct FE_field *fieldIn);

	/** Clone node field, optionally offseting all component valuesOffsets.
	  * For non-GENERAL_FE_FIELD types, the valuesOffset is not changed.*/
	static FE_node_field *clone(const FE_node_field& source, int deltaValuesOffset = 0);

	FE_node_field *access()
	{
		++this->access_count;
		return this;
	}

	static int deaccess(FE_node_field* &node_field)
	{
		if (!node_field)
		{
			return CMZN_ERROR_ARGUMENT;
		}
		--(node_field->access_count);
		if (node_field->access_count <= 0)
		{
			delete node_field;
		}
		node_field = 0;
		return CMZN_OK;
	}

	void setField(FE_field *fieldIn)
	{
		REACCESS(FE_field)(&this->field, fieldIn);
	}

	/** @return  True if field is multi component with all components defined identically */
	bool isHomogeneousMultiComponent() const
	{
		const int componentCount = get_FE_field_number_of_components(this->field);
		if (componentCount <= 1)
		{
			return false;
		}
		for (int c = 1; c < componentCount; ++c)
		{
			if (!(this->components[c].matches(this->components[0])))
			{
				return false;
			}
		}
		return true;
	}

	/** Returns the maximum total number of field parameters for any component */
	int getMaximumComponentTotalValuesCount() const
	{
		int maximumComponentTotalValuesCount = 0;
		const int componentCount = get_FE_field_number_of_components(this->field);
		for (int c = 0; c < componentCount; ++c)
		{
			if (this->components[c].getTotalValuesCount() > maximumComponentTotalValuesCount)
			{
				maximumComponentTotalValuesCount = this->components[c].getTotalValuesCount();
			}
		}
		return maximumComponentTotalValuesCount;
	}

	/** Returns the total number of field parameters for all components */
	int getTotalValuesCount() const
	{
		int totalValuesCount = 0;
		const int componentCount = get_FE_field_number_of_components(this->field);
		for (int c = 0; c < componentCount; ++c)
		{
			totalValuesCount += this->components[c].getTotalValuesCount();
		}
		return totalValuesCount;
	}

	/** @return  Minimum number of versions stored for valueLabel over all components */
	int getValueMinimumVersionsCount(cmzn_node_value_label valueLabel) const;

};

DECLARE_LIST_TYPES(FE_node_field);

PROTOTYPE_LIST_FUNCTIONS(FE_node_field);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_node_field,field, \
	struct FE_field *);

struct LIST(FE_node_field) *FE_node_field_list_clone_with_FE_field_list(
	struct LIST(FE_node_field) *node_field_list,
	struct LIST(FE_field) *fe_field_list,
	struct FE_time_sequence_package *fe_time_sequence_package);
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Returns a new FE_node_field list that is identical to <node_field_list>
except that it references equivalent same-name fields from <fe_field_list> and
uses FE_time_sequences in <fe_time_sequence_package>.
It is an error if an equivalent FE_field is not found.
==============================================================================*/

/**
 * Creates a struct FE_node_field_info with a pointer to <fe_nodeset>
 * and a copy of the <fe_node_field_list>.
 * If <fe_node_field_list> is omitted, an empty list is assumed.
 * Returned object has an access count of 1.
 * Note:
 * This should only be called by FE_nodeset functions. The returned object is
 * added to the list of FE_node_field_info in the FE_nodeset and 'owned by it'.
 * It maintains a non-ACCESSed pointer to its owning FE_nodeset which the
 * FE_nodeset will clear before it is destroyed.
 */
struct FE_node_field_info *CREATE(FE_node_field_info)(
	FE_nodeset *fe_nodeset, struct LIST(FE_node_field) *fe_node_field_list,
	int number_of_values);

PROTOTYPE_OBJECT_FUNCTIONS(FE_node_field_info);

PROTOTYPE_LIST_FUNCTIONS(FE_node_field_info);

/**
 * Clears the pointer to FE_nodeset in <node_field_info>.
 * Private function only to be called by ~FE_nodeset.
 */
int FE_node_field_info_clear_FE_nodeset(
	struct FE_node_field_info *node_field_info, void *dummy_void);

int FE_node_field_info_has_FE_field(
	struct FE_node_field_info *node_field_info, void *fe_field_void);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Returns true if <node_field_info> has an node field for <fe_field>.
==============================================================================*/

int FE_node_field_info_has_empty_FE_node_field_list(
	struct FE_node_field_info *node_field_info, void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 14 October 2002

DESCRIPTION :
Returns true if <node_field_info> has no node fields.
==============================================================================*/

int FE_node_field_info_has_FE_field_with_multiple_times(
	struct FE_node_field_info *node_field_info, void *fe_field_void);
/*******************************************************************************
LAST MODIFIED: 26 February 2003

DESCRIPTION:
Returns true if <node_field_info> has a node_field that references
<fe_field_void> and that node_field has multiple times.
==============================================================================*/

int FE_node_field_info_has_matching_FE_node_field_list(
	struct FE_node_field_info *node_field_info, void *node_field_list_void);
/*******************************************************************************
LAST MODIFIED : 14 November 2002

DESCRIPTION :
Returns true if <node_field_info> has a FE_node_field_list containing all the
same FE_node_fields as <node_field_list>.
==============================================================================*/

struct LIST(FE_node_field) *FE_node_field_info_get_node_field_list(
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Returns the node field list contained in the <node_field_info>.
==============================================================================*/

int FE_node_field_info_add_node_field(
	struct FE_node_field_info *fe_node_field_info, 
	struct FE_node_field *new_node_field, int new_number_of_values);
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Adds the <new_node_field> to the list in the <fe_node_field_info> and updates the
<new_number_of_values>.  This should only be done if object requesting the change
is known to be the only object using this field info.
==============================================================================*/

int FE_node_field_info_used_only_once(
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 24 August 2005

DESCRIPTION :
Returns 1 if the <node_field_info> access count indicates that it is being used
by only one external object.
==============================================================================*/

int FE_node_field_info_get_number_of_values(
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Returns the number of values expected for the <node_field_info>.
==============================================================================*/

int FE_node_field_info_log_FE_field_changes(
	struct FE_node_field_info *fe_node_field_info,
	struct CHANGE_LOG(FE_field) *fe_field_change_log);
/*******************************************************************************
LAST MODIFIED : 14 February 2003

DESCRIPTION :
Marks each FE_field in <fe_node_field_info> as RELATED_OBJECT_CHANGED
in <fe_field_change_log>.
==============================================================================*/

PROTOTYPE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(cmzn_node,cm_node_identifier);

/**
 * Creates and returns a non-global node to be used as a template for
 * creating other nodes in the supplied mesh.
 * @param node_field_info  A struct FE_node_field_info linking to the
 * mesh. Should have no fields.
 * @return  Accessed node, or 0 on error. Do not merge into FE_nodeset!
 */
cmzn_node *create_template_FE_node(FE_node_field_info *node_field_info);

/**
 * Creates and returns a non-global node with the specified index,
 * that is a copy of the supplied template node i.e. with all fields
 * and values from it.
 * The returned node is ready to be added into the FE_mesh the
 * template node was created by.
 * Shape is not set for new node as mapped in mesh.
 * Faces are not copied from the template node.
 * @param index  Index of node in mesh, or DS_LABEL_INDEX_INVALID if used
 * as a non-global template node i.e. when called from
 * FE_node_template::FE_node_template().
 * @param template_node  node to copy.
 * @return  Accessed node, or 0 on error.
 */
cmzn_node *create_FE_node_from_template(DsLabelIndex index, cmzn_node *template_node);

/**
 * Clear content of node and disconnect it from owning nodeset.
 * Use when removing node from nodeset or deleting nodeset to safely orphan any
 * externally accessed nodes.
 */
void FE_node_invalidate(cmzn_node *node);

struct FE_node_field_info *FE_node_get_FE_node_field_info(cmzn_node *node);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns the FE_node_field_info from <node>. Must not be modified!
==============================================================================*/

int FE_node_set_FE_node_field_info(cmzn_node *node,
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 24 February 2003

DESCRIPTION :
Changes the FE_node_field_info at <node> to <fe_node_field_info>.
Note it is very important that the old and the new FE_node_field_info structures
describe the same data layout in the nodal values_storage!
Private function only to be called by FE_region when merging FE_regions!
==============================================================================*/

/**
 * Get the node_field describing parameter storage for field at node.
 * @param node  The node to query.
 * @param field  The field to query.
 * @return  Non-accessed node_field, or 0 if not defined. Use immediately,
 * access or copy as may be destroyed at any time.
 */
const FE_node_field *cmzn_node_get_FE_node_field(cmzn_node *node,
	struct FE_field *field);

/**
 * Merges the fields from <source> into <destination>. Existing fields in the
 * <destination> keep the same node field description as before with new field
 * storage following them. Where existing fields in <destination> are passed in
 * <source>, values from <source> take precedence, but the node field structure
 * remains unchanged.
 * Function is atomic; <destination> is unchanged if <source> cannot be merged.
 * @param optimised_merge  If non-zero, time arrays are allowed to be
 * transferred from source instead of copying. Use ONLY for external region merge
 * during I/O.
 */
int merge_FE_node(cmzn_node *destination, cmzn_node *source, int optimised_merge = 0);

/**
 * Create structure storing an element with its identifier being its cm_type
 * and the number and list - in ascending order - of the nodes referred to by
 * the default coordinate field of the element. Indexed lists, indexed using
 * function compare_FE_element_type_node_sequence_identifier ensure that
 * recalling a line or face with the same nodes is extremely rapid.
 * FE_mesh uses them to find faces and lines for elements without them,
 * if they exist.
 * @param face_number  If non-negative, calculate nodes for face number of
 * element, as if the face element were supplied to this function. If so, the
 * returned structure accesses the supplied element. It is assumed this is
 * only used when creating new faces, and the face should be set as soon as
 * created.
 * @param result  Result OK on success, otherwise ERROR_NOT_FOUND if
 * nodes not obtainable, otherwise any other error.
 * @return  On success, newly created sequence.
 */
struct FE_element_type_node_sequence *CREATE(FE_element_type_node_sequence)(
	int &result, struct FE_element *element, int face_number = -1);

int DESTROY(FE_element_type_node_sequence)(
	struct FE_element_type_node_sequence **element_type_node_sequence_address);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Cleans up memory used by the FE_element_type_node_sequence.
==============================================================================*/

int FE_element_type_node_sequence_is_collapsed(
	struct FE_element_type_node_sequence *element_type_node_sequence);
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns true if the <element_type_node_sequence> represents an element that
has collapsed, ie, is a face with <= 2 unique nodes, or a line with 1 unique
node.
???RC Note that repeated nodes in the face/line are not put in the node_numbers
array twice, facilitating the simple logic in this function. If they were, then
this function would have to be modified extensively.
==============================================================================*/

/**
 * Gets the FE_element from the <element_type_node_sequence>.
 */
struct FE_element *FE_element_type_node_sequence_get_FE_element(
	struct FE_element_type_node_sequence *element_type_node_sequence);

/**
 * Sets the FE_element in the <element_type_node_sequence>. Use only to
 * substitute face element when sequence was created from face number of
 * parent element.
 */
void FE_element_type_node_sequence_set_FE_element(
	struct FE_element_type_node_sequence *element_type_node_sequence,
	struct FE_element *element);

/**
 * Returns existing FE_element_type_node_sequence from list with matching
 * identifier to supplied one.
 */
FE_element_type_node_sequence *FE_element_type_node_sequence_list_find_match(
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list,
	FE_element_type_node_sequence *element_type_node_sequence);

PROTOTYPE_OBJECT_FUNCTIONS(FE_element_type_node_sequence);

PROTOTYPE_LIST_FUNCTIONS(FE_element_type_node_sequence);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_element_type_node_sequence, \
	identifier, FE_element_type_node_sequence_identifier&);

#endif /* !defined (FINITE_ELEMENT_PRIVATE_H) */
