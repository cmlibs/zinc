/*******************************************************************************
FILE : finite_element_region.h

LAST MODIFIED : 8 August 2006

DESCRIPTION :
Object comprising a single finite element mesh including nodes, elements and
finite element fields defined on or interpolated over them.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_REGION_H)
#define FINITE_ELEMENT_REGION_H

#include "finite_element/finite_element.h"
#include "general/callback.h"
#include "general/change_log.h"
#include "general/object.h"
#include "region/cmiss_region.h"

/*
Global types
------------
*/

/**
 * A set of nodes/datapoints in the FE_region.
 */
class FE_nodeset
{
	FE_region *fe_region; // not accessed
	cmzn_field_domain_type domainType;
	struct LIST(FE_node) *nodeList;
	struct LIST(FE_node_field_info) *node_field_info_list;
	struct FE_node_field_info *last_fe_node_field_info;
	// nodes added, removed or otherwise changed
	struct CHANGE_LOG(FE_node) *fe_node_changes;
	int next_fe_node_identifier_cache;
	int access_count;

	FE_nodeset(FE_region *fe_region);

	~FE_nodeset();

public:

	static FE_nodeset *create(FE_region *fe_region)
	{
		if (fe_region)
			return new FE_nodeset(fe_region);
		return 0;
	}

	FE_nodeset *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_nodeset *&nodeset)
	{
		if (nodeset)
		{
			--(nodeset->access_count);
			if (nodeset->access_count <= 0)
				delete nodeset;
			nodeset = 0;
		}
	}

	void createChangeLog();

	struct CHANGE_LOG(FE_node) *extractChangeLog();

	struct CHANGE_LOG(FE_node) *getChangeLog()
	{
		return this->fe_node_changes;
	}

	bool containsNode(FE_node *node);

	struct LIST(FE_node) *createRelatedNodeList();

	void detach_from_FE_region();

	struct LIST(FE_node_field_info) *get_FE_node_field_info_list_private()
	{
		return this->node_field_info_list;
	}

	struct FE_node_field_info *get_FE_node_field_info(int number_of_values,
		struct LIST(FE_node_field) *fe_node_field_list);
	int get_FE_node_field_info_adding_new_field(
		struct FE_node_field_info **node_field_info_address,
		struct FE_node_field *new_node_field, int new_number_of_values);
	struct FE_node_field_info *clone_FE_node_field_info(
		struct FE_node_field_info *fe_node_field_info);
	int remove_FE_node_field_info(struct FE_node_field_info *fe_node_field_info);

	FE_region *get_FE_region()
	{
		return this->fe_region;
	}

	struct LIST(FE_node) *getNodeList()
	{
		return this->nodeList;
	}

	// @return  Non-accessed node
	FE_node *findNodeByIdentifier(int identifier)
	{
		return FIND_BY_IDENTIFIER_IN_LIST(FE_node,cm_node_identifier)(identifier, this->nodeList);
	}

	cmzn_field_domain_type getFieldDomainType() const
	{
		return this->domainType;
	}

	void setFieldDomainType(cmzn_field_domain_type domainTypeIn)
	{
		this->domainType = domainTypeIn;
	}

	void nodeChange(FE_node *node, CHANGE_LOG_CHANGE(FE_node) change, FE_node *field_info_node);
	void nodeFieldChange(FE_node *node, FE_field *fe_field);
	void nodeIdentifierChange(FE_node *node);
	void nodeRemovedChange(FE_node *node);

	void clear();
	int change_FE_node_identifier(struct FE_node *node, int new_identifier);
	FE_node *get_or_create_FE_node_with_identifier(int identifier);
	int get_next_FE_node_identifier(int start_identifier);
	int undefine_FE_field_in_FE_node_list(struct FE_field *fe_field,
		struct LIST(FE_node) *fe_node_list, int *number_in_elements_address);
	struct FE_node *create_FE_node_copy(int identifier, struct FE_node *source);
	struct FE_node *merge_FE_node(struct FE_node *node);
	int merge_FE_node_existing(struct FE_node *destination, struct FE_node *source);
	int for_each_FE_node(LIST_ITERATOR_FUNCTION(FE_node) iterator_function, void *user_data);
	cmzn_nodeiterator_id createNodeiterator();
	int remove_FE_node(struct FE_node *node);
	int remove_FE_node_list(struct LIST(FE_node) *node_list);
	int get_last_FE_node_identifier();
	int get_number_of_FE_nodes();
	bool FE_field_has_multiple_times(struct FE_field *fe_field);
	void list_btree_statistics();
	bool is_FE_field_in_use(struct FE_field *fe_field);
	bool canMerge(FE_nodeset &source);
	int merge(FE_nodeset &source, FE_region *target_root_fe_region);
};

