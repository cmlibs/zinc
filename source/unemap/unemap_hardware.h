/*******************************************************************************
FILE : unemap_hardware.h

LAST MODIFIED : 4 June 2003

DESCRIPTION :
Code for controlling the National Instruments (NI) data acquisition and unemap
signal conditioning cards.

NOTES :
1 The channels are numbered from 1 to the total number of channels
???DB.  Get rid of NI types
???DB.  Save
Operates on the group of channels specified by <channel_number>.  If
<channel_number> is between 1 and the total number of channels then the group is
((<channel_number>-1) div 64)*64+1 to ((<channel_number>-1) div 64)*64+64,
otherwise the group is all channels.
==============================================================================*/
#if !defined (UNEMAP_HARDWARE_H)
#define UNEMAP_HARDWARE_H

#if defined (WIN32_USER_INTERFACE)
#include <windows.h>
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
#include "user_interface/event_dispatcher.h"
#endif /* defined (UNIX) */

/*
Global types
------------
*/
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
???DB.  Used to be enum UNEMAP_hardware_version, but want to be able to do
	bitwise ors to get mixtures
???DB.  Replace UnEmap by UNEMAP when got rid of code switchs
==============================================================================*/
/* signal conditioning card used in second Oxford interim system */
#define UnEmap_1V2 (0x1)
/* signal conditioning card for Oxford final system */
#define UnEmap_2V1 (0x2)
/* current signal conditioning card */
#define UnEmap_2V2 (0x4)

/* see unemap_configure for a description of the arguments */
typedef void (Unemap_hardware_callback)(int,int *,int,short *,void *);

/* see unemap_calibrate for a description of the arguments */
typedef void (Unemap_calibration_end_callback)(const int,const int *, \
	const float *,const float *,void *);

/* see unemap_get_samples_acquired_background for a description of the
	arguments */
typedef void (Unemap_acquired_data_callback)(const int,const int, \
	const short *,void *);

/* see unemap_tranfer_samples_acquired for a description of the arguments */
typedef int (Unemap_transfer_samples_function)(short int *,int,void *);

/* see unemap_load_voltage_stimulating for a description of the arguments */
typedef int (Unemap_stimulation_end_callback)(void *);

/*
Global functions
----------------
*/
int unemap_configure(float sampling_frequency,int number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
	struct Event_dispatcher *event_dispatcher,
#endif /* defined (UNIX) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_frequency,float scrolling_callback_frequency,
	int synchronization_card);
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
Configures the hardware for sampling at the specified <sampling_frequency> and
with the specified <number_of_samples_in_buffer>. <sampling_frequency>,
<number_of_samples_in_buffer>, <scrolling_frequency> and
<scrolling_callback_frequency> are requests and the actual values used should
be determined using unemap_get_sampling_frequency,
unemap_get_maximum_number_of_samples, unemap_get_scrolling_frequency and
unemap_get_scrolling_callback_frequency.

Every <sampling_frequency>/<scrolling_callback_frequency> samples (one sample
	is a measurement on every channel)
- if <scrolling_callback> is not NULL, <scrolling_callback> is called with
	- first argument is the number of scrolling channels
	- second argument is an array of channel numbers (in the order they were added
		using unemap_set_scrolling_channel)
	- third argument is the number of values for each channel
		(<scrolling_frequency>/<scrolling_callback_frequency>)
	- fourth argument is an array of channel values (all the values for the first
		channel, then all the values for the second channel and so on)
	- fifth argument is the user supplied callback data
	- it is the responsibilty of the callback to free the channel numbers and
		values arrays
- for WIN32_USER_INTERFACE, if <scrolling_window> is not NULL, a
	<scrolling_message> for <scrolling_window> is added to the message queue with
	- WPARAM is an array of unsigned char amalgamating the five arguments above
		- first sizeof(int) bytes is the number of scrolling channels
		- next (number of scrolling channels)*sizeof(int) bytes are the scrolling
			channel numbers
		- next sizeof(int) bytes is the number of values for each channel
		- next (number of scrolling channels)*(number of values)*sizeof(short) bytes
			are the channel values
		- last sizeof(void *) bytes are the user supplied callback data
	- LPARAM is a ULONG specifying the number of bytes in the WPARAM array
		over the <scrolling_callback_period>) in the array
	- it is the responsibility of the window message handler to free the WPARAM
		array.

