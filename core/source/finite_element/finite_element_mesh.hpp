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

#include "opencmiss/zinc/status.h"
#include "datastore/labels.hpp"
#include "datastore/labelschangelog.hpp"
#include "datastore/maparray.hpp"
#include "finite_element/finite_element.h"
#include "general/block_array.hpp"
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
 * Template for creating a new element in the given FE_mesh
 * @see FE_mesh::create_FE_element()
 */
class FE_element_template : public cmzn::RefCounted
{
	friend class FE_mesh;

	FE_mesh *mesh; // Note: accessed: prevents mesh from being destroyed
	FE_element_shape *element_shape;
	FE_element *template_element;

	/** Caller must ensure element_shape is of same dimension as mesh */
	FE_element_template(FE_mesh *mesh_in, struct FE_element_field_info *element_field_info, FE_element_shape *element_shape_in);

	/** Creates a template copying from an existing element */
	FE_element_template(FE_mesh *mesh_in, struct FE_element *element);

	~FE_element_template();

	FE_element_template(); // not implemented
	FE_element_template(const FE_element_template&); // not implemented

public:

	FE_mesh *get_mesh() const
	{
		return this->mesh;
	}

	FE_element *get_template_element() const
	{
		return this->template_element;
	}

	FE_element_shape *get_element_shape() const
	{
		return this->element_shape;
	}
};

/**
 * A set of elements in the FE_region.
 */
class FE_mesh
{
public:

	/** Stores element shape and face maps for that shape */
	class ElementShapeFaces
	{
		FE_element_shape *shape;
		const cmzn_element_shape_type shape_type;
		const int faceCount;
		DsMapArrayLabelIndex faces; // map to connected element indexes from face mesh

	public:
		ElementShapeFaces(DsLabels *labels, FE_element_shape *shapeIn) :
			shape(ACCESS(FE_element_shape)(shapeIn)),
			shape_type(FE_element_shape_get_simple_type(shapeIn)),
			faceCount(FE_element_shape_get_number_of_faces(shapeIn)),
			faces(labels, faceCount > 0 ? faceCount : 2, DS_LABEL_INDEX_UNALLOCATED, DS_LABEL_INDEX_INVALID)
		{
		}

		~ElementShapeFaces()
		{
			DEACCESS(FE_element_shape)(&this->shape);
		}

		FE_element_shape *getShape() const
		{
			return this->shape;
		}

		cmzn_element_shape_type getShapeType() const
		{
			return this->shape_type;
		}

		int getFaceCount() const
		{
			return this->faceCount;
		}

		/** return face number for face type, or negative if invalid */
		int faceTypeToNumber(cmzn_element_face_type faceType)
		{
			// temporary, until other element shape face type enums and mappings to face numbers are supported:
			int faceNumber = static_cast<int>(faceType) - CMZN_ELEMENT_FACE_TYPE_XI1_0;
			if (faceNumber < this->faceCount)
				return faceNumber;
			return -1;
		}

		/** clear face element indexes held for elementIndex */
		void clearElementFaces(DsLabelIndex elementIndex)
		{
			this->faces.clearArray(elementIndex);
		}

		/** @return  Array of faceCount face element indexes, or 0 if not allocated. NOT to be freed. */
		DsLabelIndex *getElementFaces(DsLabelIndex elementIndex)
		{
			return this->faces.getArray(elementIndex);
		}

		/** @return  Array of faceCount face element indexes, or 0 if not allocated. NOT to be freed. */
		const DsLabelIndex *getElementFaces(DsLabelIndex elementIndex) const
		{
			return this->faces.getArray(elementIndex);
		}

		/** @return  Existing or new array of faceCount face element indexes, or 0 if failed. NOT to be freed.
		 * New array contents are initialised to DS_LABEL_INDEX_INVALID. */
		DsLabelIndex *getOrCreateElementFaces(DsLabelIndex elementIndex)
		{
			return this->faces.getOrCreateArray(elementIndex);
		}

		/** convenient function for getting a single face for an element */
		DsLabelIndex getElementFace(DsLabelIndex elementIndex, int faceNumber);

