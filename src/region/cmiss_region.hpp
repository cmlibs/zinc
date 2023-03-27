/***************************************************************************//**
 * FILE : cmiss_region.h
 *
 * Definition of cmzn_region, container of fields for representing model data,
 * and child regions for building hierarchical models.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (CMZN_REGION_H)
#define CMZN_REGION_H

#include "opencmiss/zinc/types/contextid.h"
#include "opencmiss/zinc/types/regionid.h"
#include "computed_field/computed_field.h"
#include "computed_field/field_derivative.hpp"
#include <list>


/*
Global constants
----------------
*/

/* separator character for cmzn_region in path strings */
#define CMZN_REGION_PATH_SEPARATOR_CHAR '/'
#define CMZN_REGION_PATH_SEPARATOR_STRING "/"
#define CMZN_REGION_PATH_PARENT_NAME ".."

/*
Global types
------------
*/

struct cmzn_regionevent
{
private:
	cmzn_region *region;
	int access_count;

	cmzn_regionevent(cmzn_region *regionIn);

	~cmzn_regionevent();

public:

	/** @param regionIn  Owning region; can be NULL for FINAL event */
	static cmzn_regionevent *create(cmzn_region *regionIn)
	{
		return new cmzn_regionevent(regionIn);
	}

	cmzn_regionevent *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(cmzn_regionevent* &event);

	cmzn_region *getRegion()
	{
		return this->region;
	}
};

struct cmzn_regionnotifier
{
private:
	cmzn_region_id region;  // not accessed
	cmzn_regionnotifier_callback_function function;
	void *user_data;
	int access_count;

	cmzn_regionnotifier(cmzn_region *region);

	~cmzn_regionnotifier();

public:

	/** private: external code must use cmzn_region_create_notifier */
	static cmzn_regionnotifier *create(cmzn_region *region)
	{
		if (region)
		{
			return new cmzn_regionnotifier(region);
		}
		return nullptr;
	}

	cmzn_regionnotifier *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(cmzn_regionnotifier* &notifier);

	int setCallback(cmzn_regionnotifier_callback_function function_in,
		void *user_data_in);

	void *getUserData()
	{
		return this->user_data;
	}

	void clearCallback();

	void regionDestroyed();

	void notify(cmzn_regionevent *event)
	{
		if ((this->function) && (event))
		{
			(this->function)(event, this->user_data);
		}
	}
};

typedef std::list<cmzn_regionnotifier *> cmzn_regionnotifier_list;

typedef std::list<cmzn_fieldmodulenotifier *> cmzn_fieldmodulenotifier_list;

struct cmzn_scene;

/**
 * Hierarchical constainer for local fields and Scene and child regions.
 */
struct cmzn_region
{
	friend struct cmzn_context;

private:
	char *name;
	/* non-accessed pointer to context region belongs to */
	cmzn_context *context;
	/* non-accessed pointer to parent region, or NULL if root */
	cmzn_region *parent;
	/* accessed first child and next sibling for building region tree */
	cmzn_region *first_child, *next_sibling;
	/* non-access pointer to previous sibling, if any */
	cmzn_region *previous_sibling;

	/* fields owned by this region (or master) */
	struct MANAGER(Computed_field) *field_manager;
	void *field_manager_callback_id;
	struct FE_region *fe_region;
	int field_cache_size; // 1 more than highest field cache index given out
	// all field caches currently in use for this region, for clearing
	// when fields changed, and adding value caches for new fields.
	std::list<cmzn_fieldcache_id> field_caches;
	std::vector<FieldDerivative *> fieldDerivatives;

	// Scene gives visualisation of region content
	cmzn_scene *scene;

	// incremented if any fields are modified in the region so field caches
	// can detect if their values are invalid.
	int fieldModifyCounter;

	/* increment/decrement change_level to nest changes. Message sent when zero */
	int change_level;
	/* number of hierarchical changes in progress on this region tree. A region's
	 * change_level will have one increment per ancestor hierarchical_change_level.
	 * Must be tracked to safely transfer when re-parenting regions */
	int hierarchical_change_level;

	// list of notifiers which receive region tree name/structure changes
	cmzn_regionnotifier_list regionnotifierList;
	// whether the region tree structure has changed:
	bool regionChanged;

	// list of notifiers which receive field module callbacks
	cmzn_fieldmodulenotifier_list fieldmodulenotifierList;

	/* number of objects using this region */
	int access_count;

	cmzn_region(cmzn_context* contextIn);

	~cmzn_region();

