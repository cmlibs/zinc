/**
 * FILE : computed_field_apply.hpp
 *
 * Implements fields for applying the function of other fields, including from
 * other regions, with argument binding.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_APPLY_HPP)
#define COMPUTED_FIELD_APPLY_HPP

#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldalias.h"
#include "opencmiss/zinc/fieldapply.h"
#include "computed_field/computed_field_private.hpp"


class Computed_field_apply : public Computed_field_core
{
private:
	void *other_field_manager_callback_id;
	int numberOfBindings;

public:

	Computed_field_apply() :
		Computed_field_core(),
		other_field_manager_callback_id(nullptr),
		numberOfBindings(0)
	{
	}

	virtual bool attach_to_field(cmzn_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			this->checkApplyFromOtherRegion();
			return true;
		}
		return false;
	}

	~Computed_field_apply();

private:

	static void otherRegionFieldChange(
		MANAGER_MESSAGE(cmzn_field) *message, void *apply_field_core_void);

	void checkApplyFromOtherRegion(void);

	Computed_field_core* copy()
	{
		Computed_field_apply* core = new Computed_field_apply();
		return (core);
	};

	const char* get_type_string();

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_APPLY;
	}

	int compare(Computed_field_core *other_core)
	{
		return (nullptr != dynamic_cast<Computed_field_apply*>(other_core));
	}

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& /*fieldCache*/);

	/** Bind source fields before evaluate.
	 * @return  True on success */
	bool bindSourceFields(cmzn_fieldcache& cache, RealFieldValueCache &valueCache);

	/** Unbind source fields after evaluate. */
	void unbindSourceFields(cmzn_fieldcache& cache, RealFieldValueCache &valueCache);

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	/** Override as must bind source fields and check evaluate field is defined only */
	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, RealFieldValueCache& /*valueCache*/);

	virtual void fieldAddedToRegion(void)
	{
		checkApplyFromOtherRegion();
	}

	/** override to check dependencies of source fields from this region only.
	 * If evaluate field is from another region, it is handled by separate callbacks */
	int check_dependency();

public:

	static inline Computed_field_apply *coreCast(cmzn_field_apply *apply_field)
	{
		return (static_cast<Computed_field_apply*>(reinterpret_cast<cmzn_field*>(apply_field)->core));
	}

	inline cmzn_field *getEvaluateField(void)
	{
		return this->field->source_fields[0];
	}

	/** Get argument field at bind index.
	 * @param bindIndex  Index from 0 to getNumberOfBindings() - 1.
	 * @return  Non-accessed field or nullptr if none or invalid arguments. */
	cmzn_field *getBindArgumentField(int bindIndex) const;

	/** Get source field bound to argument field.
	 * @param argumentField  Argument field to check bind, from evaluate field's region.
	 * @return  Non-accessed field or nullptr if none or invalid arguments. */
	cmzn_field *getBindArgumentSourceField(cmzn_field *argumentField) const;

	/** Set source field bound to argument field.
	 * @param argumentField  Argument field to bind, from evaluate field's region.
	 * Must be of Argument type.
	 * @param sourceField  Source field from this region to bind, or nullptr to clear.
	 * @return  Result OK on success, otherwise error */
	int setBindArgumentSourceField(cmzn_field *argumentField, cmzn_field *sourceField);

	int getNumberOfBindings() const
	{
		return this->numberOfBindings;
	}

};


class Computed_field_argument_real : public Computed_field_core
{
public:

	Computed_field_argument_real() :
		Computed_field_core()
	{
	}

	virtual bool attach_to_field(cmzn_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			return true;
		}
		return false;
	}

	~Computed_field_argument_real()
	{
	}

private:

	Computed_field_core* copy()
	{
		return new Computed_field_argument_real();
	};

	const char* get_type_string();

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_ARGUMENT_REAL;
	}

	int compare(Computed_field_core* otherCore)
	{
		return (nullptr != dynamic_cast<Computed_field_argument_real*>(otherCore));
	}

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& /*fieldCache*/);

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	/** Override as only true if source field is bound and defined at location */
	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int list();

	char* get_command_string();

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, RealFieldValueCache& /*valueCache*/);

public:

	static inline Computed_field_argument_real *coreCast(cmzn_field_argument_real *argument_real_field)
	{
		return (static_cast<Computed_field_argument_real*>(reinterpret_cast<cmzn_field*>(argument_real_field)->core));
	}

};

#endif /* !defined (COMPUTED_FIELD_APPLY_HPP) */
