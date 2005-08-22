/*******************************************************************************
FILE : computed_field_power_spectrum.h

LAST MODIFIED : 2 August 2004

DESCRIPTION :
Compute power spectrum on computed fields.
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
#if !defined (COMPUTED_FIELD_POWER_SPECTRUM_H)
#define COMPUTED_FIELD_POWER_SPECTRUM_H

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;
#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

int FFT_1d(FE_value *Xr, FE_value *Xi, int dir, int data_size);
/****************************************************************************
      LAST MODIFIED: 2 August 2004

      DESCRIPTION: Implement 1D fast Fourier transform
============================================================================*/
int FFT_md(FE_value *in_re, FE_value *in_im, int dir, int dim, int *sizes);
/****************************************************************************
      LAST MODIFIED: 2 August 2004

      DESCRIPTION: Implement 2D fast Fourier transform
============================================================================*/

int Computed_field_register_types_power_spectrum(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 2 August 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_POWER_SPECTRUM_H) */