Initially there are no scrolling channels.

<synchronization_card> is the number of the NI card, starting from 1 on the
left, for which the synchronization signal is input if the crate is a slave, via
the 5-way cable, to the attached SCU.  All other SCUs output the synchronization
signal.
==============================================================================*/

int unemap_configured(void);
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Returns a non-zero if unemap is configured and zero otherwise.
==============================================================================*/

int unemap_deconfigure(void);
/*******************************************************************************
LAST MODIFIED : 14 January 1999

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/

int unemap_get_hardware_version(int *hardware_version);
/*******************************************************************************
LAST MODIFIED : 8 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version>.
==============================================================================*/

int unemap_get_software_version(int *software_version);
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
The function does not need the software to be configured.

Returns the unemap <software_version>.
==============================================================================*/

int unemap_shutdown(void);
/*******************************************************************************
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer.
==============================================================================*/

int unemap_set_scrolling_channel(int channel_number);
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback (see unemap_configure).
==============================================================================*/

int unemap_clear_scrolling_channels(void);
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback (see unemap_configure).
==============================================================================*/

int unemap_start_scrolling(void);
/*******************************************************************************
LAST MODIFIED : 29 April 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/

int unemap_stop_scrolling(void);
/*******************************************************************************
LAST MODIFIED : 29 April 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/

int unemap_calibrate(Unemap_calibration_end_callback *calibration_end_callback,
	void *calibration_end_callback_data);
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

When the calibration is completed <calibration_end_callback> is called with -
the number of channels calibrated, the channel numbers for the calibrated
channels, the offsets for the calibrated channels, the gains for the
calibrated channels and the <calibration_end_callback_data>.
==============================================================================*/

int unemap_start_sampling(void);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts the sampling.
==============================================================================*/

int unemap_stop_sampling(void);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops the sampling.  Use <unemap_get_number_of_samples_acquired> to find out how
many samples were acquired.
==============================================================================*/

int unemap_get_sampling(void);
/*******************************************************************************
LAST MODIFIED : 16 July 2000

DESCRIPTION :
Returns a non-zero if unemap is sampling and zero otherwise.
==============================================================================*/

int unemap_set_isolate_record_mode(int channel_number,int isolate);
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

If <isolate> is not zero the group is put in isolate mode, otherwise the group
is put in record mode.  In isolate mode, the electrodes (recording, stimulation
and reference) are disconnected from the hardware and all channels are connected
to a calibration signal.
==============================================================================*/

int unemap_get_isolate_record_mode(int channel_number,int *isolate);
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/

int unemap_set_antialiasing_filter_frequency(int channel_number,
	float frequency);
/*******************************************************************************
LAST MODIFIED : 24 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the anti-aliasing filter to the specified <frequency> for all channels.
3dB cutoff frequency = 50*tap
begin OLD_CODE
3dB cutoff frequency = 1.07/(100*R*C)
C = 10 nF
R = (99-tap) * 101 ohm
dnd OLD_CODE
<frequency> is a request and the actual value used can be found with
unemap_get_antialiasing_filter.
==============================================================================*/

int unemap_set_powerup_antialiasing_filter_frequency(int channel_number);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Makes the current anti-aliasing filter frequency the power up value for the
group.
==============================================================================*/

int unemap_get_antialiasing_filter_frequency(int channel_number,
	float *frequency);
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/

int unemap_get_number_of_channels(int *number_of_channels);
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels is assigned to <*number_of_channels>.
==============================================================================*/

int unemap_get_sample_range(int channel_number,long int *minimum_sample_value,
	long int *maximum_sample_value);
/*******************************************************************************
LAST MODIFIED : 6 August 2002

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The minimum possible sample value is assigned to <*minimum_sample_value> and the
maximum possible sample value is assigned to <*maximum_sample_value>.
==============================================================================*/

int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage);
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/