		/** convenient function for setting a single face for an element */
		int setElementFace(DsLabelIndex elementIndex, int faceNumber, DsLabelIndex faceIndex);

	};

private:

	FE_region *fe_region; // not accessed
	int dimension;

	DsLabels labels; // element identifiers

	// element shape and face/parent mappings
	unsigned int elementShapeFacesCount;
	ElementShapeFaces **elementShapeFacesArray;
	block_array<DsLabelIndex, unsigned char> elementShapeMap; // map element index -> shape faces index, if not all elements have same shape
	block_array<DsLabelIndex,DsLabelIndex*> parents; // map to number of parents followed by parent element indexes in order

	// map element index -> FE_element (accessed)
	block_array<DsLabelIndex, FE_element*, 128> fe_elements;

	struct LIST(FE_element_field_info) *element_field_info_list;
	/* keep pointer to last element field information so only need to
	   note changed fields when this changes */
	struct FE_element_field_info *last_fe_element_field_info;

	FE_mesh *parentMesh; // not accessed
	FE_mesh *faceMesh; // not accessed

	/* log of elements added, removed or otherwise changed */
	DsLabelsChangeLog *changeLog;

	/* information for defining faces */
	/* existence of element_type_node_sequence_list can tell us whether faces
		 are being defined */
	struct LIST(FE_element_type_node_sequence) *element_type_node_sequence_list;
	bool definingFaces;

	// scale factor sets in use. Used as identifier for finding scale factor
	// arrays stored with elements 
	std::vector<cmzn_mesh_scale_factor_set*> scale_factor_sets;

	// list of element iterators to invalidate when mesh destroyed
	cmzn_elementiterator *activeElementIterators;

	int access_count;

private:

	FE_mesh(FE_region *fe_regionIn, int dimensionIn);

	~FE_mesh();

	struct FE_element_field_info *clone_FE_element_field_info(
		struct FE_element_field_info *fe_element_field_info);

	int findOrCreateFace(DsLabelIndex parentIndex, int faceNumber, DsLabelIndex& faceIndex);

	int remove_FE_element_private(struct FE_element *element);

	int merge_FE_element_existing(struct FE_element *destination, struct FE_element *source);

	struct Merge_FE_element_external_data;
	int merge_FE_element_external(struct FE_element *element,
		Merge_FE_element_external_data &data);

	inline ElementShapeFaces *getElementShapeFaces(DsLabelIndex elementIndex)
	{
		if (elementIndex >= 0)
		{
			if (this->elementShapeFacesCount > 1)
			{
				unsigned char shapeIndex;
				if (this->elementShapeMap.getValue(elementIndex, shapeIndex))
					return this->elementShapeFacesArray[static_cast<unsigned int>(shapeIndex)];
			}
			else if (this->elementShapeFacesArray)
				return this->elementShapeFacesArray[0];
		}
		return 0;
	}

	int addElementParent(DsLabelIndex elementIndex, DsLabelIndex parentIndex);

	int removeElementParent(DsLabelIndex elementIndex, DsLabelIndex parentIndex);

	void clearElementParents(DsLabelIndex elementIndex);

	void clearElementFaces(DsLabelIndex elementIndex);

public:

	static FE_mesh *create(FE_region *fe_region, int dimension)
	{
		if (fe_region && (0 < dimension) && (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
			return new FE_mesh(fe_region, dimension);
		return 0;
	}

	void detach_from_FE_region();

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

	// use only by FE_element_log_FE_field_changes
	FE_element_field_info *get_last_fe_element_field_info() const
	{
		return this->last_fe_element_field_info;
	}

	// use only by FE_element_log_FE_field_changes
	void set_last_fe_element_field_info(FE_element_field_info *fe_element_field_info)
	{
		this->last_fe_element_field_info = fe_element_field_info;
	}

	// in following change is a logical OR of values from enum DsLabelChangeType
	void elementChange(DsLabelIndex elementIndex, int change);
	void elementChange(DsLabelIndex elementIndex, int change, FE_element *field_info_element);
	void elementFieldListChange(FE_element *element, int change,
		struct LIST(FE_field) *changed_fe_field_list);
	void elementFieldChange(FE_element *element, FE_field *fe_field);
	void elementIdentifierChange(FE_element *element);
	void elementAddedChange(FE_element *element);
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

	/** @return Accessed changes */
	DsLabelsChangeLog *extractChangeLog();

	/** @retrun non-Accessed changes */
	DsLabelsChangeLog *getChangeLog()
	{
		return this->changeLog;
	}

	ElementShapeFaces *setElementShape(DsLabelIndex elementIndex, FE_element_shape *element_shape);

	bool setElementShapeFromTemplate(DsLabelIndex elementIndex, FE_element_template &element_template);

	inline const ElementShapeFaces *getElementShapeFacesConst(DsLabelIndex elementIndex) const
	{
		return const_cast<FE_mesh*>(this)->getElementShapeFaces(elementIndex);
	}

	FE_element_shape *getElementShape(DsLabelIndex elementIndex)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
			return elementShapeFaces->getShape();
		return 0;
	}

	cmzn_element_shape_type getElementShapeType(DsLabelIndex elementIndex)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
			return elementShapeFaces->getShapeType();
		return CMZN_ELEMENT_SHAPE_TYPE_INVALID;
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

	/** get size i.e. number of elements in mesh */
	int getSize() const
	{
		return this->labels.getSize();
	}

	inline DsLabelIdentifier getElementIdentifier(DsLabelIndex elementIndex) const
	{
		return this->labels.getIdentifier(elementIndex);
	}

	/** @return  Non-accessed element object at index */
	inline FE_element *getElement(DsLabelIndex elementIndex) const
	{
		FE_element *element = 0;
		if (elementIndex >= 0)
			this->fe_elements.getValue(elementIndex, element);
		return element;
	}

	DsLabelIdentifier get_next_FE_element_identifier(DsLabelIdentifier start_identifier)
	{
		return this->labels.getFirstFreeIdentifier(start_identifier);
	}

	void list_btree_statistics();

	bool containsElement(FE_element *element) const
	{
		return (FE_element_get_FE_mesh(element) == this) && (get_FE_element_index(element) >= 0);
	}

	DsLabelIndex findIndexByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->labels.findLabelByIdentifier(identifier);
	}

	/** @return  Non-accessed element */
	FE_element *findElementByIdentifier(DsLabelIdentifier identifier) const
	{
		return this->getElement(this->labels.findLabelByIdentifier(identifier));
	}

	void removeElementiterator(cmzn_elementiterator *iterator); // private, but needed by cmzn_elementiterator

	cmzn_elementiterator *createElementiterator(DsLabelsGroup *labelsGroup = 0);

	struct FE_element *get_first_FE_element_that(
		LIST_CONDITIONAL_FUNCTION(FE_element) *conditional_function, void *user_data_void);

	int for_each_FE_element(LIST_ITERATOR_FUNCTION(FE_element) iterator_function, void *user_data_void);

	DsLabelsGroup *createLabelsGroup();

	int change_FE_element_identifier(struct FE_element *element, int new_identifier);

	FE_element_template *create_FE_element_template(FE_element *element);

	FE_element_template *create_FE_element_template(FE_element_shape *element_shape);

	struct FE_element *get_or_create_FE_element_with_identifier(int identifier,
		struct FE_element_shape *element_shape);

	FE_element *create_FE_element(int identifier, FE_element_template *element_template);

	int merge_FE_element_template(struct FE_element *destination, FE_element_template *fe_element_template);