	/** Called only by context->createRegion */
	static cmzn_region *create(cmzn_context* contextIn);

	/** Clear context pointer; called only when context is being destroyed. */
	void clearContext()
	{
		this->context = nullptr;
	}

	void detachFields();

	static void Computed_field_change(struct MANAGER_MESSAGE(Computed_field) *message, void *region_void);

	/** Notify clients and parent region of region tree structure changes.
	 * Only call if changed and not caching */
	void notifyRegionChanged();

	/** Mark this region's tree structure as changed and notify clients if not caching */
	void setRegionChanged();

	/** Mark this region's tree structure as changed from a child change; complete with call to endRegionChangedChild() */
	void beginRegionChangedChild();

	/** Complete change notification from child change and notify clients if not caching */
	void endRegionChangedChild();

	/** Mark this region's tree structure as changed from child change and notify clients if not caching */
	void setRegionChangedChild()
	{
		this->setRegionChanged();
	}

	/** Mark this region's name as changed and notify clients if not caching */
	void setRegionChangedName()
	{
		this->setRegionChanged();
	}

	// clear cached changes to prevent notifications about them
	void clearCachedChanges();

	/**
	 * Adds delta_change_level to change_level of region and all its descendents.
	 * Begins or ends change cache as many times as magnitude of delta_change_level.
	 * +ve = beginChange, -ve = endChange.
	 */
	void deltaTreeChange(int delta_change_level);

	/** Counterpart to canMerge() to call for source region without a matching global region */
	bool isMergable();

	int mergeFields(cmzn_region& sourceRegion);

	int mergePrivate(cmzn_region& sourceRegion);

public:

	inline cmzn_region *access()
	{
		++(this->access_count);
		return this;
	}

	static int deaccess(cmzn_region* &region)
	{
		if (!region)
			return CMZN_ERROR_ARGUMENT;
		--(region->access_count);
        if (region->access_count <= 0)
        {
            delete region;
        }
        region = nullptr;
        return CMZN_OK;
	}

	static void reaccess(cmzn_region* &region, cmzn_region *newRegion)
	{
		if (newRegion)
        {
			++(newRegion->access_count);
        }
		if ((region) && (--(region->access_count) == 0))
			delete region;
		region = newRegion;
	}

	// all code which modifies values of fields must call this to ensure
	// field caches are recalculated
	inline void setFieldModify()
	{
		++(this->fieldModifyCounter);
	}

	// field caches store current value when calculating at a new location
	// and if the region value changes then cache is invalid
	inline int getFieldModifyCounter() const
	{
		return this->fieldModifyCounter;
	}

	/** begin cache changes to fields in region, not region tree structure */
	void beginChangeFields();

	/** end cache changes to fields in region, not region tree structure */
	void endChangeFields();

	/** begin cache changes to region tree and fields */
	void beginChange()
	{
		++(this->change_level);
		this->beginChangeFields();
	}

	/** end cache changes to region tree and fields */
	int endChange();

	int getSumHierarchicalChangeLevel() const;

	/** begin cache changes to region tree and fields for entire tree */
	void beginHierarchicalChange()
	{
		++(this->hierarchical_change_level);
		this->deltaTreeChange(+1);
	}

	/** end cache changes to region tree and fields for entire tree */
	void endHierarchicalChange()
	{
		--(this->hierarchical_change_level);
		this->deltaTreeChange(-1);
	}

	/**
	 * Returns pointer to context this region was created for. Can be NULL if
	 * context is destroyed already during clean-up.
	 * @return  Non-accessed pointer to cmzn_context or NULL if none/cleared.
	 */
	cmzn_context *getContext() const
	{
		return this->context;
	}

	struct MANAGER(Computed_field) *getFieldManager() const
	{
		return this->field_manager;
	}

	FE_region *get_FE_region() const
	{
		return this->fe_region;
	}

	/**
	 * Callback for changes to FE_region attached to region.
	 * Updates definitions of Computed_field wrappers for changed FE_fields in the
	 * region.
	 * Ensures region has cmiss_number and xi fields, at the appropriate time.
	 * Triggers computed field changes if not already changed.
	 * @private  Should only be called from finite_element_region.cpp
	 */
	void FeRegionChange();

	/** @return  Non-accessed region, or nullptr if not found. */
	cmzn_region *findChildByName(const char *name) const;

	/** Create child region with supplied name. Fails if name is in use.
	 * @param name  Unique name for child in this region. Avoid / and name "..".
	 * @return  Non-accessed region, or nullptr if failed. */
	cmzn_region *createChild(const char *name);

