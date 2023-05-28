/**
 *FILE : computed_field_group.hpp
 *
 * Implements a "group" computed_field which group regions, 
 * node and data point component.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined (COMPUTED_FIELD_GROUP_HPP)
#define COMPUTED_FIELD_GROUP_HPP

#include "cmlibs/zinc/fieldmodule.h"
#include "cmlibs/zinc/fieldgroup.h"
#include "computed_field/computed_field_group_base.hpp"
#include "mesh/mesh_group.hpp"
#include "mesh/nodeset_group.hpp"
#include <map>

struct cmzn_field_hierarchical_group_change_detail : public cmzn_field_group_base_change_detail
{
private:
	int localChangeSummary;
	int nonlocalChangeSummary;

public:
	cmzn_field_hierarchical_group_change_detail() :
		localChangeSummary(CMZN_FIELD_GROUP_CHANGE_NONE),
		nonlocalChangeSummary(CMZN_FIELD_GROUP_CHANGE_NONE)
	{
	}

	void clear()
	{
		localChangeSummary = CMZN_FIELD_GROUP_CHANGE_NONE;
		nonlocalChangeSummary = CMZN_FIELD_GROUP_CHANGE_NONE;
	}

	virtual int getChangeSummary() const
	{
		return localChangeSummary | nonlocalChangeSummary;
	}

	int getLocalChangeSummary() const
	{
		return localChangeSummary;
	}

	int getNonlocalChangeSummary() const
	{
		return nonlocalChangeSummary;
	}

	/** Inform local group has been added, but wasn't before */
	void changeAddLocal()
	{
		localChangeSummary |= CMZN_FIELD_GROUP_CHANGE_ADD;
	}

	/** Inform local group has had part removed */
	void changeRemoveLocal()
	{
		localChangeSummary |= CMZN_FIELD_GROUP_CHANGE_REMOVE;
	}

	void changeMergeLocal(int inChangeSummary)
	{
		localChangeSummary |= inChangeSummary;
	}

	void changeMergeNonlocal(int inChangeSummary)
	{
		nonlocalChangeSummary |= inChangeSummary;
	}
};

typedef std::map<cmzn_region_id, cmzn_field_group_id> Region_field_map;
typedef std::map<cmzn_region_id, cmzn_field_group_id>::iterator Region_field_map_iterator;
typedef std::map<cmzn_region_id, cmzn_field_group_id>::const_iterator Region_field_map_const_iterator;

typedef int (*cmzn_field_group_iterator_function)(cmzn_field_group_id,void *);

class Computed_field_element_group;
class Computed_field_node_group;

