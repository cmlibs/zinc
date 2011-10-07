/***************************************************************************//**
 * tessellation.hpp
 *
 * Objects for describing how elements / continuous field domains are
 * tessellated or sampled into graphics.
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#if !defined (TESSELLATION_HPP)
#define TESSELLATION_HPP

extern "C" {
#include "api/zn_tessellation.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
}

struct Cmiss_graphics_module;
struct Option_table;
struct Parse_state;

/***************************************************************************//**
 * Object describing how elements / continuous field domains are  tessellated
 * or sampled into graphics.
 */
struct Cmiss_tessellation;

DECLARE_LIST_TYPES(Cmiss_tessellation);
DECLARE_MANAGER_TYPES(Cmiss_tessellation);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Cmiss_tessellation);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_tessellation,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Cmiss_tessellation);
PROTOTYPE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_tessellation,name,const char *);

/***************************************************************************//**
 * Private; only to be called from graphics_module.
 */
int Cmiss_tessellation_manager_set_owner_private(struct MANAGER(Cmiss_tessellation) *manager,
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//**
 * Create and return a handle to a new tessellation.
 * Private; only to be called from graphics_module.
 *
 * @return  Handle to the newly created tessellation if successful, otherwise NULL.
 */
struct Cmiss_tessellation *Cmiss_tessellation_create_private();

/***************************************************************************//**
 * Internal function returning true if the tessellation has coarse and fine
 * divisions both equal to the fixed divisions supplied.
 *
 * @param tessellation  The tessellation to query.
 * @param dimensions  The size of the fixed_divisions array.
 * @param fixed_divisions  Array of divisions to match.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_tessellation_has_fixed_divisions(Cmiss_tessellation_id tessellation,
	int dimensions, int *fixed_divisions);

/***************************************************************************//**
 * Function to process the string to be passed into an tessellation object.
 *
 * @param input  string to be passed into the function..
 * @param values_in  pointer of an array of int in which values will be written
 * 	 into.
 * @param size_in  pointer to an int which value will be assigned in this function.
 */
int string_to_divisions(const char *input, int **values_in, int *size_in);

/***************************************************************************//**
 * Adds a token to the option_table which if matched reads the following string
 * as a series of positive integers separated by *, mainly used for element
 * discretization.
 *
 * @param option_table  The command option table to add the entry to.
 * @param token  The required token for this option.
 * @param divisions_address  Address of where to allocate return int array.
 * Must be initialised to NULL or allocated array.
 * @param size_address  Address of int to receive array size. Must be
 * initialised to size of initial *divisions_address, or 0 if none.
 */
int Option_table_add_divisions_entry(struct Option_table *option_table,
	const char *token, int **divisions_address, int *size_address);

/***************************************************************************//**
 * Adds an entry to the <option_table> under the given <token> that selects a
 * Cmiss_tessellation from the graphics_module.
 * @param tessellation_address  Address of tessellation pointer which must be
 * NULL or ACCESSed.
 */
int Option_table_add_Cmiss_tessellation_entry(struct Option_table *option_table,
	const char *token, struct Cmiss_graphics_module *graphics_module,
	struct Cmiss_tessellation **tessellation_address);

/***************************************************************************//**
 * gfx define tessellation command.
 * @param state  Command parse state.
 * @param graphics_module_void  Cmiss_graphics_module.
 */
int gfx_define_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

/***************************************************************************//**
 * gfx destroy tessellation command.
 * @param state  Command parse state.
 * @param graphics_module_void  Cmiss_graphics_module.
 */
int gfx_destroy_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

/***************************************************************************//**
 * gfx list tessellation command.
 */
int gfx_list_tessellation(struct Parse_state *state, void *dummy_to_be_modified,
	void *graphics_module_void);

#endif
