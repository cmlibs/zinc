/*******************************************************************************
FILE : select_spectrum.c

LAST MODIFIED : 19 May 1998

DESCRIPTION :
Declares select widget functions for spectrum objects.
==============================================================================*/

#include "graphics/spectrum.h"
#include "select/select_spectrum.h"
#include "select/select_private.h"

/*
Module types
------------
*/
FULL_DECLARE_SELECT_STRUCT_TYPE(Spectrum);

/*
Module functions
----------------
*/
DECLARE_DEFAULT_SELECT_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(
	Spectrum)
DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(
	Spectrum)
DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(
	Spectrum)
DECLARE_DEFAULT_SELECT_MANAGER_CREATE_FUNCTION(Spectrum)

DECLARE_SELECT_MODULE_FUNCTIONS(Spectrum,"Material named:")

/*
Global functions
----------------
*/
DECLARE_SELECT_GLOBAL_FUNCTIONS(Spectrum)

