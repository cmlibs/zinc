/* @(#)04	1.3  com/contrib/PointerShape/hourglassM.h, aic, aic324, 9317324e 5/5/93 18:05:29 */
/*
 *  COMPONENT_NAME: AIC           AIXwindows Interface Composer
 *  
 *  ORIGINS: 58
 *  
 *  
 *                   Copyright IBM Corporation 1991, 1993
 *  
 *                         All Rights Reserved
 *  
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation, and that the name of IBM not be
 *   used in advertising or publicity pertaining to distribution of the
 *   software without specific, written prior permission.
 *  
 *   IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 *   ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *   PURPOSE. IN NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 *   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 *   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 *   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 *   OR PERFORMANCE OF THIS SOFTWARE.
*/
/*---------------------------------------------------------------------
 * $Date$             $Revision$
 *---------------------------------------------------------------------
 * 
 *
 *             Copyright (c) 1992, Visual Edge Software Ltd.
 *
 *  ALL  RIGHTS  RESERVED.  Permission  to  use,  copy,  modify,  and
 *  distribute  this  software  and its documentation for any purpose
 *  and  without  fee  is  hereby  granted,  provided  that the above
 *  copyright  notice  appear  in  all  copies  and  that  both  that
 *  copyright  notice and this permission notice appear in supporting
 *  used  in advertising  or publicity  pertaining to distribution of
 *  the software without specific, written prior permission. The year
 *  included in the notice is the year of the creation of the work.
 *
 *  VISUAL EDGE SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 *  SOFTWARE,  INCLUDING  ALL IMPLIED  WARRANTIES OF  MERCHANTABILITY
 *  AND  FITNESS FOR A  PARTICULAR PURPOSE.  IN NO EVENT SHALL VISUAL
 *  EDGE   SOFTWARE   BE   LIABLE   FOR  ANY  SPECIAL,   INDIRECT  OR
 *  CONSEQUENTIAL  DAMAGES  OR  ANY DAMAGES WHATSOEVER RESULTING FROM
 *  LOSS  OF  USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *  NEGLIGENCE  OR  OTHER  TORTIOUS  ACTION,  ARISING  OUT  OF  OR IN
 *  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *--------------------------------------------------------------------------*/
#define hourglass_mask_width 32
#define hourglass_mask_height 32
static char hourglass_mask_bits[] = {
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x01,
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03,
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x01,
   0x80, 0xff, 0xff, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xfe, 0x3f, 0x00,
   0x00, 0xfc, 0x1f, 0x00, 0x00, 0xf8, 0x0f, 0x00, 0x00, 0xf0, 0x07, 0x00,
   0x00, 0xe0, 0x03, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x07, 0x00,
   0x00, 0xf8, 0x0f, 0x00, 0x00, 0xfc, 0x1f, 0x00, 0x00, 0xfe, 0x3f, 0x00,
   0x00, 0xff, 0x7f, 0x00, 0x80, 0xff, 0xff, 0x00, 0xc0, 0xff, 0xff, 0x01,
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03,
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03, 0xc0, 0xff, 0xff, 0x01,
   0xe0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x03};
