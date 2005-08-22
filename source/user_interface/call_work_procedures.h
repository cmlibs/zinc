/*******************************************************************************
FILE : call_work_procedures.h

LAST MODIFIED : 4 March 2002

DESCRIPTION :
SAB. This module provides some private X functions so that we can call timer events
and work procedures when necessary.  Unfortunately Xt does not provide the
necessary hooks in its public interface.  These functions and structure
definitions are reimplemented here, so that if a particular X implementation
did not implement these structures the same, this code would not work.
This small amount of cheating enables us to wrestle main loop control off X.
The code is basically copied from the X Window System, Version 11, Releae 6.4
from the Open Group X Project Team 30 January 1998
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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

Boolean XtCallWorkProc(XtAppContext app);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Exporting this private Xt function so that we can call it from user_interface.
==============================================================================*/

int XtTimeForTimeout(XtAppContext app, long *sec, long *usec);
/*******************************************************************************
LAST MODIFIED : 4 March 2002

DESCRIPTION :
Exporting this private Xt function so that we can call it from user_interface.
==============================================================================*/
