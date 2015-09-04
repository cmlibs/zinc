/**
 * FILE : finite_element_mesh.hpp
 *
 * Class defining a domain consisting of a set of finite elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_MESH_HPP)
#define FINITE_ELEMENT_MESH_HPP

#include "finite_element/finite_element.h"
#include "general/change_log.h"
#include "general/list.h"
#include <vector>

class FE_mesh;

struct cmzn_mesh_scale_factor_set;

/** Handle to a set of scale factors for a mesh.
 * Scale factors are used to scale global field parameters before use.
 * Actual values are stored under this handle in each element. */
typedef struct cmzn_mesh_scale_factor_set *cmzn_mesh_scale_factor_set_id;

/**
 * Identifier of set of scale factors, under which scale factors are stored,
 * e.g. in elements.
 */
struct cmzn_mesh_scale_factor_set
{
private:
	FE_mesh *fe_mesh; // non-accessed pointer to owner
	char *name;
	int access_count;

	cmzn_mesh_scale_factor_set();

	~cmzn_mesh_scale_factor_set();

	cmzn_mesh_scale_factor_set(FE_mesh *fe_meshIn, const char *nameIn);

public:

	static cmzn_mesh_scale_factor_set *create(FE_mesh *fe_meshIn, const char *nameIn)
	{
		return new cmzn_mesh_scale_factor_set(fe_meshIn, nameIn);
	}

	cmzn_mesh_scale_factor_set *access()
	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_mesh_scale_factor_set* &scale_factor_set)
	{
		if (!scale_factor_set)
			return CMZN_ERROR_ARGUMENT;
		--(scale_factor_set->access_count);
		if (scale_factor_set->access_count <= 0)
			delete scale_factor_set;
		scale_factor_set = 0;
		return CMZN_OK;
	}

	/** @return  Internal name, not a copy */
	const char *getName() const
	{
		return this->name;
	}

	int setName(const char *nameIn);
};

/**
 * A set of elements in the FE_region.
 */
class FE_mesh
{
	FE_region *fe_region; // not accessed
	int dimension;

	// set of elements in this mesh
	struct LIST(FE_element) *elementList;
	struct LIST(FE_element_field_info) *element_field_info_list;

	FE_mesh *parentMesh; // not accessed
	FE_mesh *faceMesh; // not accessed

	/* elements added, removed or otherwise changed */
	struct CHANGE_LOG(FE_element) *fe_element_changes;
	/* keep pointer to last element field information so only need to
		 note changed fields when this changes */
	struct FE_element_field_info *last_fe_element_field_info;

	/* information for defining faces */
	/* existence of element_type_node_sequence_list can tell us whether faces
		 are being defined */
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list;
	bool definingFaces;

	/* Keep a record of where we got to searching for valid identifiers.
		We reset the cache if we delete any items */
	int next_fe_element_identifier_cache;

	// scale factor sets in use. Used as identifier for finding scale factor
	// arrays stored with elements 
	std::vector<cmzn_mesh_scale_factor_set*> scale_factor_sets;

	int access_count;

private:

	FE_mesh(FE_region *fe_regionIn, int dimensionIn);

	~FE_mesh();

	struct FE_element_field_info *clone_FE_element_field_info(
		struct FE_element_field_info *fe_element_field_info);

	int find_or_create_face(struct FE_element *parent_element, int face_number);

	int remove_FE_element_private(struct FE_element *element);

public:

	static FE_mesh *create(FE_region *fe_region, int dimension)
	{
		if (fe_region && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return new FE_mesh(fe_region, dimension);
		return 0;
	}

	FE_mesh *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(FE_mesh *&mesh)
	{
		if (mesh)
		{
			--(mesh->access_count);
			if (mesh->access_count <= 0)
				delete mesh;
			mesh = 0;
		}
	}

	void elementChange(FE_element *element, CHANGE_LOG_CHANGE(FE_element) change, FE_element *field_info_element);
	void elementFieldListChange(FE_element *element, CHANGE_LOG_CHANGE(FE_element) change,
		struct LIST(FE_field) *changed_fe_field_list);
	void elementFieldChange(FE_element *element, FE_field *fe_field);
	void elementIdentifierChange(FE_element *element);
	void elementRemovedChange(FE_element *element);

	int getDimension() const
	{
		return this->dimension;
	}

	FE_region *get_FE_region()
	{
		return this->fe_region;
	}

	FE_mesh *getFaceMesh() const
	{
		return this->faceMesh;
	}

	void setFaceMesh(FE_mesh *faceMeshIn)
	{
		this->faceMesh = faceMeshIn;
	}

	FE_mesh *getParentMesh() const
	{
		return this->parentMesh;
	}

	void setParentMesh(FE_mesh *parentMeshIn)
	{
		this->parentMesh = parentMeshIn;
	}

	void clear();

	void createChangeLog();

	struct CHANGE_LOG(FE_element) *extractChangeLog();

	struct CHANGE_LOG(FE_element) *getChangeLog()
	{
		return this->fe_element_changes;
	}

	struct LIST(FE_element_field_info) *get_FE_element_field_info_list_private()
	{
		return this->element_field_info_list;
	}

	struct FE_element_field_info *get_FE_element_field_info(
		struct LIST(FE_element_field) *fe_element_field_list);

	int remove_FE_element_field_info(
		struct FE_element_field_info *fe_element_field_info);

	int check_field_element_node_value_labels(FE_field *field,
		FE_region *target_fe_region);

	cmzn_mesh_scale_factor_set *find_scale_factor_set_by_name(const char *name);

	cmzn_mesh_scale_factor_set *create_scale_factor_set();

	bool is_FE_field_in_use(struct FE_field *fe_field);

	int get_number_of_FE_elements();

	int get_next_FE_element_identifier(int start_identifier);

	void list_btree_statistics();

	bool containsElement(FE_element *element);

	FE_element *findElementByIdentifier(int identifier) const;

	cmzn_elementiterator_id createElementiterator();

	struct FE_element *get_first_FE_element_that(
		LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function, void *user_data_void);

	int for_each_FE_element(LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data_void);

	struct LIST(FE_element) *createRelatedElementList();

	int change_FE_element_identifier(struct FE_element *element, int new_identifier);

	struct FE_element *get_or_create_FE_element_with_identifier(int identifier,
		struct FE_element_shape *element_shape);

	struct FE_element *create_FE_element_copy(int identifier, struct FE_element *source);

	int merge_FE_element_existing(struct FE_element *destination, struct FE_element *source);

	struct FE_element *merge_FE_element(struct FE_element *element);

	int merge_FE_element_and_faces(struct FE_element *element);

	int begin_define_faces();

	void end_define_faces();

	int define_faces();

	int remove_FE_element(struct FE_element *element);

	int remove_FE_element_list(struct LIST(FE_element) *remove_element_list);

	bool canMerge(FE_mesh &source);

	struct Merge_FE_element_external_data;
	int merge_FE_element_external(struct FE_element *element,
		Merge_FE_element_external_data &data);

	int merge(FE_mesh &source);
};

/**
 * List conditional function returning true if <element> is not in <fe_region>.
 * @param fe_mesh_void  void pointer to FE_mesh.
 */
int FE_element_is_not_in_FE_mesh(struct FE_element *element,
	void *fe_mesh_void);

#endif /* !defined (FINITE_ELEMENT_MESH_HPP) */
