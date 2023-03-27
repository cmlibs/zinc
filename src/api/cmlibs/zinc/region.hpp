/**
 * @file region.hpp
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_REGION_HPP__
#define CMZN_REGION_HPP__

#include "cmlibs/zinc/region.h"
#include "cmlibs/zinc/context.hpp"

namespace CMLibs
{
namespace Zinc
{

class Fieldmodule;
class Scene;
class StreaminformationRegion;
class Regionnotifier;

class Region
{
protected:
	cmzn_region_id id;

public:

	Region() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Region(cmzn_region_id in_region_id) : id(in_region_id)
	{  }

	Region(const Region& region) : id(cmzn_region_access(region.id))
	{ }

	Region& operator=(const Region& region)
	{
		cmzn_region_id temp_id = cmzn_region_access(region.id);
		if (0 != id)
		{
			cmzn_region_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Region()
	{
		if (0 != id)
		{
			cmzn_region_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_region_id getId() const
	{
		return id;
	}

	int beginChange()
	{
		return cmzn_region_begin_change(id);
	}

	int endChange()
	{
		return cmzn_region_end_change(id);
	}

	int beginHierarchicalChange()
	{
		return cmzn_region_begin_hierarchical_change(id);
	}

	int endHierarchicalChange()
	{
		return cmzn_region_end_hierarchical_change(id);
	}

	Region createChild(const char *name)
	{
		return Region(cmzn_region_create_child(id, name));
	}

	Region createSubregion(const char *path)
	{
		return Region(cmzn_region_create_subregion(id, path));
	}

	Region createRegion()
	{
		return Region(cmzn_region_create_region(id));
	}

	inline Fieldmodule getFieldmodule() const;

	int readFile(const char *fileName)
	{
		return cmzn_region_read_file(id, fileName);
	}

	Context getContext() const
	{
		return Context(cmzn_region_get_context(id));
	}

	char *getName() const
	{
		return cmzn_region_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_region_set_name(id, name);
	}

	Region getParent() const
	{
		return Region(cmzn_region_get_parent(id));
	}

	char *getPath() const
	{
		return cmzn_region_get_path(id);
	}

	char *getRelativePath(const Region& baseRegion) const
	{
		return cmzn_region_get_relative_path(id, baseRegion.id);
	}

	Region getRoot() const
	{
		return Region(cmzn_region_get_root(id));
	}

	Region getFirstChild() const
	{
		return Region(cmzn_region_get_first_child(id));
	}

	Region getNextSibling() const
	{
		return Region(cmzn_region_get_next_sibling(id));
	}

	Region getPreviousSibling() const
	{
		return Region(cmzn_region_get_previous_sibling(id));
	}

	int appendChild(const Region& newChild)
	{
		return cmzn_region_append_child(id, newChild.id);
	}

	int insertChildBefore(const Region& newChild, const Region& refChild)
	{
		return cmzn_region_insert_child_before(id, newChild.id, refChild.id);
	}

	int removeChild(const Region& oldChild)
	{
		return cmzn_region_remove_child(id, oldChild.id);
	}

	Region findChildByName(const char *name) const
	{
		return Region(cmzn_region_find_child_by_name(id, name));
	}

	Region findSubregionAtPath(const char *path) const
	{
		return Region(cmzn_region_find_subregion_at_path(id, path));
	}

	bool containsSubregion(const Region& subregion) const
	{
		return cmzn_region_contains_subregion(id, subregion.id);
	}

	int writeFile(const char *fileName) const
	{
		return cmzn_region_write_file(id, fileName);
	}

	inline Scene getScene() const;

	inline StreaminformationRegion createStreaminformationRegion();

	inline int read(const StreaminformationRegion& streaminformationRegion);

	inline int write(const StreaminformationRegion& streaminformationRegion) const;

	int getTimeRange(double *minimumValueOut, double *maximumValueOut)
	{
		return cmzn_region_get_time_range(this->id, minimumValueOut, maximumValueOut);
	}

	int getHierarchicalTimeRange(double *minimumValueOut, double *maximumValueOut)
	{
		return cmzn_region_get_hierarchical_time_range(this->id, minimumValueOut, maximumValueOut);
	}

	inline Regionnotifier createRegionnotifier();

};

inline bool operator==(const Region& a, const Region& b)
{
	return a.getId() == b.getId();
}

inline Region Context::getDefaultRegion() const
{
	return Region(cmzn_context_get_default_region(id));
}

inline int Context::setDefaultRegion(const Region& region)
{
	return cmzn_context_set_default_region(id, region.getId());
}

inline Region Context::createRegion()
{
	return Region(cmzn_context_create_region(id));
}

class Regionevent
{
protected:
	cmzn_regionevent_id id;

public:

	Regionevent() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Regionevent(cmzn_regionevent_id in_regionevent_id) :
		id(in_regionevent_id)
	{  }

	Regionevent(const Regionevent& regionEvent) :
		id(cmzn_regionevent_access(regionEvent.id))
	{  }

	Regionevent& operator=(const Regionevent& regionEvent)
	{
		cmzn_regionevent_id temp_id = cmzn_regionevent_access(regionEvent.id);
		if (0 != id)
			cmzn_regionevent_destroy(&id);
		id = temp_id;
		return *this;
	}

	~Regionevent()
	{
		if (0 != id)
		{
			cmzn_regionevent_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_regionevent_id getId() const
	{
		return id;
	}

};

/**
 * @brief Base class functor for region notifier callbacks
 *
 * Base class functor for region notifier callbacks:
 * - Derive from this class adding any user data required.
 * - Implement virtual operator()(const Regionevent&) to handle callback.
 * @see Regionnotifier::setCallback()
 */