class Computed_field_group : public Computed_field_group_base
{
private:
	cmzn_field_hierarchical_group_change_detail change_detail;
	bool containsAllLocal;
	cmzn_field_group_subelement_handling_mode subelementHandlingMode;
	cmzn_nodeset_group* nodesetGroups[2];  // 0 == nodes, 1 == datapoints
	cmzn_mesh_group* meshGroups[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	Region_field_map child_region_group_map;  // map to accessed FieldGroup in child regions

	inline static int FE_nodeset_to_index(const FE_nodeset* feNodeset)
	{
		return (feNodeset->getFieldDomainType() == CMZN_FIELD_DOMAIN_TYPE_NODES) ? 0 : 1;
	}

	/*
	 * @param index  0 (nodeset), 1 (datapoints)
	 * @param clear  If true, clear the subelement group.
	 * @param remove  If true, remove the subelement group. Only removed if group holds only access.
	 */
	void clearRemoveNodesetGroup(int index, bool clear = true, bool remove = true);

	/*
	 * @param index  0 (1-D), 1 (2-D) or 2 (3-D)
	 * @param clear  If true, clear the subelement group.
	 * @param remove  If true, remove the subelement group. Only removed if group holds only access.
	 */
	void clearRemoveMeshGroup(int index, bool clear = true, bool remove = true);

	cmzn_field_group* getFieldGroup()
	{
		return reinterpret_cast<cmzn_field_group*>(this->field);
	}

public:

	Computed_field_group(cmzn_region *region);

	virtual ~Computed_field_group();

	/** For internal use and region use only.
	 * @param feNodeset  Finite element nodeset from this region only. Not checked.
	 * @return Non-accessed nodeset group or nullptr if none. */
	cmzn_nodeset_group* getLocalNodesetGroup(const FE_nodeset* feNodeset) const
	{
		const int index = FE_nodeset_to_index(feNodeset);
		return this->nodesetGroups[index];
	}

	/** For internal use and region use only.
	 * @param feMesh  Finite element mesh from this region only. Not checked.
	 * @return Non-accessed mesh group or nullptr if none. */
	cmzn_mesh_group* getLocalMeshGroup(const FE_mesh* feMesh) const
	{
		const int index = feMesh->getDimension() - 1;
		return this->meshGroups[index];
	}

	/** Able to get or create nodeset group in subregions.
	 * @param feNodeset  Finite element nodeset containing master list to be grouped.
	 * @param canGet  If true, can find an existing group.
	 * @param canCreate  If true, can create a new nodeset group if not found.
	 * @return  Non-accessed nodeset group, or nullptr if none */
	cmzn_nodeset_group* getOrCreateNodesetGroup(FE_nodeset* feNodeset, bool canGet = true, bool canCreate = true);

	/** Able to get or create mesh group in subregions.
	 * @param feMesh  Finite element mesh containing master list to be grouped.
	 * @param canGet  If true, can find an existing group.
	 * @param canCreate  If true, can create a new nodeset group if not found.
	 * @return  Non-accessed mesh group, or nullptr if none */
	cmzn_mesh_group* getOrCreateMeshGroup(FE_mesh* mesh, bool canGet = true, bool canCreate = true);

	/** Get or create group subregions, linking up intermediate regions to this group field.
	 * @param canGet  If true, can find an existing group, including by name.
	 * @param canCreate  If true, can create an existing group if not found.
	 * @return  Non-accessed subregion group, or nullptr if none */
	cmzn_field_group* getOrCreateSubregionFieldGroup(cmzn_region *subregion, bool canGet=true, bool canCreate=true);

	cmzn_field_group* getFirstNonEmptyGroup();

	int for_each_group_hiearchical(cmzn_field_group_iterator_function function, void *user_data);

	int remove_empty_subgroups();


	/* Record that group has changed locally by object add, with client notification */
	void changeAddLocal()
	{
		this->change_detail.changeAddLocal();
		this->field->setChanged();
	}

	/* Record that group has changed locally by object remove, with client notification */
	void changeRemoveLocal()
	{
		this->change_detail.changeRemoveLocal();
		this->field->setChanged();
	}

	/* Record that group has changed locally by object remove, but do not notify clients.
	 * Only used when objects removed because destroyed in parent domain. */
	void changeRemoveLocalNoNotify()
	{
		this->change_detail.changeRemoveLocal();
	}

	virtual cmzn_field_change_detail *extract_change_detail();

	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(Computed_field) *message);

	virtual void subregionRemoved(cmzn_region *subregion)
	{
		this->remove_child_group(subregion);
	}

	bool isEmptyLocal() const;

	virtual bool isEmpty() const
	{
		return (isEmptyLocal() && isEmptyNonLocal());
	}

	bool wasModified() const
	{
		return this->change_detail.getChangeSummary() != CMZN_FIELD_GROUP_CHANGE_NONE;
	}

	virtual int clear();

	int clearLocal();

	int addLocalRegion();

	int removeLocalRegion();

	bool containsLocalRegion();

	int addRegion(cmzn_region* subregion);

	bool containsRegion(cmzn_region* subregion);

	int removeRegion(cmzn_region* subregion);

	cmzn_field_group_subelement_handling_mode getSubelementHandlingMode() const
	{
		return this->subelementHandlingMode;
	}

	int setSubelementHandlingMode(cmzn_field_group_subelement_handling_mode mode);

private:

	Computed_field_core* copy()
	{
		Computed_field_group *core = new Computed_field_group(this->field->getRegion());
		core->containsAllLocal = this->containsAllLocal;
		return (core);
	};

	virtual const char* get_type_string();

	void remove_child_group(struct cmzn_region *child_region);

	int compare(Computed_field_core* other_field);

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
	{
		inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
		return 1;
	}

	int list();

	inline int isSubGroupEmpty(Computed_field_core *source_core) const
	{
		Computed_field_group_base *group_base = dynamic_cast<Computed_field_group_base *>(source_core);
		if (group_base)
		{
			return group_base->isEmpty();
		}
		display_message(ERROR_MESSAGE,
			"Computed_field_group::isSubGroupEmpty.  Subgroup not derived from Computed_field_group_base");
		return 0;
	}

	bool isEmptyNonLocal() const;

};

/** Get handle to implementation of group.
 * @param  group  Group field. Caller must ensure this is a valid pointer.
 * @return  Computed_field_group*. */
inline Computed_field_group* cmzn_field_group_core_cast(cmzn_field_group* group)
{
	return (static_cast<Computed_field_group*>(reinterpret_cast<cmzn_field*>(group)->core));
}

/** If field is a group, get its group core, otherwise nullptr */
inline Computed_field_group* cmzn_field_get_group_core(const cmzn_field* field)
{
	if (field)
	{
		return dynamic_cast<Computed_field_group*>(field->core);
	}
	return nullptr;
}

bool cmzn_field_group_was_modified(cmzn_field_group_id group);

#endif /* !defined (COMPUTED_FIELD_GROUP_HPP) */