	/** Find subregion at relative path from this region, with special
	 * region name ".." mapping to parent of a given region.
	 * @param path  String ../relative/path/to/region.
	 * @return  Non-accessed region, or nullptr if not found. */
	cmzn_region *findSubregionAtPath(const char *path) const;

	/** Create subregion at relative path from this region, with special
	 * region name ".." mapping to parent of a given region. Fails if
	 * subregion already exists.
	 * @param path  String ../relative/path/to/region.
	 * @return  Non-accessed region, or nullptr if failed. */
	cmzn_region *createSubregion(const char *path);

	cmzn_fielditerator *createFielditerator() const;

	/** Called only by Field constructor code to assign cache index */
	int addField(cmzn_field *field);

	/** Called only by FieldDerivative to assign cache index */
	void addFieldDerivative(FieldDerivative *fieldDerivative);

	/** Called only by FieldDerivative to remove cache index */
	void removeFieldDerivative(FieldDerivative *fieldDerivative);

	/** Returns size of field cache array to fit all assigned cache indexes. */
	int getFieldcacheSize() const
	{
		return this->field_cache_size;
	}

	/** Called only by Fieldcache constructor.
	 * Adds cache to the list of caches for this region. Region needs this list to
	 * add new value caches for any fields created while the cache exists.
	 */
	void addFieldcache(cmzn_fieldcache *fieldcache)
	{
		if (fieldcache)
			this->field_caches.push_back(fieldcache);
	}

	/** Called only by Fieldcache destructor.
	 * Removes cache from the list of caches for this region.
	 */
	void removeFieldcache(cmzn_fieldcache *fieldcache)
	{
		if (fieldcache)
			this->field_caches.remove(fieldcache);
	}

	/**
	 * Private function for clearing field value caches for field in all caches
	 * listed in region.
	 */
	void clearFieldValueCaches(cmzn_field *field);

	/** @return  Non-accessed field of name, or nullptr if not found */
	cmzn_field *findFieldByName(const char *fieldName) const
	{
		if (!fieldName)
			return nullptr;
		return FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
			(char *)fieldName, this->field_manager);
	}

	const char *getName() const
	{
		return this->name;
	}

	int setName(const char *name);

	/** @return  Non-accessed parent region or nullptr if none */
	cmzn_region *getParent() const
	{
		return this->parent;
	}

	/** @return  Non-accessed root region which can be this region */
	cmzn_region *getRoot() const
	{
		cmzn_region *root = const_cast<cmzn_region *>(this);
		while (root->parent)
		{
			root = root->parent;
		}
		return root;
	}

	/** @return  Allocated string path/to/region */
	char *getPath() const;

	/** @return  Allocated string ../../relative/path/to/region */
	char *getRelativePath(const cmzn_region *baseRegion) const;

	cmzn_region *getFirstChild() const
	{
		return this->first_child;
	}

	cmzn_region *getNextSibling() const
	{
		return this->next_sibling;
	}

	cmzn_region *getPreviousSibling() const
	{
		return this->previous_sibling;
	}

	int appendChild(cmzn_region *new_child)
	{
		return this->insertChildBefore(new_child, nullptr);
	}

	int insertChildBefore(cmzn_region *new_child, cmzn_region *ref_child);

	int removeChild(cmzn_region *old_child);

	bool containsSubregion(const cmzn_region *subregion) const;

	void addFieldmodulenotifier(cmzn_fieldmodulenotifier *notifier);

	void removeFieldmodulenotifier(cmzn_fieldmodulenotifier *notifier);

	void addRegionnotifier(cmzn_regionnotifier *notifier);

	void removeRegionnotifier(cmzn_regionnotifier *notifier);

	/** @return  Non-accessed pointer to scene for region, or nullptr if none */
	cmzn_scene *getScene() const
	{
		return this->scene;
	}

	int getTimeRange(FE_value& minimumTime, FE_value& maximumTime, bool hierarchical) const;

	/**
	 * Check that fields and other object definitions in source region are properly
	 * defined and compatible with definitions in this region.
	 * Converts legacy field representations e.g. read from older EX files, hence
	 * source region can be modified, and function fails if conversion is not
	 * possible.
	 * A successful call to this function is prerequisite for calling merge().
	 * @see merge()
	 * @param sourceRegion  Source region to check. Can be modified.
	 * @return  True if compatible and conversions successful, false if failed.
	 */
	bool canMerge(cmzn_region& sourceRegion);

	/**
	 * Merge fields and other objects from source region tree into this region,
	 * transferring objects in some cases for efficiency.
	 * @see canMerge()
	 * @param sourceRegion  Source region to merge from. Modified and left in an
	 * in an unusable state by this function as some of its objects may have been
	 * merged into this region, hence must be destroyed after calling.
	 * @return  Result OK on success, otherwise any error code.
	 */
	int merge(cmzn_region& sourceRegion);

};

