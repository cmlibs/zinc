/*******************************************************************************
FILE : api/cmiss_variable_new_coordinates.cpp

LAST MODIFIED : 20 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new coordinates objects.
==============================================================================*/

#include <new>
#include "api/cmiss_variable_new_coordinates.h"
#include "computed_variable/variable_coordinates.hpp"

/*
Global functions
----------------
*/

Cmiss_variable_new_id
	Cmiss_variable_new_prolate_spheroidal_to_rectangular_cartesian_create(
	Scalar lambda,Scalar mu,Scalar theta,Scalar focus)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new prolate_spheroidal_to_rectangular_cartesian.
==============================================================================*/
{
	return (
#if defined (USE_SMART_POINTER)
		reinterpret_cast<Cmiss_variable_new_id>(new Variable_handle(
			new Variable_prolate_spheroidal_to_rectangular_cartesian(lambda,mu,theta,
			focus)))
#else /* defined (USE_SMART_POINTER) */
		reinterpret_cast<Cmiss_variable_new_id>(Variable_handle(
			new Variable_prolate_spheroidal_to_rectangular_cartesian(lambda,mu,theta,
			focus)))
#endif /* defined (USE_SMART_POINTER) */
		);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_prolate(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the prolate input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(
#endif /* defined (USE_SMART_POINTER) */
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_prolate()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_lambda(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the lambda input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(
#endif /* defined (USE_SMART_POINTER) */
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_lambda()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_mu(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the mu input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(
#endif /* defined (USE_SMART_POINTER) */
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_mu()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_theta(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the theta input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(
#endif /* defined (USE_SMART_POINTER) */
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_theta()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_focus(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian)
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the focus input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/
{
	Cmiss_variable_new_input_id result;
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle
		variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#if defined (USE_SMART_POINTER)
	Variable_handle *variable_handle_address;
#else /* defined (USE_SMART_POINTER) */
	Variable *variable_address;
#endif /* defined (USE_SMART_POINTER) */

	result=0;
	if (
#if defined (USE_SMART_POINTER)
		(variable_handle_address=reinterpret_cast<Variable_handle *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		boost::dynamic_pointer_cast<
		Variable_prolate_spheroidal_to_rectangular_cartesian,Variable>(*
		variable_handle_address))
#else /* defined (USE_SMART_POINTER) */
		(variable_address=reinterpret_cast<Variable *>(
		variable_prolate_spheroidal_to_rectangular_cartesian))&&
		(variable_prolate_spheroidal_to_rectangular_cartesian_handle=
		dynamic_cast<Variable_prolate_spheroidal_to_rectangular_cartesian *>(
		variable_address))
#endif /* defined (USE_SMART_POINTER) */
		)
	{
		result=reinterpret_cast<Cmiss_variable_new_input_id>(
#if defined (USE_SMART_POINTER)
			new Variable_input_handle(
#endif /* defined (USE_SMART_POINTER) */
			variable_prolate_spheroidal_to_rectangular_cartesian_handle->
			input_focus()
#if defined (USE_SMART_POINTER)
			)
#endif /* defined (USE_SMART_POINTER) */
			);
	}

	return (result);
}