int unemap_get_number_of_samples_acquired(unsigned long *number_of_samples);
/*******************************************************************************
LAST MODIFIED : 11 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The number of samples acquired per channel since <unemap_start_sampling> is
assigned to <*number_of_samples>.
==============================================================================*/

int unemap_transfer_samples_acquired(int channel_number,int number_of_samples,
	Unemap_transfer_samples_function *transfer_samples_function,
	void *transfer_samples_function_data,int *number_of_samples_transferred);
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are transferred.  If
<channel_number> is 0 then the <samples> for all channels are transferred.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are transferred.  Otherwise,
<number_of_samples> are transferred. If <number_of_samples_transferred> is not
NULL, then it is set to the number of samples transferred.

The <transfer_samples_function> is used to transfer the samples.  It is called
with samples (short int *), number_of_samples (int) and
<transfer_samples_function_data>.  It should return the number of samples
transferred.
==============================================================================*/

int unemap_write_samples_acquired(int channel_number,int number_of_samples,
	FILE *file,int *number_of_samples_written);
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the samples for that channel are written to <file>.  If
<channel_number> is 0 then the samples for all channels are written to <file>.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are written.  Otherwise,
<number_of_samples> are written. If <number_of_samples_written> is not NULL,
then it is set to the number of samples written.
==============================================================================*/

int unemap_get_samples_acquired(int channel_number,int number_of_samples,
	short int *samples,int *number_of_samples_got);
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive, then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are got.  Otherwise,
<number_of_samples> are got. If <number_of_samples_got> is not NULL, then it is
set to the number of samples got.
==============================================================================*/

int unemap_get_samples_acquired_background(int channel_number,
	int number_of_samples,Unemap_acquired_data_callback *callback,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 13 January 2002

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are got.  Otherwise,
<number_of_samples> are got.

The function gets the samples specified by the <channel_number> and calls the
<callback> with the <channel_number>, the number of samples got, the samples and
the <user_data>.  The <callback> is responsible for deallocating the samples
memory.

When the function returns, it is safe to call any of the other functions
(including unemap_start_sampling), but the <callback> may not have finished or
even been called yet.  This function allows data to be transferred in the
background in a client/server arrangement.
==============================================================================*/

int unemap_get_maximum_number_of_samples(unsigned long *number_of_samples);
/*******************************************************************************
LAST MODIFIED : 11 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer, in number of samples per channel, is assigned to
<*number_of_samples>.
==============================================================================*/

int unemap_get_sampling_frequency(float *frequency);
/*******************************************************************************
LAST MODIFIED : 11 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The sampling frequency is assigned to <*frequency>.
==============================================================================*/

int unemap_get_scrolling_frequency(float *frequency);
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/

int unemap_get_scrolling_callback_frequency(float *frequency);
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling callback frequency is assigned to <*frequency>.
==============================================================================*/

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain);
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the gain before the band pass filter and the gain after the band pass
filter to the specified values.

For UNEMAP_1V1 there is no gain before the filter (<pre_filter_gain> ignored).
For UNEMAP_2V1 and UNEMAP_2V2 the gain before the filter can be 1, 2, 4 or 8.

For UNEMAP_1V1, the post filter gain can be 10, 20, 50, 100, 200, 500 or 1000
(fixed gain of 10)
For UNEMAP_2V1 and UNEMAP_2V2, the post filter gain can be 11, 22, 55, 110, 220,
550 or 1100 (fixed gain of 11).
==============================================================================*/

int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain);
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/

int unemap_load_voltage_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_voltages,float voltages_per_second,float *voltages,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_channels> is greater than 0 then the function applies to the group
that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to voltage stimulating and loads
the stimulating signal.  If <number_of_voltages> is 0 then the stimulating
signal is the zero signal.  If <number_of_voltages> is 1 then the stimulating
signal is constant at the <*voltages>.  If <number_of_voltages> is >1 then the
stimulating signal is that in <voltages> at the specified number of
<voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.

If <number_of_cycles> is zero then the waveform is repeated until
<unemap_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <unemap_stop_stimulating>.

The <stimulation_end_callback> is called with <stimulation_end_callback_data>
when the stimulation ends.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/