/*
Global functions
----------------
*/

/***************************************************************************//**
 * Remove all nodes, elements, data and finite element fields from this region.
 *
 * @param region  The region to clear the fields from. Must not be a group.
 * @return  1 on success, 0 if no region supplied.
 */
int cmzn_region_clear_finite_elements(struct cmzn_region *region);

/***************************************************************************//**
 * Returns FE_region for this cmzn_region.
 */
struct FE_region *cmzn_region_get_FE_region(struct cmzn_region *region);

/***************************************************************************//**
 * Returns the field manager for this region.
 */
struct MANAGER(Computed_field) *cmzn_region_get_Computed_field_manager(
	struct cmzn_region *region);

/***************************************************************************//**
 * Allocates and returns the path to the root_region ("/").
 *
 * @return  Allocated string "/".
 */
char *cmzn_region_get_root_region_path(void);

/*******************************************************************************
 * Internal only. External API is cmzn_fieldmodule_find_field_by_name.
 * @return  Accessed handle to field of given name, or NULL if none.
 */
cmzn_field_id cmzn_region_find_field_by_name(cmzn_region_id region,
	const char *field_name);

/***************************************************************************//**
 * Deprecated legacy version of cmzn_region_find_subregion_at_path returning
 * non-ACCESSed region as final argument.
 *
 * @param region  The region to search.
 * @param path  The directory-style path to the subregion.
 * @param subregion_address  Address to put region at path. Set to NULL if no
 * region is identified.
 * @return  1 if region found, 0 otherwise.
 */
int cmzn_region_get_region_from_path_deprecated(struct cmzn_region *region,
	const char *path, struct cmzn_region **subregion_address);

/**
 * Returns true if region has no parent.
 */
bool cmzn_region_is_root(struct cmzn_region *region);

/**
 * Separates a region/path/name into the region plus region-path and remainder
 * string containing text from the first unrecognized child region name.
 *
 * Examples:
 * "fibres" or "/fibres" -> root_region, "" and "fibres" if fibres was not a
 *     child region of the root region
 * "heart/fibres" or "/heart/fibres" -> heart region, "heart" and "fibres" if
 *     heart region has no child region called fibres
 * "body/heart" -> heart region, "body/heart" and NULL name if root region
 *     contains body region contains heart region
 * "heart/bob/fred/" -> region heart, "heart" and "bob/fred" if region heart
 *     has no child region called bob
 * Parent regions can be reached by ..:
 * "../bob/coordinates" -> region ../bob, "../bob" and "cooordinates"
 *
 * @param baseRegion  The base region for path.
 * @path path  The input path and additional characters.
 * @param regionAddress on success, points to region partially matched by path
 * @param regionPathAddress on success, returns allocated string path to the
 *   returned region, stripped of leading and trailing region path separators
 * @param remainderAddress on success, returns pointer to allocated remainder
 *   of path stripped of leading and trailing region path separators, or NULL
 *   if all of path was resolved
 * @return 1 on success, 0 on failure
 */
int cmzn_region_get_partial_region_path(cmzn_region *baseRegion,
	const char *path, cmzn_region **regionAddress,
	char **regionPathAddress,	char **remainderAddress);

int cmzn_region_list(struct cmzn_region *region,
	int indent, int indent_increment);
/*******************************************************************************
LAST MODIFIED : 5 March 2003

DESCRIPTION :
Lists the cmzn_region hierarchy starting from <region>. Contents are listed
indented from the left margin by <indent> spaces; this is incremented by
<indent_increment> for each child level.
==============================================================================*/

/** Called only by ~FieldDerivative.
 * Add the field derivative to the list in the region and assign its unique
 * cache index.
 * NOTE: Throws an exception on any failure.
 * @return  Accessed field derivative or nullptr if failed.
 */
void cmzn_region_add_field_derivative(cmzn_region *region,
	FieldDerivative *fieldDerivative);

/** Called only by ~FieldDerivative */
void cmzn_region_remove_field_derivative(cmzn_region *region, FieldDerivative *field_derivative);

#endif /* !defined (CMZN_REGION_H) */