	DsLabelIndex getElementFace(DsLabelIndex elementIndex, int faceNumber)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (!elementShapeFaces)
			return DS_LABEL_INDEX_INVALID;
		return elementShapeFaces->getElementFace(elementIndex, faceNumber);
	}

	int setElementFace(DsLabelIndex elementIndex, int faceNumber, DsLabelIndex faceIndex);

	int getElementFaceNumber(DsLabelIndex elementIndex, DsLabelIndex faceIndex);

	/** @return  Number of parent elements.
	 * @param parents  If has parents, pointer to parent element indexes. Not to be freed */
	int getElementParents(DsLabelIndex elementIndex, const DsLabelIndex *&parents) const
	{
		if (elementIndex >= 0)
		{
			DsLabelIndex *parentsArray;
			if (this->parents.getValue(elementIndex, parentsArray) && parentsArray)
			{
				parents = parentsArray + 1;
				return *parentsArray;
			}
		}
		return 0;
	}

	/** @return  Number of parent elements. */
	inline int getElementParentsCount(DsLabelIndex elementIndex) const
	{
		if (this->parentMesh)
		{
			const DsLabelIndex *parents;
			return this->getElementParents(elementIndex, parents);
		}
		return 0;
	}

	// GRC restore implementation from descendant up
	bool isElementAncestor(DsLabelIndex elementIndex, FE_mesh *descendantMesh, DsLabelIndex descendantIndex);

	/** return true if element or parent has exactly 1 parent element */
	bool isElementExterior(DsLabelIndex elementIndex);

	/** return true if elementIndex has faceIndex on the given face */
	bool isElementFaceOfType(DsLabelIndex elementIndex, DsLabelIndex faceIndex, cmzn_element_face_type faceType)
	{
		ElementShapeFaces *elementShapeFaces = this->getElementShapeFaces(elementIndex);
		if (elementShapeFaces)
		{
			const DsLabelIndex *faces = elementShapeFaces->getElementFaces(elementIndex);
			if (faces)
			{
				const int faceNumber = elementShapeFaces->faceTypeToNumber(faceType);
				if ((0 <= faceNumber) && (faces[faceNumber] == faceIndex))
					return true;
			}
		}
		return false;
	}

	/** return index of parent element that either this element is on the given face of,
		* or the parent is on the given face of a top-level element. */
	DsLabelIndex getElementParentOnFace(DsLabelIndex elementIndex, cmzn_element_face_type faceType);

	DsLabelIndex getElementFirstNeighbour(DsLabelIndex elementIndex, int faceNumber, int &newFaceNumber);

	int defineElementFaces(DsLabelIndex elementIndex);

	int begin_define_faces();

	void end_define_faces();

	int define_faces();

	int destroyElement(struct FE_element *element);

	int destroyAllElements();

	int destroyElementsInGroup(DsLabelsGroup& labelsGroup);

	bool canMerge(FE_mesh &source);

	int merge(FE_mesh &source);
};

struct cmzn_elementiterator : public cmzn::RefCounted
{
	friend class FE_mesh;

private:
	FE_mesh *fe_mesh;
	DsLabelIterator *iter;
	cmzn_elementiterator *nextIterator; // for linked list of active iterators in FE_mesh

	// takes ownership of iter_in access count
	cmzn_elementiterator(FE_mesh *fe_mesh_in, DsLabelIterator *iter_in) :
		fe_mesh(fe_mesh_in),
		iter(iter_in),
		nextIterator(0)
	{
	}

	virtual ~cmzn_elementiterator()
	{
		if (this->fe_mesh)
			this->fe_mesh->removeElementiterator(this);
		cmzn::Deaccess(this->iter);
	}

	void invalidate()
	{
		// sufficient to clear fe_mesh as only called from fe_mesh destructor
		this->fe_mesh = 0;
	}

	template<class REFCOUNTED>
		friend inline void cmzn::Deaccess(REFCOUNTED*& elementIterator);

public:

	/** @return  Non-accessed element or 0 if iteration ended or iterator invalidated */
	FE_element *nextElement()
	{
		if (this->fe_mesh)
		{
			const DsLabelIndex elementIndex = this->iter->nextIndex();
			return this->fe_mesh->getElement(elementIndex);
		}
		return 0;
	}

	void setIndex(DsLabelIndex index)
	{
		this->iter->setIndex(index);
	}
};

#endif /* !defined (FINITE_ELEMENT_MESH_HPP) */