int unemap_load_current_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_currents,float currents_per_second,float *currents,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data);
/*******************************************************************************
LAST MODIFIED : 16 November 2001

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_channels> is greater than 0 then the function applies to the group
that is the union of ((channel_number-1) div 64)*64+1 to
((channel_number-1) div 64)*64+64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to the group of all
channels.  Otherwise, the function fails.

Sets all the stimulating channels in the group to current stimulating and loads
the stimulating signal.  If <number_of_currents> is 0 then the stimulating
signal is the zero signal.  If <number_of_currents> is 1 then the stimulating
signal is constant at the <*currents>.  If <number_of_currents> is >1 then the
stimulating signal is that in <currents> at the specified number of
<currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.

If <number_of_cycles> is zero then the waveform is repeated until
<unemap_stop_stimulating>, otherwise the waveform is repeated the
<number_of_cycles> times or until <unemap_stop_stimulating>.

The <stimulation_end_callback> is called with <stimulation_end_callback_data>
when the stimulation ends.

Use unemap_set_channel_stimulating to make a channel into a stimulating channel.
Use <unemap_start_stimulating> to start the stimulating.
==============================================================================*/

int unemap_start_stimulating(void);
/*******************************************************************************
LAST MODIFIED : 5 June 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts stimulation for all channels that have been loaded (with
unemap_load_voltage_stimulating or unemap_load_current_stimulating) and have not
yet started.
==============================================================================*/

#if defined (OLD_CODE)
int unemap_start_voltage_stimulating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages);
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_voltages> is 0 then the stimulating signal is the zero signal.  If
<number_of_voltages> is 1 then the stimulating signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the stimulating signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/

int unemap_start_current_stimulating(int channel_number,int number_of_currents,
	float currents_per_second,float *currents);
/*******************************************************************************
LAST MODIFIED : 16 April 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts stimulating down the channels that are in the group and have been
specified as stimulating with <unemap_set_channel_stimulating>.  If
<number_of_currents> is 0 then the stimulating signal is the zero signal.  If
<number_of_currents> is 1 then the stimulating signal is constant at the
<*currents>.  If <number_of_currents> is >1 then the stimulating signal is that
in <currents> at the specified number of <currents_per_second>.

The <currents> are those desired as a proportion of the maximum (dependent on
the impedance being driven).  The function sets <currents> to the actual values
used.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int unemap_stop_stimulating(int channel_number);
/*******************************************************************************
LAST MODIFIED : 15 April 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/

int unemap_set_channel_stimulating(int channel_number,int stimulating);
/*******************************************************************************
LAST MODIFIED : 11 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Zero <stimulating> means off.  Non-zero <stimulating> means on.  If
<channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/

int unemap_get_channel_stimulating(int channel_number,int *stimulating);
/*******************************************************************************
LAST MODIFIED : 11 February 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <stimulating> is set to 1 if <channel_number> is stimulating
and 0 otherwise.  Otherwise the function fails.
==============================================================================*/

int unemap_start_calibrating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages);
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Starts generating the calibration signal for the specified group.  If
<number_of_voltages> is 0 then the calibration signal is the zero signal.  If
<number_of_voltages> is 1 then the calibration signal is constant at the
<*voltages>.  If <number_of_voltages> is >1 then the calibration signal is that
in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.
==============================================================================*/

int unemap_stop_calibrating(int channel_number);
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/

int unemap_set_power(int on);
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/

int unemap_get_power(int *on);
/*******************************************************************************
LAST MODIFIED : 16 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on then <*on> is set to 1, otherwise <*on> is set to 0.
==============================================================================*/

int unemap_read_waveform_file(FILE *in_file,char *waveform_file_name,
	int *number_of_values,float *values_per_second,float **values,
	int *constant_voltage);
/*******************************************************************************
LAST MODIFIED : 12 November 2000

DESCRIPTION :
The function does not need the hardware to be configured.

A waveform suitable for stimulation is read from a file.  If
<*constant_voltage>, then the <*values> are voltages otherwise the <*values> are
proportions of the maximum current.
==============================================================================*/

int unemap_get_number_of_stimulators(int *number_of_stimulators);
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/

int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number);
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
#endif /* !defined (UNEMAP_HARDWARE_H) */