/**
 * Internal object containing finite element fields, nodes, elements, datapoints.
 */
struct FE_region;

/**
 * Structure describing FE_region field, node and element changes, for passing
 * back to owner region.
 */
class FE_region_changes
{
	struct CHANGE_LOG(FE_node) *fe_node_changes[2];
	struct CHANGE_LOG(FE_field) *fe_field_changes;
	struct CHANGE_LOG(FE_element) *fe_element_changes[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int access_count;

	FE_region_changes(struct FE_region *fe_region);
	~FE_region_changes();

public:

	static FE_region_changes *create(struct FE_region *fe_region)
	{
		if (fe_region)
			return new FE_region_changes(fe_region);
		return 0;
	}

	FE_region_changes *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_region_changes *&changes)
	{
		if (changes)
		{
			--(changes->access_count);
			if (changes->access_count <= 0)
				delete changes;
			changes = 0;
		}
	}

	struct CHANGE_LOG(FE_element) *getElementChanges(int dimension)
	{
		if ((1 <= dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return this->fe_element_changes[dimension - 1];
		return 0;
	}

	struct CHANGE_LOG(FE_field) *getFieldChanges()
	{
		return this->fe_field_changes;
	}

	struct CHANGE_LOG(FE_node) *getNodeChanges(cmzn_field_domain_type domainType)
	{
		if (CMZN_FIELD_DOMAIN_TYPE_NODES == domainType)
			return this->fe_node_changes[0];
		if (CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS == domainType)
			return this->fe_node_changes[1];
		return 0;
	}

	bool elementOrParentChanged(FE_element *element);

};

/*
Global macros
-------------
*/

#define FE_region_FE_object_method_class( object_type ) FE_region_FE_object_method_class_FE_ ## object_type \

#define DEFINE_FE_region_FE_object_method_class( object_type )	\
class FE_region_FE_object_method_class( object_type ) \
{ \
 public: \
	 typedef int change_log_change_object_type; \
	 typedef struct CHANGE_LOG(FE_ ## object_type) change_log_object_type;	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_unchanged = \
			CHANGE_LOG_OBJECT_UNCHANGED(FE_ ## object_type);	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_added = \
			CHANGE_LOG_OBJECT_ADDED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_removed = \
			CHANGE_LOG_OBJECT_REMOVED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_identifier_changed = \
			CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_ ## object_type);	\
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_not_identifier_changed = \
			CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_object_changed = \
			CHANGE_LOG_OBJECT_CHANGED(FE_ ## object_type); \
	 static const enum CHANGE_LOG_CHANGE(FE_ ## object_type) change_log_related_object_changed = \
			CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_ ## object_type); \
	 static inline FE_ ## object_type* string_to_object(struct FE_region *fe_region, const char *string) \
	 { \
			return FE_region_ ## object_type ## _string_to_FE_ ## object_type(fe_region, string); \
	 } \
\
	static inline FE_ ## object_type* get_first_object_that(struct FE_region *fe_region, \
			 LIST_CONDITIONAL_FUNCTION(FE_ ## object_type) *conditional_function, \
			 void *user_data_void) \
	 { \
			return FE_region_get_first_FE_ ## object_type ## _that(fe_region, \
				 conditional_function, user_data_void); \
	 } \
\
	 static inline int FE_region_contains_object(struct FE_region *fe_region, struct FE_## object_type *object) \
	 { \
			return FE_region_contains_FE_ ## object_type(fe_region, object); \
	 } \
\
	 static inline int FE_object_to_string(struct FE_## object_type *object, char **string) \
	 {	\
			return FE_ ##object_type ## _to_ ##object_type## _string(object, string); \
	 } \
\
	 static inline change_log_object_type *get_object_changes(struct FE_region_changes *changes) \
	 { \
			return changes->fe_ ## object_type ## _changes; \
	 } \
\
	 static inline int change_log_query(struct CHANGE_LOG(FE_ ##object_type) *change_log, \
			struct FE_ ##object_type *object, int *change_address )	\
	 { \
			return CHANGE_LOG_QUERY(FE_ ##object_type)( \
				 change_log, object, change_address); \
	 } \
\
};

/*
Global functions
----------------
*/

/**
 * Creates a struct FE_region owning sets of nodes, elements and datapoints.
 * If <basis_manager> or <element_shape_list> are not supplied then new ones
 * are created for this region.
 */
struct FE_region *FE_region_create(struct MANAGER(FE_basis) *basis_manager,
	struct LIST(FE_element_shape) *element_shape_list);

PROTOTYPE_OBJECT_FUNCTIONS(FE_region);

/**
 * Increments the change counter in <region>. No update callbacks will occur until
 * change counter is restored to zero by calls to FE_region_end_change.
 */
int FE_region_begin_change(struct FE_region *fe_region);

/**
 * Decrements the change counter in <region>. No update callbacks occur until
 * the change counter is restored to zero by this function.
 */
int FE_region_end_change(struct FE_region *fe_region);

/**
 * Return true if the fe_field's owning fe_region is currently caching changes
 * and this fe_field has changes that affect its values.
 */
bool FE_field_has_cached_changes(FE_field *fe_field);

/**
 * Removes all the fields, nodes and elements from <fe_region>.
 * Note this function uses FE_region_begin/end_change so it sends a single change
 * message if not already in the middle of changes.
 */
int FE_region_clear(struct FE_region *fe_region);

/**
 * Internal function for getting pointer to element change log.
 * Warning; do not hold beyond immediate function as logs are constantly
 * recreated.
 */
struct CHANGE_LOG(FE_element) *FE_region_get_FE_element_changes(
	struct FE_region *fe_region, int dimension);

/**
 * Internal function for getting pointer to field change log.
 * Warning; do not hold beyond immediate function as logs are constantly
 * recreated.
 */
struct CHANGE_LOG(FE_field) *FE_region_get_FE_field_changes(
	struct FE_region *fe_region);

/**
 * @return  Non-accessed field of that name, or 0 without error if not found.
 */
struct FE_field *FE_region_get_FE_field_from_name(struct FE_region *fe_region,
	const char *field_name);

/**
 * Safely change the name of field in fe_region to new_name, and inform clients.
 *
 * @param fe_region  The owning region of the field.
 * @param field  The field to rename.
 * @param new_name  The new name of the field. Must be unique in FE_region.
 * @return  1 on success, 0 if failed.
 */
int FE_region_set_FE_field_name(struct FE_region *fe_region,
	struct FE_field *field, const char *new_name);

/**
 * @returns  Boolean true if field is in/from fe_region, otherwise false.
 */
bool FE_region_contains_FE_field(struct FE_region *fe_region,
	struct FE_field *field);

/**
 * Returns the first field in <fe_region> that satisfies <conditional_function>
 * with <user_data_void>.
 * A NULL <conditional_function> returns the first FE_field in <fe_region>, if any.
 * @return  Non-accessed field, or 0 if not found.
 */
struct FE_field *FE_region_get_first_FE_field_that(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function,
	void *user_data_void);

/**
 * Calls <iterator_function> with <user_data> for each FE_field in <region>.
 */
int FE_region_for_each_FE_field(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_field) iterator_function, void *user_data);

/**
 * Returns the number of FE_fields in <fe_region>.
 */
int FE_region_get_number_of_FE_fields(struct FE_region *fe_region);

/***************************************************************************//**
 * Returns an FE_field with the given <name> merged into <fe_region> and with
 * the given properties. If a field of the given <name> already exists, checks
 * that it has the same properties -- if not an error is reported. If no field
 * of that name exists, one is created and FE_region_merge_FE_field called for
 * it. Hence, this function may result in change messages being sent, so use
 * begin/end change if several calls are to be made.
 * Field must be of FE_field_type GENERAL_FE_FIELD.
 * Field properties other than those listed are ignored in comparisons and
 * left at their default values.
 */
struct FE_field *FE_region_get_FE_field_with_general_properties(
	struct FE_region *fe_region, const char *name, enum Value_type value_type,
	int number_of_components);

/***************************************************************************//**
 * Returns an FE_field with the given <name> merged into <fe_region> and with
 * the given properties. If a field of the given <name> already exists, checks
 * that it has the same properties -- if not an error is reported. If no field
 * of that name exists, one is created and FE_region_merge_FE_field called for
 * it. Hence, this function may result in change messages being sent, so use
 * begin/end change if several calls are to be made.
 */
struct FE_field *FE_region_get_FE_field_with_properties(
	struct FE_region *fe_region, const char *name, enum FE_field_type fe_field_type,
	struct FE_field *indexer_field, int number_of_indexed_values,
	enum CM_field_type cm_field_type, struct Coordinate_system *coordinate_system,
	enum Value_type value_type, int number_of_components, char **component_names,
	int number_of_times, enum Value_type time_value_type,
	struct FE_field_external_information *external);

/**
 * Checks <fe_field> is compatible with <fe_region> and any existing FE_field
 * using the same identifier, then merges it into <fe_region>.
 * If no FE_field of the same identifier exists in FE_region, <fe_field> is added
 * to <fe_region> and returned by this function, otherwise changes are merged into
 * the existing FE_field and it is returned.
 * A NULL value is returned on any error.
 */
struct FE_field *FE_region_merge_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field);

/**
 * Returns true if field is defined on any nodes or element in fe_region.
 */
bool FE_region_is_FE_field_in_use(struct FE_region *fe_region,
	struct FE_field *fe_field);

int FE_region_remove_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 3 March 2003

DESCRIPTION :
Removes <fe_field> from <fe_region>.
Fields can only be removed if not defined on any nodes and element in
<fe_region>.
==============================================================================*/

struct Set_FE_field_conditional_FE_region_data
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
User data structure passed to set_FE_field_conditional_FE_region.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function;
	void *user_data;
	struct FE_region *fe_region;
}; /* struct Set_FE_field_conditional_FE_region_data */

/**
 * Returns true if field is defined with multiple times on any node or element.
 */
int FE_region_FE_field_has_multiple_times(struct FE_region *fe_region,
	struct FE_field *fe_field);

/**
 * Returns an <fe_field> which the <fe_region> considers to be its default
 * coordinate field. Generally the first field with the type_coordinate flag
 * set found alphabetically.
 */
struct FE_field *FE_region_get_default_coordinate_FE_field(
	struct FE_region *fe_region);

/**
 * Returns the FE_basis_manager used for bases in this <fe_region>.
 */
struct MANAGER(FE_basis) *FE_region_get_basis_manager(
	struct FE_region *fe_region);

struct LIST(FE_field) *FE_region_get_FE_field_list(struct FE_region *fe_region);

/**
 * Returns the LIST of FE_element_shapes used for elements in this <fe_region>.
 */
struct LIST(FE_element_shape) *FE_region_get_FE_element_shape_list(
	struct FE_region *fe_region);

/**
 * Gets <fe_region> to change the identifier of <element> to <new_identifier>.
 * Fails if the identifier is already in use by an element of the same dimension
 * in the FE_region.
 */
int FE_region_change_FE_element_identifier(struct FE_region *fe_region,
	struct FE_element *element, int new_identifier);

/***************************************************************************//**
 * Returns the element of the supplied dimension with the supplied identifier
 * in fe_region, or NULL without error if none.
 */
struct FE_element *FE_region_get_FE_element_from_identifier(
	struct FE_region *fe_region, int dimension, int identifier);

/***************************************************************************//**
 * Returns the element identified by <cm> in <fe_region>, or NULL without error
 * if no such element found.
 * Assumes CM_ELEMENT = 3-D, CM_FACE = 2-D, CM_LINE = 1-D.
 * Deprecated.
 * @see FE_region_get_FE_element_from_identifier
 */
struct FE_element *FE_region_get_FE_element_from_identifier_deprecated(
	struct FE_region *fe_region, struct CM_element_information *identifier);

/***************************************************************************//**
 * Returns the top-level element of the highest dimension with the supplied
 * identifier in fe_region, or NULL without error if none.
 */
struct FE_element *FE_region_get_top_level_FE_element_from_identifier(
	struct FE_region *fe_region, int identifier);

/**
 * Convenience function returning an existing element of dimension and
 * identifier from fe_region or any of its ancestors. If no existing element is
 * found, a new element is created with the given identifier and the supplied
 * shape, or unspecified shape of the given dimension if no shape provided.
 * If the returned element is not already in fe_region it is merged.
 * It is expected that the calling function has wrapped calls to this function
 * with FE_region_begin/end_change.
 */
struct FE_element *FE_region_get_or_create_FE_element_with_identifier(
	struct FE_region *fe_region, int dimension, int identifier,
	struct FE_element_shape *element_shape);

/**
 * @return  Non-accessed fe_nodeset or 0 if not found.
 */
FE_nodeset *FE_region_find_FE_nodeset_by_field_domain_type(
	struct FE_region *fe_region, enum cmzn_field_domain_type domain_type);

/**
 * Find handle to the mesh scale factor set of the given name, if any.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @param fe_region  The FE_region to query.
 * @param name  The name of the scale factor set. 
 * @return  Handle to the scale factor set, or 0 if none.
 * Up to caller to destroy handle.
 */
cmzn_mesh_scale_factor_set *FE_region_find_mesh_scale_factor_set_by_name(
	struct FE_region *fe_region, const char *name);

/**
 * Create a mesh scale factor set. The new set is given a unique name in the
 * fe_region, which can be changed.
 * Scale factors are stored in elements under a scale factor set.
 *
 * @param fe_region  The FE_region to modify.
 * @return  Handle to the new scale factor set, or 0 on failure. Up to caller
 * to destroy the returned handle.
 */
cmzn_mesh_scale_factor_set *FE_region_create_mesh_scale_factor_set(
	struct FE_region *fe_region);

/***************************************************************************//**
 * Returns the next unused element number for elements of <dimension> in
 * <fe_region> starting from <start_identifier>.
 * Search is performed on the ultimate master FE_region for <fe_region> since
 * it owns the FE_element namespace.
 * @param start_identifier  Minimum number for new identifier. Pass 0 to give
 * the first available number >= 1.
 */
int FE_region_get_next_FE_element_identifier(struct FE_region *fe_region,
	int dimension, int start_identifier);

/***************************************************************************//**
 * Returns the number of elements of all dimensions in <fe_region>.
 * Prefer FE_region_get_number_of_FE_elements_of_dimension.
 */
int FE_region_get_number_of_FE_elements_all_dimensions(
	struct FE_region *fe_region);

/***************************************************************************//**
 * Returns the number of elements of given dimensions in <fe_region>.
 */
int FE_region_get_number_of_FE_elements_of_dimension(
	struct FE_region *fe_region, int dimension);

/***************************************************************************//**
 * @return  The highest dimension of element in the region, or zero if none.
 */
int FE_region_get_highest_dimension(struct FE_region *fe_region);

int FE_region_contains_FE_element(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 9 December 2002

DESCRIPTION :
Returns true if <element> is in <fe_region>.
==============================================================================*/

int FE_region_contains_FE_element_conditional(struct FE_element *element,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
FE_element conditional function version of FE_region_contains_FE_element.
Returns true if <element> is in <fe_region>.
==============================================================================*/

int FE_element_is_not_in_FE_region(struct FE_element *element,
	void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 3 April 2003

DESCRIPTION :
Returns true if <element> is not in <fe_region>.
==============================================================================*/

/**
 * Checks <fe_region> contains <element>. If <fe_field> is already defined on it,
 * returns successfully, otherwise defines the field at the element using the
 * <components>.
 * Should place multiple calls to this function between begin_change/end_change.
 */
int FE_region_define_FE_field_at_element(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field,
	struct FE_element_field_component **components);

/**
 * Checks the source element is compatible with region & that there is no
 * existing element of supplied identifier, then creates element of that
 * identifier as a copy of source and adds it to the fe_region.
 *
 * @param fe_region  The region to create the element copy in.
 * @param identifier  Non-negative integer identifier of new element, or -1 to
 * automatically generate (starting at 1). Fails if supplied identifier already
 * used by an existing element.
 * @return  New element (non-accessed), or NULL if failed.
 */
struct FE_element *FE_region_create_FE_element_copy(struct FE_region *fe_region,
	int identifier, struct FE_element *source);

/**
 * Checks <element> is compatible with <fe_region> and any existing FE_element
 * using the same identifier, then merges it into <fe_region>.
 * If no FE_element of the same identifier exists in FE_region, <element> is
 * added to <fe_region> and returned by this function, otherwise changes are
 * merged into the existing FE_element and it is returned.
 * During the merge, any new fields from <element> are added to the existing
 * element of the same identifier. Where those fields are already defined on the
 * existing element, the existing structure is maintained, but the new values
 * are added from <element>. Fails if fields are not consistently defined.
 *
 * @return  On success, the element from the region which differs from the
 * element argument if modifying an existing element, or NULL on error.
 */
struct FE_element *FE_region_merge_FE_element(struct FE_region *fe_region,
	struct FE_element *element);

/**
 * Merges field changes from source into destination element. Checks both
 * elements are compatible with the same master region / fe_region, and adds
 * destination to fe_region if it is not the master.
 *
 * @return  1 on success, 0 on error.
 */
int FE_region_merge_FE_element_existing(struct FE_region *fe_region,
	struct FE_element *destination, struct FE_element *source);

/***************************************************************************//**
 * Sets up <fe_region> to automatically define faces on any elements merged into
 * it, and their faces recursively.
 * @param face_dimension  Dimension of faces to define, 2, 1 or 0 for nodes.
 * Special value -1 defines faces of all dimensions.
 * Call FE_region_end_define_faces as soon as face definition is finished.
 * Should put face definition calls between calls to begin_change/end_change.
 */
int FE_region_begin_define_faces(struct FE_region *fe_region, int face_dimension);

/***************************************************************************//**
 * Ends face definition in <fe_region>. Cleans up internal cache.
 */
int FE_region_end_define_faces(struct FE_region *fe_region);

int FE_region_merge_FE_element_and_faces_and_nodes(struct FE_region *fe_region,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 14 May 2003

DESCRIPTION :
Version of FE_region_merge_FE_element that merges not only <element> into
<fe_region> but any of its faces that are defined.
<element> and any of its faces may already be in <fe_region>.
Also merges nodes referenced directly by <element> and its parents, if any.

- MUST NOT iterate over the list of elements in a region to add or define faces
as the list will be modified in the process; copy the list and iterate over
the copy.

FE_region_begin/end_change are called internally to reduce change messages to
one per call. User should place calls to the begin/end_change functions around
multiple calls to this function.

If calls to this function are placed between FE_region_begin/end_define_faces,
then any missing faces are created and also merged into <fe_region>.
Function ensures that elements share existing faces and lines in preference to
creating new ones if they have matching shape and nodes.

???RC Can only match faces correctly for coordinate fields with standard node
to element maps and no versions. A grid-based coordinate field would fail
utterly since it has no nodes. A possible future solution for all cases is to
match the geometry exactly either by using the FE_element_field_values
(coefficients of the monomial basis functions), although there is a problem with
xi-directions not matching up, or actual centre positions of the face being a
trivial rejection, narrowing down to a single face or list of faces to compare
against.
==============================================================================*/

/***************************************************************************//**
 * Ensures for elements of every dimension > 1 that there are face and line
 * elements of lower dimension in the region.
 */
int FE_region_define_faces(struct FE_region *fe_region);

/**
 * Removes <element> and all its faces that are not shared with other elements
 * from <fe_region>.
 * FE_region_begin/end_change are called internally to reduce change messages to
 * one per call. User should place calls to the begin/end_change functions
 * around multiple calls to this function.
 * Can only remove faces and lines if there are no parents in this fe_region.
 * This function is recursive.
 */
int FE_region_remove_FE_element(struct FE_region *fe_region,
	struct FE_element *element);

/**
 * Attempts to removes all the elements in <element_list>, and all their faces
 * that are not shared with other elements from <fe_region>.
 * FE_region_begin/end_change are called internally to reduce change messages to
 * one per call. User should place calls to the begin/end_change functions
 * around multiple calls to this function.
 * Can only remove faces and lines if there are no parents in this fe_region.
 * On return, <element_list> will contain all the elements that are still in
 * <fe_region> after the call.
 * A true return code is only obtained if all elements from <element_list> are
 * removed.
 */
int FE_region_remove_FE_element_list(struct FE_region *fe_region,
	struct LIST(FE_element) *element_list);

int FE_region_element_or_parent_has_field(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 12 November 2002

DESCRIPTION :
Returns true if <element> is in <fe_region> and has <field> defined on it or
any parents also in <fe_region>.
==============================================================================*/

/***************************************************************************//**
 * Returns the first element in <fe_region> that satisfies
 * <conditional_function> with <user_data_void>.
 * A NULL <conditional_function> returns the first FE_element in <fe_region>,
 * if any.
 * This version iterates over elements of all dimensions from highest to lowest.
 * @see FE_region_get_first_FE_element_of_dimension_that
 */
struct FE_element *FE_region_get_first_FE_element_that(
	struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void);

/***************************************************************************//**
 * Returns the first element of <dimension> in <fe_region> that satisfies
 * <conditional_function> with <user_data_void>.
 * A NULL <conditional_function> returns the first FE_element of given dimension
 * in <fe_region>, if any.
 */
struct FE_element *FE_region_get_first_FE_element_of_dimension_that(
	struct FE_region *fe_region, int dimension,
	LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function,
	void *user_data_void);

/***************************************************************************//**
 * Calls <iterator_function> with <user_data> for each FE_element in <region>
 * of all dimensions from highest to lowest.
 */
int FE_region_for_each_FE_element(struct FE_region *fe_region,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data);

/***************************************************************************//**
 * Calls <iterator_function> with <user_data> for each FE_element in <region>
 * of the given dimension.
 * @param dimension  The dimension of the element, at least 1, current max 3.
 */
int FE_region_for_each_FE_element_of_dimension(struct FE_region *fe_region,
	int dimension, LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *user_data);

/***************************************************************************//**
 * Calls <iterator_function> with <iterator_user_data> for each element of
 * all dimensions from highest to lowest in fe_region which passes the
 * <conditional_function> with <conditional_user_data> argument.
 */
int FE_region_for_each_FE_element_conditional(struct FE_region *fe_region,
	LIST_CONDITIONAL_FUNCTION(FE_element) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *iterator_user_data);

/***************************************************************************//**
 * Calls <iterator_function> with <iterator_user_data> for each element of
 * all given dimensions in fe_region which passes the <conditional_function>
 * with <conditional_user_data> argument.
 */
int FE_region_for_each_FE_element_of_dimension_conditional(
	struct FE_region *fe_region, int dimension,
	LIST_CONDITIONAL_FUNCTION(FE_element) conditional_function,
	void *conditional_user_data,
	LIST_ITERATOR_FUNCTION(FE_element) iterator_function,
	void *iterator_user_data);

/***************************************************************************//**
 * Create an element iterator object for iterating through the elements of the
 * given dimension in the fe_region, which are ordered from lowest to highest
 * identifier. The iterator initially points at the position before the first
 * element.
 *
 * @param fe_region  The region whose elements are to be iterated over.
 * @param dimension  The dimension of elements to iterate over.
 * @return  Handle to element_iterator at position before first, or NULL if error.
 */
cmzn_elementiterator_id FE_region_create_elementiterator(
	struct FE_region *fe_region, int dimension);

/***************************************************************************//**
 * @return  Element from highest dimension mesh in region with identifier equal
 * to number in string name.
 */
struct FE_element *FE_region_element_string_to_FE_element(
	struct FE_region *fe_region, const char *name);

/***************************************************************************//**
 * Converts name string of format "CM_ELEMENT_TYPE NUMBER" to an element
 * identifier and returns the element in <fe_region> with that identifier.
 */
struct FE_element *FE_region_any_element_string_to_FE_element(
	struct FE_region *fe_region, const char *name);

/**
 * Smooths node-based <fe_field> over its nodes and elements in <fe_region>.
 */
int FE_region_smooth_FE_field(struct FE_region *fe_region,
	struct FE_field *fe_field, FE_value time);

/**
 * Finds or creates a struct FE_time_sequence in <fe_region> with the given
 * <number_of_times> and <times>.
 */
struct FE_time_sequence *FE_region_get_FE_time_sequence_matching_series(
	struct FE_region *fe_region, int number_of_times, const FE_value *times);

/**
 * Finds or creates a struct FE_time_sequence in <fe_region> which has the list
 * of times formed by merging the two time_sequences supplied.
 */
struct FE_time_sequence *FE_region_get_FE_time_sequence_merging_two_time_series(
	struct FE_region *fe_region, struct FE_time_sequence *time_sequence_one,
	struct FE_time_sequence *time_sequence_two);

/**
 * Finds or creates a struct FE_basis in <fe_region> with the given basis_type.
 */
struct FE_basis *FE_region_get_FE_basis_matching_basis_type(
	struct FE_region *fe_region, int *basis_type);

/**
 * Sets the owning cmiss_region for this fe_region. Can also clear it.
 * Private - only for use by cmzn_region on construction and destruction!
 */
void FE_region_set_cmzn_region_private(struct FE_region *fe_region,
	struct cmzn_region *cmiss_region);

/**
 * @return  The cmzn_region containing this fe_region. Not accessed.
 */
struct cmzn_region *FE_region_get_cmzn_region(struct FE_region *fe_region);

/**
 * Check that fields and other object definitions in source FE_region are
 * properly defined and compatible with definitions in target FE_region.
 * Converts legacy field representations e.g. read from older EX files, hence
 * source_fe_region can be modified, and function fails if conversion is not
 * possible.
 * @param target_fe_region  Optional target/global FE_region to check
 * compatibility with. Omit to confirm conversion of legacy field
 * representations only.
 * @param source_fe_region  Source region to check. Can be modified.
 * @return  True if compatible and conversions successful, false if failed or
 * source region is missing.
 */
bool FE_region_can_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region);

/**
 * Merges into <target_fe_region> the fields, nodes and elements from
 * <source_fe_region>. Note that <source_fe_region> is left in a polluted state
 * containing objects that partly belong to the <target_fe_region> and partly to
 * itself. Currently it needs to be left around for the remainder of the merge
 * up and down the region graph, but it needs to be destroyed as soon as possible.
 * @param target_root_fe_region  Target / global root matching source root for
 * embedding data. Possibly unnecessary.
 */
int FE_region_merge(struct FE_region *target_fe_region,
	struct FE_region *source_fe_region, struct FE_region *target_root_fe_region);

struct LIST(FE_element) *FE_region_create_related_element_list_for_dimension(
	struct FE_region *fe_region, int dimension);

/***************************************************************************//**
 * List statistics about btree structures storing a region's nodes and elements.
 */
void FE_region_list_btree_statistics(struct FE_region *fe_region);

#endif /* !defined (FINITE_ELEMENT_REGION_H) */