class Regioncallback
{
	friend class Regionnotifier;
private:
	Regioncallback(const Regioncallback&); // not implemented
	Regioncallback& operator=(const Regioncallback&); // not implemented

	static void C_callback(cmzn_regionevent_id regionevent_id, void *callbackVoid)
	{
		Regionevent regionevent(cmzn_regionevent_access(regionevent_id));
		Regioncallback *callback = reinterpret_cast<Regioncallback *>(callbackVoid);
		(*callback)(regionevent);
	}

	virtual void operator()(const Regionevent &regionevent) = 0;

protected:
	Regioncallback()
	{ }

public:
	virtual ~Regioncallback()
	{ }
};

class Regionnotifier
{
protected:
	cmzn_regionnotifier_id id;

public:

	Regionnotifier() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit Regionnotifier(cmzn_regionnotifier_id in_regionnotifier_id) :
		id(in_regionnotifier_id)
	{  }

	Regionnotifier(const Regionnotifier& regionNotifier) :
		id(cmzn_regionnotifier_access(regionNotifier.id))
	{  }

	Regionnotifier& operator=(const Regionnotifier& regionNotifier)
	{
		cmzn_regionnotifier_id temp_id = cmzn_regionnotifier_access(regionNotifier.id);
		if (0 != id)
		{
			cmzn_regionnotifier_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Regionnotifier()
	{
		if (0 != id)
		{
			cmzn_regionnotifier_destroy(&id);
		}
	}

	bool isValid() const
	{
		return (0 != id);
	}

	cmzn_regionnotifier_id getId() const
	{
		return id;
	}

	int setCallback(Regioncallback& callback)
	{
		return cmzn_regionnotifier_set_callback(id, callback.C_callback, static_cast<void*>(&callback));
	}

	int clearCallback()
	{
		return cmzn_regionnotifier_clear_callback(id);
	}
};

inline Regionnotifier Region::createRegionnotifier()
{
	return Regionnotifier(cmzn_region_create_regionnotifier(id));
}

}  // namespace Zinc
}

#endif
