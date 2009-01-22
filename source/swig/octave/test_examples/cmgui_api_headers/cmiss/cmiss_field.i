/*******************************************************************************
FILE : cmiss_field.i

LAST MODIFIED : 23 December 2008

DESCRIPTION :
Swig interface file for wrapping the Cmiss_command_data_execute_command 
==============================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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
 /* cmiss_field.i */

%module cmiss_field
%include carrays.i
%array_functions(float, float_array);

%{
#include "api/cmiss_field.h"
//typedef struct Cmiss_field *Cmiss_field_id;
//typedef struct Cmiss_node *Cmiss_node_id;

extern int Cmiss_field_get_number_of_components(Cmiss_field_id field);
extern int Cmiss_field_evaluate_at_node(Cmiss_field_id field,
	Cmiss_node_id node, float time, int number_of_values, float *values);
%}

extern int Cmiss_field_get_number_of_components(Cmiss_field_id field);
extern int Cmiss_field_evaluate_at_node(Cmiss_field_id field,
	Cmiss_node_id node, float time, int number_of_values, float *values);
