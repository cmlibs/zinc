/*******************************************************************************
FILE : unemap_hardware.c

LAST MODIFIED : 20 August 2003

DESCRIPTION :
Code for controlling the National Instruments (NI) data acquisition and unemap
signal conditioning cards.

SOFTWARE_VERSION:
	2 June 2003.  Added so that applications can deal with different versions of
		the hardware service
3 2 June 2003.  Replaces SERVICE_VERSION in
		unemap_hardware_service/unemap_hardware_service.c
	4 June 2003.  For unemap_configure added <scrolling_frequency> and renamed
		<scrolling_refresh_frequency> to <scrolling_callback_frequency>.  Added
		unemap_get_scrolling_frequency and unemap_get_scrolling_callback_frequency.
		This is to allow more control of scrolling
4 10 August 2003.  Allowed being able to sample a subset of the channels.  This
		allows faster sampling.  Specify the channels in unemap_configure.

CODE SWITCHS :
WIN32_SYSTEM - uses Windows GDI (graphics device interface?)
WIN32_USER_INTERFACE - uses Windows GDI (graphics device interface?)
MIRADA - first Oxford interim system, based on Mirada card (???DB.  Not yet
	included)
NI_DAQ - uses National Instruments PCI or PXI data acquisition cards
	Now the same object will work for all hardware versions
CALIBRATE_SIGNAL_SQUARE - alternative is a sine wave
SWITCHING_TIME - for calculating gains during calibration
SYNCHRONOUS_STIMULATION - allows stimulators to be started at the same time
	USE_WFM_LOAD - start using WFM_LOAD and WFM_Group_Control rather than WFM_OP
???DB.  If works then don't need a lot of the start_stimulation_* globals
	DA_INTERRUPTS_OFF - old way of handling short waveform buffers, now pad
		buffers

NOTES :
1 Problem with NT crashing (blue screen) when using a large acquisition buffer
	Stop message is
	0x0000000A IRQL_NOT_LESS_OR_EQUAL
	A  E1429FB4
	B        1F
	C         0
	D  80118149
	Search MSDN for IRQL_NOT_LESS_OR_EQUAL to get information
	NB Also get the stop message
	0x0000003F NO_MORE_SYSTEM_PTES
	A         0
	B       53A memory requested
	C      133A free system memory
	D      2D0B total system memory
	(Also see unemap_hardware_service.c)
1.1 Steps to reproduce problem on esp33 (PXI crate, 32180KB, not 32MB)
1.1.1 Have
unemap.*.samplingFrequencyHz: 5000
unemap.*.numberOfSamples: 100000
			in Unemap.  This ensures that will ask for buffer bigger than available
			RAM.  A high sampling rate is not necessary, but means that the buffer
			fills quickly
1.1.2 Set the environment variable UNEMAP_OS_MEMORY_MB, for the hardware
			service, to 13 (need to re-boot NT for it to take affect)
1.1.3 Start unemap acquisition with Cancel (default) for configuration
1.1.4 Leave in isolate mode, start test signal, start sampling and wait for
			buffer to fill
1.1.5 Stop sampling and save signals to file.  Wait until saving finished (file
			written) on client (Linux)
1.1.6 Start sampling again.  This is where NT will crash.  If it doesn't, exit
			unemap and repeat 1.1.3-6 .  If can repeat 10 times without crashing then
			problem is fixed
1.2 Tried many things, mostly using #defines,
1.2.1 NO_SCROLLING_CALLBACK
			Don't have a callback after every so many scans ie don't use
			Config_DAQ_Event_Message and scrolling_callback_NI .  This fixes the
			problem and suggests that, during the saving, the address of the callback
			function or the settings for the event message are being corrupted and
			the service jumps into protected memory.
			It is not the module_scrolling_callback that is being corrupted
			(commenting everything except the updating of buffer position and size
			out of scrolling_callback_NI doesn't fix the problem)
1.2.2 CLEAR_DAQ_FOR_START_SAMPLING
			Do a DAQ_Clear, and set up again the SCAN and event messages.  Does not
			fix the problem
1.2.3 USE_VIRTUAL_LOCK
			Thought that it may be that the buffer isn't being locked in physical
			memory properly, so tried a different way.  Doesn't fix the problem.
			Still not sure about locking, because its SCAN_Start that fails when the
			buffer is too big - error message is
			unemap_configure.  SCAN_Start failed.  -10447.  3
			This means "The operating environment is unable to grant a page lock"
1.2.4 USE_INTERRUPTS_FOR_AI
			Thought that it may be a DMA conflict between the file I/O (saving
			signals to disk) and the sampling.  Changed to interrupts for sampling.
			This means that can't sample as fast (reduce rate to 1000).  Doesn't fix
			the problem
1.2.5 WIN32_IO
      Thought that may be something wrong with the generic file I/O (fopen,
			etc), so swapped to the Microsoft ones.  Doesn't fix the problem
1.2.6 Upgrade to Service Pack 6a for NT.  Doesn't fix the problem
1.2.7 RECONFIGURE_FOR_START_SAMPLING, RECONFIGURE_FOR_START_SAMPLING_1,
			RECONFIGURE_FOR_START_SAMPLING_2 and RECONFIGURE_FOR_START_SAMPLING_3
			Do a unemap_deconfigure and unemap_configure (also have set up and start
			scrolling again) before starting sampling.  First actually called
			unemap_deconfigure and unemap_configure.  Then copied code in.  This fixes
			the problem.  Disadvantage is that DA has been turned off and scrolling
			isn't general
1.2.8 RECONFIGURE_FOR_START_SAMPLING, RECONFIGURE_FOR_START_SAMPLING_2 and
			RECONFIGURE_FOR_START_SAMPLING_3
			Don't free and allocate again the buffers.  This fixes the problem.  Only
			differences with CLEAR_DAQ_FOR_START_SAMPLING seem to be that waveform
			generation and scrolling are stopped and scrolling is set up and started
			again
1.2.9 RECONFIGURE_FOR_START_SAMPLING and RECONFIGURE_FOR_START_SAMPLING_3
			Don't do a DAQ_Clear (clears event messages although the function
			description gives the impression that it doesn't) or set up the event
			message.  This fixes the problem
1.2.10 Upgrade to version 6.6 of NI-DAQ.  Doesn't fix the problem
1.2.11 Upgrade to version 6.9.1 of NI-DAQ.  Seems to fix the problem, but
			reappears when change UNEMAP_OS_MEMORY_MB to 12
1.2.12 RECONFIGURE_FOR_START_SAMPLING
			Don't stop waveform generation.  Doesn't fix the problem
1.2.13 STOP_WAVEFORM_IN_START_SAMPLING
			This fixes the problem.  If the WFM_Group_Control(CLEAR) is only done for
			the first unemap_start_sampling, then it doesn't work.  If the
			WFM_Group_Control(CLEAR) is done for the second, but not the first,
			unemap_start_sampling, then it does fix the problem.  If
			WFM_Group_Control(CLEAR) is only done for the first card, then it does not
			fix the problem
1.2.14 DA_INTERRUPTS_OFF
			Doesn't fix the problem
1.2.15 USE_ACPI
			The NI website has DRIVER_IRQL_NOT_LESS_OR_EQUAL associated with GPIB and
			ACPI (Advanced Power Configuration Interface).  Modified service.c to
			respond to the power management messages.  To do this need to use
			RegisterServiceCtrlHandlerEx which is not in NT (in 2000)
1.2.16 Windows versions of unemap
			On a different machine (running Windows) the problem remains.
			On the same machine without the hardware service.  Seems to be paging,
				when tried to stop with task manager, NT crashed with the same error
				code.  When increased OS memory to 22MB, the same thing happened.
				- still crashs if just write a dummy file eg replace (#if) the then
					clause of "if (output_file=fopen(file_name,"wb"))" in
					save_write_signal_file with
						fwrite("ok5",1,3,output_file);
						return_code=1;
						fclose(output_file);
				- it is to do with the event messages (Config_DAQ_Event_Message)
					- if remove body of scrolling_callback_NI will still crash
					- if don't have testing will still crash
					- can reduce buffer size and sampling rate and will still crash
				- it is to do with memory swapping - having a save window for the
					signal file causes memory swapping, alternatively, opening a file
					outside of unemap causes memory swapping
				- SCAN_Start must start a separate thread/process - calls to
					Config_DAQ_Event_Message before it change the events for the
					acquistion and calls after don't
1.2.17 Windows 2000
			Installed Windows 2000 on the D: partition with NTFS
1.2.17.1 Started with NT on C: and parallel port CD drive (PPCD) installed
1.2.17.2 Set PP to ECP in BIOS (was EPP) - needed for w2k PPCD drivers
1.2.17.3 Boot from Windows98 startup floppy disk (modified to have DOS PPCD
				driver - dosepatc.exe from www.scmmicro.com).  Could use w2k startup
				disks (pressing F6 to say have SCSI driver and using epatap2k driver
				disk, from scmmicro, with the txtsetup.oem that I created) now that
				PP set up right in BIOS)
1.2.17.4 Put Windows 2000 CD in PPCD and run
				F:\i386\winnt.exe
				where F is the CD drive.  Format D: as NTFS, copy files, reboot and
				finish installation
1.2.17.5 Install epatap2k driver
			- performs poorly because only 32MB (recommended minimum 64MB)
			- still crashes (even with NiDaq 6.9.1)
			- tried USE_ACPI, but still crashs/locks and doesn't get any messages
1.2.18 Windows NT
			Installed Windows NT on the D: partition with FAT
1.2.18.1 EITHER
					Started with NT on C: and parallel port CD drive (PPCD) installed
				OR
					Boot from Windows98 startup floppy disk (modified to have DOS PPCD
						driver - dosepatc.exe from www.scmmicro.com)
					If want to reformat the partition (in same filesystem) will need to
						reformat now
						e:
						format c:
1.2.18.2 Put NI NT4 recovery disk in PPCD and ran
				F:\os\nt40\i386\winnt_32.exe
				where F is the CD drive.  Prompted for and wrote 3 NT setup floppies.
				Format D: as FAT
???DB.  Give details
1.2.18.1 NIDAQ 5.1
				- doesn't support 6071E cards
1.2.18.2 NIDAQ 6.1
				- locks up with unemap_29jul00.exe (does not use a service, sampling at
					1kHz, 5000 samples)
				- locks up with unemap_ni_14mar02.exe (does not use service)
				- locks up with unemap_14mar02.exe and
					unemap_hardware_service_14mar02.exe
1.2.18.3 NIDAQ 6.6.  Message during installation about missing ordinal.  I think
				that it was "Ordinal 6453 could not be located in dynamic link library
				mfc42.dll".  Similar problem described on ni.com with LabView 5.0
				installation - mfc42.dll is used by InstallShield and is missing or the
				wrong version (maybe I need service packs)
				- locks up with unemap_29jul00.exe (does not use service)
				- locks up with unemap_ni_14mar02.exe (does not use service)
				- locks up with unemap_14mar02.exe and
					unemap_hardware_service_14mar02.exe
1.2.18.4 Install NT SP3
1.2.19 9 May 2002.  Is the problem because have fully populated system?
			Removed all but 1 of the PXI6071Es
1.2.19.1 unemap_ni crashed using NT on C:
1.2.19.2 10 May 2002.  unemap_ni locked up using NT on D:
1.2.20 11 May 2002.  Set NO_SCROLLING_CALLBACK.  unemap_ni OK with NT on C:
1.2.21 12 May 2002.  Added and set NO_SCROLLING_BODY (scrolling_callback_NI is
				then just a stub).  unemap_ni OK with NT on C:
1.2.22 13 May 2002.  Added and set
			SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY.  unemap_ni OK with NT on
			C: .  This contradicts 1.2.1 .  1.2.1 was done with the hardware service
			(sockets), which may explain the change?
1.2.23 Added and set NO_MODULE_SCROLLING_CALLBACK.  unemap_ni OK with NT on C:
1.2.24 Started investigating cutting things out of scrolling_hardware_callback
			in page_window.
1.2.24.1 Added and set NO_SCROLLING_HARDWARE_CALLBACK_BODY.  unemap_ni OK with
				NT on C:
1.2.24.2 Added and set NO_SCROLLING_SIGNAL.  unemap_ni crashed using NT on C:
1.2.24.3 Added and set NO_SCROLLING_WINDOW_UPDATE.  unemap_ni OK with NT on C:
				The last section cut out was at the start involving GetDC and
				GetClientRect - unemap_ni crashs is they are left in.  So it seems to
				be the Windows calls (graphics or socket) in the callback that cause the
				problem.
1.2.25 Maybe NIDAQ is calling the callback again before its come back from the
			first callback.  Added and set UNEMAP_THREAD_SAFE, which uses mutex's to
			make the body of the callback be skipped if it is called before it
			returns.  unemap_ni crashed using NT on C:  The NIDAQ documentation says
			that the callback is called in the context of the process and is not
			called again before the previous call returns
1.2.26 Is it to do with the length of time the callback takes?  Tried changing
			the scrolling rate, unemap_configure in start_experiment from 25Hz to 1Hz.
			unemap_ni crashed using NT on C:
1.2.27 24 May 2002.  ReleaseDC needs to be called for each GetDC for a common
			DC.  I am using a private DC because of the CS_OWNDC style in
			RegisterClassEx.  CS_OWNDC is so that you should only need to GET_DC_ONCE.
			So I implemented this.  unemap_ni OK with NT on C:
1.2.27.1 25 May 2002.  Changed to unemap with unemap_hardware_service, 7 6071Es
				100000 samples, 1 kHz and UNEMAP_OS_MEMORY_MB set to 13 (was 22).  After
				scrolling for a while, it crashed NT when I tried to stop sampling
1.2.27.2 25 May 2002.  Back to unemap_ni, now with 7 6071Es, 100000 samples,
				1 kHz and UNEMAP_OS_MEMORY_MB set to 13.  After scrolling for a while,
				it crashed NT when I tried to stop sampling.  After scrolling for a
				while, it crashed NT when I clicked on the acquisition window title bar
				(might have been a double click which takes to full screen)
1.2.27.3 25 May 2002.  Back to unemap_ni, now with 1 6071E, 300000 samples,
				1 kHz and UNEMAP_OS_MEMORY_MB set to 13.  After clicking Experiment
				there were some messages
				The system is running low on virtual memory ...
				SCAN_Start failed.  -10810.  0.  9658880
				SCAN_Start failed.  -10447.  0.  8450560
				SCAN_Start failed.  -10447.  0.  7846400
				SCAN_Start failed.  -10447.  0.  7544320
				NB
				-10447 is a memPageLockError
				-10810 is an internalDriverError
				Still working.  Maximum number of samples 115520 (text widget shows
				11152 because not wide enough).  Something is wrong with what I wrote
				down - shouldn't be all zeros for second number, third number should
				be bigger.
				OK after scrolling overnight - double click title bar, start/stop
				sampling
1.2.27.3 26 May 2002.  unemap with unemap_hardware_service, 1 6071E, 300000
				samples, 1 kHz and UNEMAP_OS_MEMORY_MB set to 13.  Was able to get a
				bigger sample buffer (150920), but was able to get same when retried
				1.2.27
1.2.27.4 26 May 2002.  SCAN_Start is clearly locking the buffer into physical
				memory.  So, why am I using GlobalAlloc and GlobalLock?  SCAN_Start
				requires the buffer to be 4 byte aligned.  GlobalAlloc guarantees 8 byte
				aligned.  Should I be using GMEM_FIXED instead of GMEM_MOVEABLE?  Then
				wouldn't need GlobalLock.  Set up and defined USE_GMEM_FIXED.
				unemap_ni.exe with 7 6071Es, 300000 samples, 1 kHz and
				UNEMAP_OS_MEMORY_MB set to 13 crashed using NT on C:  
1.2.27.5 27 May 2002.  Changed back to unemap_ni_24may02.exe
1.2.27.6 27 May 2002.  Set up and defined USE_MALLOC.
				unemap_ni.exe with 7 6071Es, 300000 samples, 1 kHz and
				UNEMAP_OS_MEMORY_MB set to 13
???DB.  Locking memory twice?
	Try GMEM_FIXED
	Try malloc
???DB.  Can exception handling (Visual C/C++) be used to stop NT from crashing
	and localize the problem?
???DB.  Previous problems solved by NI
1 turn off the end of buffer interrupts
	status=AO_Change_Parameter((module_NI_CARDS[i]).device_number,
		da_channel,ND_LINK_COMPLETE_INTERRUPTS,ND_OFF);
2 "I hope that your trip to England is going well! OK, do you ever have one of
	those moments where you just feel like a real moron because you missed the
	answer because you were looking too hard?  Well, this is one of those
	cases.  The solution is to call WFM_Group_Control for boards 2 and 3 BEFORE
	you call it for board 1.  That way, the boards are ready and waiting for
	the trigger signal when it gets sent from board one.  What was happening
	was that board 1 WAS sending out the trigger signal, but it was doing in
	before the other 2 were ready to receive it.  I tested this today, and it
	works like a champ, no external routing.  Again, I am so sorry that I over
	looked the obvious here." (from Jason Reid, 27 Nov 2000)
1.3 11 August 2003.  Got IRQL_NOT_LESS_OR_EQUAL when
		- using SOFTWARE_VERSION 3
		- had basic cycle length pacing going (from pacing window)
		- changed to isolate
		- did Calibrate
		Not reproducible

NO_BATT_GOOD_WATCH - no thread watching BattGood

???To do
1 Try moving pulse generation set up from unemap_start_sampling to
	unemap_configure.  Depends what ND_RESET does.
2 Set up stimulation
3 Set up calibration
4 Propagate relay_power_on_UnEmap2vx ?

???To do before Peng's system is shipped
1 Asynchronous signal file saving.  Needs to be made thread safe for this
2 Synchronous stimulation
	see SYNCHRONOUS_STIMULATION

???DB.  Adding mixing 12-bit and 16-bit cards.  Restrict sampling rate to
16 bit rate
1 Remove restriction from search_for_NI_cards.  DONE
2 Check all cards for 16 bit in unemap_configure.  DONE
3 How to do across crates?
3.1 Add unemap_get_maximum_sampling_frequency (or have
		unemap_get_sampling_frequency return maximum when not configured)?  NO
3.2 Just deconfigure and configure again if different?  DONE

???DB.  Adding being able to sample faster on fewer channels
0 Add version information to -remove and install
0.1 Check changes to do with service version in service.h.  DONE
0.2 Add SOFTWARE_VERSION and date to service displayed name.  DONE
1 Change SOFTWARE_VERSION.  DONE
2 Add channels to be configured to unemap_configure
	???DB.  Different numbers of channels on different cards?  OK - on SCAN
		trigger runs through its scan sequence
	???DB.  No channels being sampled on card or crate?
2.1 hardware_buffer_size.  DONE
2.2 NUMBER_OF_CHANNELS_ON_NI_CARD.  Do again after add valid channel check. DONE
2.3 argument list.  DONE
3 Add configured channels/scan list to NI_card
3.1 struct NI_card.  DONE
3.2 get rid of channel_vector.  DONE
3.3 search_for_NI_cards.  DONE
3.4 unemap_configure.  DONE
3.5 module_start_sampling_card.  So, can have cards with no scanned channels
	not scanned at all.  What about outputting scan trigger from these cards?
	What about a crate with no scanned channels?  Alternatively, always sample at
	least one channel per card - add number_of_configured channels?  Decided to
	always scan at least one channel per card.  DONE
3.6 channel_reorder.  DONE
4 Add valid channel check to
4.1 unemap_set_scrolling_channel.  DONE
4.2 unemap_set_isolate_record_mode.  DONE
4.3 unemap_get_isolate_record_mode.  DONE
4.4 unemap_set_antialiasing_filter_frequency.  DONE
4.5 unemap_set_powerup_antialiasing_filter_frequency.  DONE
4.6 unemap_get_antialiasing_filter_frequency.  DONE
4.7 unemap_get_sample_range.  DONE
4.8 unemap_get_voltage_range.  DONE
4.9 unemap_transfer_samples_acquired.  DONE
4.10 unemap_write_samples_acquired.  DONE
4.11 unemap_get_samples_acquired.  DONE
4.12 unemap_get_samples_acquired_background.  DONE
4.13 unemap_set_gain.  DONE
4.14 unemap_get_gain.  DONE
4.15 unemap_load_voltage_stimulating.  DONE
4.16 unemap_load_current_stimulating.  DONE
4.17 unemap_stop_stimulating.  DONE
4.18 unemap_set_channel_stimulating.  DONE
4.19 unemap_get_channel_stimulating.  DONE
4.20 unemap_start_calibrating.  DONE
4.21 unemap_stop_calibrating.  DONE
4.22 unemap_channel_valid_for_stimulator.  DONE
4.23 channel_number.  DONE
5 unemap_get_number_of_channels.  Gets maximum number.  DONE
6 write/transfer/get channels in configured order.  DONE
???DB.  Check that scan channels are 0 to 63
==============================================================================*/

#define USE_MALLOC

#define SYNCHRONOUS_STIMULATION 1
/*#define DA_INTERRUPTS_OFF 1*/
#define USE_WFM_LOAD 1

/*#define CALIBRATE_SIGNAL_SQUARE 1*/
#define SWITCHING_TIME 1

#if defined (OLD_CODE)
/* the signal conditioning card version */
#define UNEMAP_2V1 1
#endif /* defined (OLD_CODE) */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#if defined (UNEMAP_THREAD_SAFE) && defined (UNIX)
#include <pthread.h>
#endif /* defined (UNEMAP_THREAD_SAFE) && defined (UNIX) */
#if defined (NI_DAQ)
#include "nidaq.h"
/* for RTSI */
#include "nidaqcns.h"
#endif /* defined (NI_DAQ) */
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "unemap/unemap_hardware.h"

/*
Module constants
----------------
*/
#define SOFTWARE_VERSION 4

#if !defined (DA_INTERRUPTS_OFF)
#define MINIMUM_NUMBER_OF_DA_POINTS 500
#endif /* !defined (DA_INTERRUPTS_OFF) */

#if defined (NI_DAQ)
#define PCI6031E_DEVICE_CODE 220
#define PCI6033E_DEVICE_CODE 222
#define PXI6031E_DEVICE_CODE 259
#define PXI6071E_DEVICE_CODE 258
#define DIGITAL_IO_COUNTER ND_COUNTER_1
#define NUMBER_OF_CHANNELS_ON_NI_CARD ((i16)64)
#define SCAN_COUNTER ND_COUNTER_0
/*???DB.  Would like CALIBRATE_CHANNEL and STIMULATE_CHANNEL to be constants,
	but I think that they swapped between UnEmap_1V2 and UnEmap_2V1.  Also,
	they were both 1 for the last UnEmap_1V2 executable */
i16 CALIBRATE_CHANNEL=0,STIMULATE_CHANNEL=1;
#define PC_LED_SHIFT_REGISTER_UnEmap2vx 1
#define Stim_Source_SHIFT_REGISTER_UnEmap2vx 2
#define EXT_ISO_SHIFT_REGISTER_UnEmap2vx 3
#define D_CAL_SHIFT_REGISTER_UnEmap2vx 4
#define D_REC_SHIFT_REGISTER_UnEmap2vx 5
#define FCS_SHIFT_REGISTER_UnEmap2vx 6
#define FUD_SHIFT_REGISTER_UnEmap2vx 7
#define Master_SHIFT_REGISTER_UnEmap2v1 8
#define Batt24_SHIFT_REGISTER_UnEmap2vx 14
#define BattB_SHIFT_REGISTER_UnEmap2vx 15
#define BattA_SHIFT_REGISTER_UnEmap2vx 16
/* the first stimulate channel */
#define Stim_Base_SHIFT_REGISTER_UnEmap2vx 17
#define MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx (float)4.5
/*???DB.  Will eventually be a constant ? */
/*???DB.  Not happy with settling constants ?  Perhaps OK because dealing with
	448 channels rather than just 64 (register) */
#define CALIBRATE_FREQUENCY ((float)100)
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
float switching_time=(float)0.0003;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
float tol_settling;
int settling_step_max;
float initial_antialiasing_filter_frequency;
/*???debug */
	/*???DB.  Read in, but no longer used */
int number_of_valid_ni_channels=64;
#endif /* defined (NI_DAQ) */

/*
Module types
------------
*/
#if defined (NI_DAQ)
enum NI_card_type
/*******************************************************************************
LAST MODIFIED : 6 August 1999

DESCRIPTION :
???DB.  Might just want to have if its AD or AD_DA ? 
==============================================================================*/
{
	PCI6031E_AD_DA,
	PCI6033E_AD,
	PXI6031E_AD_DA,
	PXI6071E_AD_DA
}; /* enum NI_card_type */

struct NI_card
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
Information associated with each NI card/signal conditioning card pair.
==============================================================================*/
{
	enum NI_card_type type;
	/* to make the pinout for UnEmap_1V2 the same as for later versions */
	int channel_reorder[NUMBER_OF_CHANNELS_ON_NI_CARD];
	/* to allow sampling on a subset of the channels */
	i16 number_of_configured_channels,
		configured_channels[NUMBER_OF_CHANNELS_ON_NI_CARD],
		hardware_buffer_offsets[NUMBER_OF_CHANNELS_ON_NI_CARD];
	i16 device_number;
	HGLOBAL memory_object;
	i16 *hardware_buffer;
	i16 gain,input_mode,polarity,time_base;
	u16 sampling_interval;
	u32 hardware_buffer_size;
	int unemap_hardware_version;
	int anti_aliasing_filter_taps;
	i16 *da_buffer;
	u32 da_buffer_size;
	int stimulation_end_callback_id;
	Unemap_stimulation_end_callback *stimulation_end_callback;
	void *stimulation_end_callback_data;
	struct
	{
		/* the current calibrate/record mode of the card */
		unsigned char isolate_mode;
		/* the stimulate channel settings.  Need to be saved while in calibrate
			mode */
		unsigned char stimulate_channels[8];
		/* the current settings of the NI output lines (that are not for control) */
		unsigned char gain_output_line_0,gain_output_line_1;
		/* the current settings of the 80 shift register digital outputs */
		unsigned char shift_register[10];
	} unemap_2vx;
}; /* struct NI_card */
#endif /* defined (NI_DAQ) */

/*
Module variables
----------------
*/
#if defined (NI_DAQ)
#if defined (OLD_CODE)
/*???DB.  Now in card */
/*???DB.  Can't free da_buffer while analog output is in progress */
i16 *da_buffer=(i16 *)NULL;
#endif /* defined (OLD_CODE) */
i16 filter_increment_output_line;
i16 chip_select_line_UnEmap1vx,id_line_1_UnEmap1vx,id_line_2_UnEmap1vx,
	id_line_3_UnEmap1vx,led_line_UnEmap1vx,up_down_line_UnEmap1vx;
int relay_power_on_UnEmap2vx=0;
/* battery_A_line_UnEmap2v1/master_line_UnEmap2v2 is output for the first NI
	card and input for the others */
i16 battery_A_line_UnEmap2v1,battery_good_input_line_UnEmap2vx,
	gain_0_output_line_UnEmap2vx,gain_1_output_line_UnEmap2vx,
	master_line_UnEmap2v2,shift_register_clock_output_line_UnEmap2vx,
	shift_register_data_output_line_UnEmap2vx,
	shift_register_strobe_output_line_UnEmap2vx;
i16 BattA_setting_UnEmap2vx;
	/*???DB.  Get rid of BattA_setting_UnEmap2vx */
/*???debug */
char first_sample=1;
unsigned long max_sample_number,min_sample_number;
#endif /* defined (NI_DAQ) */

/* scrolling information set by unemap_configure and
	unemap_set_scrolling_channel */
Unemap_hardware_callback
	*module_scrolling_callback=(Unemap_hardware_callback *)NULL;
void *module_scrolling_callback_data=(void *)NULL;
#if defined (WIN32_USER_INTERFACE)
UINT module_scrolling_message=(UINT)0;
HWND module_scrolling_window=(HWND)NULL;
#endif /* defined (WIN32_USER_INTERFACE) */
int module_number_of_scrolling_channels=0,
	module_number_of_scrolling_values_per_channel=1,
	module_scrolling_callback_period=1,
	*module_scrolling_channel_numbers=(int *)NULL;
int module_sampling_on=0,module_scrolling_on=1;
#if defined (UNEMAP_THREAD_SAFE)
#if defined (WIN32_SYSTEM)
HANDLE scrolling_callback_mutex=(HANDLE)NULL;
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
pthread_mutex_t *scrolling_callback_mutex=(pthread_mutex_t *)NULL,
	scrolling_callback_mutex_storage;
#endif /* defined (UNIX) */
/*???debug */
int scrolling_callback_mutex_locked_count=0;
#endif /* defined (UNEMAP_THREAD_SAFE) */

Unemap_calibration_end_callback *module_calibration_end_callback=
	(Unemap_calibration_end_callback *)NULL;
void *module_calibration_end_callback_data=NULL;
int *module_calibration_channels=(int *)NULL,
	module_calibration_number_of_channels=0;
float *module_calibration_gains=(float *)NULL,
	*module_calibration_offsets=(float *)NULL;

typedef int (Buffer_full_callback)(void *);
Buffer_full_callback *module_buffer_full_callback=(Buffer_full_callback *)NULL;
unsigned long module_buffer_full_size=0;
void *module_buffer_full_callback_data=NULL;

#if defined (NI_DAQ)
/* hardware information */
	/*???DB.  NI_CARDS->ni_cards, once made sure no other ni_cards */
float module_sampling_frequency=(float)0;
int module_configured=0,*module_configured_channels=(int *)NULL,
	module_number_of_configured_channels=0,module_number_of_NI_CARDS=0,
	module_number_of_stimulators=0,*module_stimulator_NI_CARD_indices=(int *)NULL;
struct NI_card *module_NI_CARDS=(struct NI_card *)NULL;
unsigned long module_sample_buffer_size=0,module_starting_sample_number;
u32 module_sampling_high_count=0,module_sampling_low_count=0;
#endif /* defined (NI_DAQ) */
int module_slave=0;

#if defined (SYNCHRONOUS_STIMULATION)
#if defined (NI_DAQ)
/* for allowing stimulation on different cards to start at the same time */
f64 start_stimulation_da_frequency;
i16 start_stimulation_da_channel;
u32 start_stimulation_iterations;
struct NI_card *start_stimulation_card=(struct NI_card *)NULL;
#endif /* defined (NI_DAQ) */
#endif /* defined (SYNCHRONOUS_STIMULATION) */

/*
Module functions
----------------
*/
static int lock_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE mutex
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *mutex
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

DESCRIPTION :
Locks the <mutex>.
==============================================================================*/
{
	int return_code;

	ENTER(lock_mutex);
	return_code=1;
#if defined (UNEMAP_THREAD_SAFE)
	if (mutex)
	{
#if defined (WIN32_SYSTEM)
		if (WAIT_FAILED==WaitForSingleObject(mutex,INFINITE))
		{
			display_message(ERROR_MESSAGE,
				"lock_mutex.  WaitForSingleObject failed.  Error code %d",
				GetLastError());
			return_code=0;
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_lock(mutex);
#endif /* defined (UNIX) */
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;

	return (return_code);
} /* lock_mutex */

static int try_lock_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE mutex
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *mutex
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 20 May 2002

DESCRIPTION :
Trys to lock the <mutex>.
==============================================================================*/
{
	int return_code;

	ENTER(try_lock_mutex);
	return_code=0;
#if defined (UNEMAP_THREAD_SAFE)
	if (mutex)
	{
#if defined (WIN32_SYSTEM)
		if (WAIT_OBJECT_0==WaitForSingleObject(mutex,(DWORD)0))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		/*???DB.  To be done */
		pthread_mutex_trylock(mutex);
#endif /* defined (UNIX) */
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;

	return (return_code);
} /* try_lock_mutex */

static int unlock_mutex(
#if defined (WIN32_SYSTEM)
	HANDLE mutex
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
	pthread_mutex_t *mutex
#endif /* defined (UNIX) */
	)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

DESCRIPTION :
Unlocks the <mutex>.
==============================================================================*/
{
	int return_code;

	ENTER(unlock_mutex);
	return_code=1;
#if defined (UNEMAP_THREAD_SAFE)
	if (mutex)
	{
#if defined (WIN32_SYSTEM)
		ReleaseMutex(mutex);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
		pthread_mutex_unlock(mutex);
#endif /* defined (UNIX) */
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;

	return (return_code);
} /* unlock_mutex */

static FILE *fopen_UNEMAP_HARDWARE(char *file_name,char *type)
/*******************************************************************************
LAST MODIFIED : 11 January 1999

DESCRIPTION :
Opens a file in the UNEMAP_HARDWARE directory.
==============================================================================*/
{
	char *hardware_directory,*hardware_file_name;
	FILE *hardware_file;

	ENTER(fopen_UNEMAP_HARDWARE);
	hardware_file=(FILE *)NULL;
	if (file_name&&type)
	{
		hardware_file_name=(char *)NULL;
		if (hardware_directory=getenv("UNEMAP_HARDWARE"))
		{
			if (ALLOCATE(hardware_file_name,char,strlen(hardware_directory)+
				strlen(file_name)+2))
			{
				strcpy(hardware_file_name,hardware_directory);
#if defined (WIN32_SYSTEM)
				if ('\\'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"\\");
				}
#else /* defined (WIN32_SYSTEM) */
				if ('/'!=hardware_file_name[strlen(hardware_file_name)-1])
				{
					strcat(hardware_file_name,"/");
				}
#endif /* defined (WIN32_SYSTEM) */
			}
		}
		else
		{
			if (ALLOCATE(hardware_file_name,char,strlen(file_name)+1))
			{
				hardware_file_name[0]='\0';
			}
		}
		if (hardware_file_name)
		{
			strcat(hardware_file_name,file_name);
			hardware_file=fopen(hardware_file_name,type);
			DEALLOCATE(hardware_file_name);
		}
	}
	LEAVE;

	return (hardware_file);
} /* fopen_UNEMAP_HARDWARE */

#if defined (NI_DAQ)
static int set_shift_register(struct NI_card *ni_card,int register_number,
	int high,int apply)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
If <high> is not zero then shift <register_number> of the <ni_card> is set high,
other wise the register is set low.  If <apply> is non-zero then this change is
downloaded to the physical device otherwise the change is cached - this allows
several changes to be applied at once.
==============================================================================*/
{
	int bit_number,byte_number,return_code;
	i16 status;
	unsigned char bit_mask;
	u32 counter,counter_start;

	ENTER(set_shift_register);
	return_code=0;
	/* check arguments */
	if (ni_card&&
		((UnEmap_2V1==ni_card->unemap_hardware_version)||
		(UnEmap_2V2==ni_card->unemap_hardware_version))&&
		(0<register_number)&&(register_number<=80))
	{
		byte_number=(register_number-1)/8;
		bit_mask=(unsigned char)0x1;
		bit_mask <<= (register_number-1)%8;
		if (high)
		{
			((ni_card->unemap_2vx).shift_register)[byte_number] |= bit_mask;
		}
		else
		{
			((ni_card->unemap_2vx).shift_register)[byte_number] &= ~bit_mask;
		}
		if (apply)
		{
			/* use timer to make sure that don't go too quickly */
			/* make sure that RCLK and Rstrobe are low */
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_clock_output_line_UnEmap2vx,(i16)0);
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)0);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
			/* clock in the data */
			for (byte_number=9;byte_number>=0;byte_number--)
			{
				bit_mask=(unsigned char)0x80;
				for (bit_number=7;bit_number>=0;bit_number--)
				{
					/* set the data line */
					if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							shift_register_data_output_line_UnEmap2vx,(i16)1);
					}
					else
					{
						DIG_Out_Line(ni_card->device_number,(i16)0,
							shift_register_data_output_line_UnEmap2vx,(i16)0);
					}
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set the clock line high */
					DIG_Out_Line(ni_card->device_number,(i16)0,
						shift_register_clock_output_line_UnEmap2vx,(i16)1);
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set the clock line low */
					DIG_Out_Line(ni_card->device_number,(i16)0,
						shift_register_clock_output_line_UnEmap2vx,(i16)0);
					/* pause */
					status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
						ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
							ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					bit_mask >>= 1;
				}
			}
			/* apply strobe */
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)1);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
			DIG_Out_Line(ni_card->device_number,(i16)0,
				shift_register_strobe_output_line_UnEmap2vx,(i16)0);
			/* pause */
			status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
				ND_COUNT,&counter_start);
			do
			{
				status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
					ND_COUNT,&counter);
				if (counter<counter_start)
				{
					counter_start=0;
				}
			} while ((0==status)&&(counter-counter_start<(u32)40));
		}
		return_code=1;
	}
	else
	{
		if (ni_card)
		{
			display_message(ERROR_MESSAGE,
				"set_shift_register.  Invalid argument(s).  %p %d %d (%d or %d)",
				ni_card,register_number,ni_card->unemap_hardware_version,UnEmap_2V1,
				UnEmap_2V2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_shift_register.  Invalid argument(s).  %p %d",ni_card,
				register_number);
		}
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_shift_register */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
#define READ_WORD( file , word ) \
fscanf(file," %80[^=]",word); \
i=strlen(word); \
while ((i>0)&&(isspace(word[i-1]))) \
{ \
	i--; \
} \
word[i]='\0';

static int search_for_NI_cards(void)
/*******************************************************************************
LAST MODIFIED : 4 August 2003

DESCRIPTION :
==============================================================================*/
{
	char word[81];
	int unemap_hardware_version;
	FILE *digital_io_lines;
	float gain;
	int i,j,return_code,*stimulator_card_indices;
	i16 card_code,card_number,GA0_setting,GA1_setting,input_mode,polarity,status;
	struct NI_card *ni_card;
	unsigned char shift_registers[10];
	unsigned short shift_register;
	u16 sampling_interval;

	ENTER(search_for_NI_cards);
	return_code=1;
	if (!module_NI_CARDS)
	{
		if (digital_io_lines=fopen_UNEMAP_HARDWARE("digital.txt","r"))
		{
			module_NI_CARDS=(struct NI_card *)NULL;
			module_number_of_NI_CARDS=0;
			module_number_of_stimulators=0;
			module_stimulator_NI_CARD_indices=(int *)NULL;
			card_number=1;
			word[0]='\0';
			/* determine the hardware version */
			return_code=0;
			if (1==fscanf(digital_io_lines," cs = %hd ",
				&chip_select_line_UnEmap1vx))
			{
				unemap_hardware_version=UnEmap_1V2;
				READ_WORD(digital_io_lines,word);
				return_code=1;
			}
			else
			{
				if (1==fscanf(digital_io_lines," BattGood = %hd ",
					&battery_good_input_line_UnEmap2vx))
				{
					unemap_hardware_version=UnEmap_2V1;
					READ_WORD(digital_io_lines,word);
					return_code=1;
				}
				else
				{
					if (1==fscanf(digital_io_lines," version = %10s ",word))
					{
						if (!strcmp("UnEmap_1vx",word))
						{
							unemap_hardware_version=UnEmap_1V2;
							chip_select_line_UnEmap1vx=0;
							READ_WORD(digital_io_lines,word);
							if (!strcmp("cs",word))
							{
								fscanf(digital_io_lines," = %hd ",&chip_select_line_UnEmap1vx);
								READ_WORD(digital_io_lines,word);
							}
						}
						else
						{
							if (!strcmp(word,"UnEmap_2v1"))
							{
								unemap_hardware_version=UnEmap_2V1;
								return_code=1;
							}
							else
							{
								if (!strcmp(word,"UnEmap_2v2"))
								{
									unemap_hardware_version=UnEmap_2V2;
									return_code=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Unknown version, %s, from digital.txt",word);
									return_code=0;
								}
							}
							if (return_code)
							{
								battery_good_input_line_UnEmap2vx=0;
								READ_WORD(digital_io_lines,word);
								if (!strcmp("BattGood",word))
								{
									fscanf(digital_io_lines," = %hd ",
										&battery_good_input_line_UnEmap2vx);
									READ_WORD(digital_io_lines,word);
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Error reading version from digital.txt");
						return_code=0;
					}
				}
			}
			if (return_code)
			{
				initial_antialiasing_filter_frequency=(float)200;
				input_mode=1;
				polarity=0;
				gain=(float)1;
				sampling_interval=0;
				switch (unemap_hardware_version)
				{
					case UnEmap_1V2:
					{
						/* NB.  chip_select_line_UnEmap1vx default is set above (before it
							is read in */
						filter_increment_output_line=4;
						up_down_line_UnEmap1vx=5;
						led_line_UnEmap1vx=2;
						id_line_1_UnEmap1vx=6;
						id_line_2_UnEmap1vx=3;
						id_line_3_UnEmap1vx=7;
#if defined (OLD_CODE)
						/* for UnEmap_1V1 (no working systems) */
						filter_increment_output_line=5;
						up_down_line_UnEmap1vx=4;
						led_line_UnEmap1vx=2;
						id_line_1_UnEmap1vx=6;
						id_line_2_UnEmap1vx=3;
						id_line_3_UnEmap1vx=7;
#endif /* defined (OLD_CODE) */
						if (0==strcmp("u/d",word))
						{
							fscanf(digital_io_lines," = %hd ",&up_down_line_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("inc",word))
						{
							fscanf(digital_io_lines," = %hd ",&filter_increment_output_line);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("led",word))
						{
							fscanf(digital_io_lines," = %hd ",&led_line_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id1",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_1_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id2",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_2_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("id3",word))
						{
							fscanf(digital_io_lines," = %hd ",&id_line_3_UnEmap1vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("filter",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&initial_antialiasing_filter_frequency);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("gain",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("input_mode",word))
						{
							fscanf(digital_io_lines," = %hd ",&input_mode);
							if (input_mode< -1)
							{
								input_mode= -1;
							}
							else
							{
								if (input_mode>2)
								{
									input_mode=2;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("polarity",word))
						{
							fscanf(digital_io_lines," = %hd ",&polarity);
							if (polarity<0)
							{
								polarity=0;
							}
							else
							{
								if (polarity>1)
								{
									polarity=1;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						/*???debug */
						{
							FILE *unemap_debug;

							if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
							{
								fprintf(unemap_debug,"version=UnEmap_1vx\n");
								fprintf(unemap_debug,"chip_select_line=%d\n",
									chip_select_line_UnEmap1vx);
								fprintf(unemap_debug,"up_down_line=%d\n",
									up_down_line_UnEmap1vx);
								fprintf(unemap_debug,"increment_line=%d\n",
									filter_increment_output_line);
								fprintf(unemap_debug,"led_line=%d\n",led_line_UnEmap1vx);
								fprintf(unemap_debug,"id_line_1=%d\n",id_line_1_UnEmap1vx);
								fprintf(unemap_debug,"id_line_2=%d\n",id_line_2_UnEmap1vx);
								fprintf(unemap_debug,"id_line_3=%d\n",id_line_3_UnEmap1vx);
								fprintf(unemap_debug,"antialiasing_filter_frequency=%g\n",
									initial_antialiasing_filter_frequency);
								fprintf(unemap_debug,"gain=%g\n",gain);
								fprintf(unemap_debug,"input_mode=");
								switch (input_mode)
								{
									case -1:
									{
										fprintf(unemap_debug,"none\n");
									} break;
									case 0:
									{
										fprintf(unemap_debug,"differential\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"referenced single-ended\n");
									} break;
									case 2:
									{
										fprintf(unemap_debug,"non-referenced single-ended\n");
									} break;
								}
								fprintf(unemap_debug,"polarity=");
								switch (polarity)
								{
									case 0:
									{
										fprintf(unemap_debug,"bipolar\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"unipolar\n");
									} break;
								}
								fclose(unemap_debug);
							}
						}
#if defined (DEBUG)
#endif /* defined (DEBUG) */
					} break;
					case UnEmap_2V1:
					case UnEmap_2V2:
					{
						filter_increment_output_line=5;
						switch (unemap_hardware_version)
						{
							case UnEmap_2V1:
							{
								battery_A_line_UnEmap2v1=2;
							} break;
							case UnEmap_2V2:
							{
								master_line_UnEmap2v2=2;
							} break;
						}
						gain_0_output_line_UnEmap2vx=7;
						gain_1_output_line_UnEmap2vx=3;
						shift_register_clock_output_line_UnEmap2vx=6;
						shift_register_data_output_line_UnEmap2vx=4;
						shift_register_strobe_output_line_UnEmap2vx=1;
						BattA_setting_UnEmap2vx=0;
						GA0_setting=0;
						GA1_setting=0;
						shift_registers[0]=(unsigned char)0x79;
						shift_registers[1]=(unsigned char)0x0;
						for (i=2;i<10;i++)
						{
							shift_registers[i]=(unsigned char)0xff;
						}
						tol_settling=(float)1.;
						settling_step_max=20;
						if (0==strcmp("Rstrobe",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_strobe_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA1",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain_1_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("Rdata",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_data_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("FINC",word))
						{
							fscanf(digital_io_lines," = %hd ",&filter_increment_output_line);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("RCLK",word))
						{
							fscanf(digital_io_lines," = %hd ",
								&shift_register_clock_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA0",word))
						{
							fscanf(digital_io_lines," = %hd ",&gain_0_output_line_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("filter",word))
						{
							fscanf(digital_io_lines," = %f ",
								&initial_antialiasing_filter_frequency);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("gain",word))
						{
							fscanf(digital_io_lines," = %f ",&gain);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("input_mode",word))
						{
							fscanf(digital_io_lines," = %hd ",&input_mode);
							if (input_mode< -1)
							{
								input_mode= -1;
							}
							else
							{
								if (input_mode>2)
								{
									input_mode=2;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("polarity",word))
						{
							fscanf(digital_io_lines," = %hd ",&polarity);
							if (polarity<0)
							{
								polarity=0;
							}
							else
							{
								if (polarity>1)
								{
									polarity=1;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA0 setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&GA0_setting);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("GA1 setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&GA1_setting);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("shift registers",word))
						{
							fscanf(digital_io_lines," = ");
							j=0;
							while ((j<10)&&
								(1==fscanf(digital_io_lines,"%2hx",&shift_register)))
							{
								shift_registers[j]=(unsigned char)shift_register;
								j++;
							}
							if (j<10)
							{
								display_message(ERROR_MESSAGE,
									"search_for_NI_cards.  Could not read shift registers");
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("settling magnitude",word))
						{
							fscanf(digital_io_lines," = %f ",&tol_settling);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("relay power on",word))
						{
							fscanf(digital_io_lines," = %d ",&relay_power_on_UnEmap2vx);
							if (relay_power_on_UnEmap2vx)
							{
								shift_registers[0] &= (unsigned char)0xe7;
							}
							else
							{
								shift_registers[0] |= (unsigned char)0x18;
							}
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("number of valid ni channels",word))
						{
							fscanf(digital_io_lines," = %d ",&number_of_valid_ni_channels);
							if (number_of_valid_ni_channels<1)
							{
								number_of_valid_ni_channels=1;
							}
							else
							{
								if (number_of_valid_ni_channels>NUMBER_OF_CHANNELS_ON_NI_CARD)
								{
									number_of_valid_ni_channels=NUMBER_OF_CHANNELS_ON_NI_CARD;
								}
							}
							READ_WORD(digital_io_lines,word);
						}
						switch (unemap_hardware_version)
						{
							case UnEmap_2V1:
							{
								if (0==strcmp("BattA",word))
								{
									fscanf(digital_io_lines," = %hd ",&battery_A_line_UnEmap2v1);
									READ_WORD(digital_io_lines,word);
								}
							} break;
							case UnEmap_2V2:
							{
								if (0==strcmp("Master",word))
								{
									fscanf(digital_io_lines," = %hd ",&master_line_UnEmap2v2);
									READ_WORD(digital_io_lines,word);
								}
							} break;
						}
						if (0==strcmp("BattA setting",word))
						{
							fscanf(digital_io_lines," = %hd ",&BattA_setting_UnEmap2vx);
							READ_WORD(digital_io_lines,word);
						}
						if (0==strcmp("sampling interval",word))
						{
							fscanf(digital_io_lines," = %hd ",&sampling_interval);
							READ_WORD(digital_io_lines,word);
						}
						/* disable setting sampling_interval to prevent confusion when
							mixing 12 and 16 bit cards */
						sampling_interval=0;
						if (0==strcmp("max settling steps",word))
						{
							fscanf(digital_io_lines," = %d ",&settling_step_max);
							READ_WORD(digital_io_lines,word);
						}
						/*???debug */
						{
							FILE *unemap_debug;

							if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
							{
								switch (unemap_hardware_version)
								{
									case UnEmap_2V1:
									{
										fprintf(unemap_debug,"version=UnEmap_2v1\n");
									} break;
									case UnEmap_2V2:
									{
										fprintf(unemap_debug,"version=UnEmap_2v2\n");
									} break;
								}
								fprintf(unemap_debug,"battery_good_input_line=%d\n",
									battery_good_input_line_UnEmap2vx);
								fprintf(unemap_debug,"shift_register_strobe_output_line=%d\n",
									shift_register_strobe_output_line_UnEmap2vx);
								fprintf(unemap_debug,"gain_1_output_line=%d\n",
									gain_1_output_line_UnEmap2vx);
								fprintf(unemap_debug,"shift_register_data_output_line=%d\n",
									shift_register_data_output_line_UnEmap2vx);
								fprintf(unemap_debug,"filter_increment_output_line=%d\n",
									filter_increment_output_line);
								fprintf(unemap_debug,"shift_register_clock_output_line=%d\n",
									shift_register_clock_output_line_UnEmap2vx);
								fprintf(unemap_debug,"gain_0_output_line=%d\n",
									gain_0_output_line_UnEmap2vx);
								fprintf(unemap_debug,"antialiasing_filter_frequency=%g\n",
									initial_antialiasing_filter_frequency);
								fprintf(unemap_debug,"gain=%g\n",gain);
								fprintf(unemap_debug,"input_mode=");
								switch (input_mode)
								{
									case -1:
									{
										fprintf(unemap_debug,"none\n");
									} break;
									case 0:
									{
										fprintf(unemap_debug,"differential\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"referenced single-ended\n");
									} break;
									case 2:
									{
										fprintf(unemap_debug,"non-referenced single-ended\n");
									} break;
								}
								fprintf(unemap_debug,"polarity=");
								switch (polarity)
								{
									case 0:
									{
										fprintf(unemap_debug,"bipolar\n");
									} break;
									case 1:
									{
										fprintf(unemap_debug,"unipolar\n");
									} break;
								}
								fprintf(unemap_debug,"GA0 setting=");
								if (GA0_setting)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"GA1 setting=");
								if (GA1_setting)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"shift registers=");
								for (j=0;j<10;j++)
								{
									fprintf(unemap_debug,"%02x",
										(unsigned short)shift_registers[j]);
								}
								fprintf(unemap_debug,"\n");
								fprintf(unemap_debug,"settling_magnitude=%g\n",tol_settling);
								fprintf(unemap_debug,"relay_power_on=%d\n",
									relay_power_on_UnEmap2vx);
								fprintf(unemap_debug,"number_of_valid_ni_channels=%d\n",
									number_of_valid_ni_channels);
								switch (unemap_hardware_version)
								{
									case UnEmap_2V1:
									{
										fprintf(unemap_debug,"battery_A_line=%d\n",
											battery_A_line_UnEmap2v1);
									} break;
									case UnEmap_2V2:
									{
										fprintf(unemap_debug,"master_line=%d\n",
											master_line_UnEmap2v2);
									} break;
								}
								fprintf(unemap_debug,"BattA setting=");
								if (BattA_setting_UnEmap2vx)
								{
									fprintf(unemap_debug,"on\n");
								}
								else
								{
									fprintf(unemap_debug,"off\n");
								}
								fprintf(unemap_debug,"sampling interval=%d\n",
									sampling_interval);
								fprintf(unemap_debug,"max settling steps=%d\n",
									settling_step_max);
								fclose(unemap_debug);
							}
						}
#if defined (DEBUG)
#endif /* defined (DEBUG) */
					} break;
					default:
					{
						return_code=0;
					} break;
				}
				if (return_code)
				{
					/* gain should be 1,2,5,10,20,50 or 100 */
					if (gain<2)
					{
						gain=(float)1;
					}
					else
					{
						if (gain<4)
						{
							gain=(float)2;
						}
						else
						{
							if (gain<8)
							{
								gain=(float)5;
							}
							else
							{
								if (gain<15)
								{
									gain=(float)10;
								}
								else
								{
									if (gain<35)
									{
										gain=(float)20;
									}
									else
									{
										if (gain<75)
										{
											gain=(float)50;
										}
										else
										{
											gain=(float)100;
										}
									}
								}
							}
						}
					}
					while (return_code&&(0==Init_DA_Brds(card_number,&card_code)))
					{
						if ((PCI6031E_DEVICE_CODE==card_code)||
							(PCI6033E_DEVICE_CODE==card_code)||
							(PXI6031E_DEVICE_CODE==card_code)||
							(PXI6071E_DEVICE_CODE==card_code))
						{
#if defined (OLD_CODE)
							/* make sure that not mixing 12-bit and 16-bit */
							if ((0==module_number_of_NI_CARDS)||
								((PXI6071E_DEVICE_CODE==card_code)&&
								(PXI6071E_AD_DA==(module_NI_CARDS[0]).type))||
								((PXI6071E_DEVICE_CODE!=card_code)&&
								(PXI6071E_AD_DA!=(module_NI_CARDS[0]).type)))
							{
#endif /* defined (OLD_CODE) */
								if (REALLOCATE(ni_card,module_NI_CARDS,struct NI_card,
									module_number_of_NI_CARDS+1))
								{
									module_NI_CARDS=ni_card;
									ni_card += module_number_of_NI_CARDS;
									ni_card->device_number=card_number;
									ni_card->number_of_configured_channels=0;
									ni_card->unemap_hardware_version=unemap_hardware_version;
									switch (card_code)
									{
										case PCI6031E_DEVICE_CODE:
										{
											ni_card->type=PCI6031E_AD_DA;
										} break;
										case PCI6033E_DEVICE_CODE:
										{
											ni_card->type=PCI6033E_AD;
										} break;
										case PXI6031E_DEVICE_CODE:
										{
											ni_card->type=PXI6031E_AD_DA;
										} break;
										case PXI6071E_DEVICE_CODE:
										{
											ni_card->type=PXI6071E_AD_DA;
										} break;
									}
									if ((PCI6031E_AD_DA==ni_card->type)||
										(PXI6031E_AD_DA==ni_card->type)||
										(PXI6071E_AD_DA==ni_card->type))
									{
										if (REALLOCATE(stimulator_card_indices,
											module_stimulator_NI_CARD_indices,int,
											module_number_of_stimulators+1))
										{
											module_stimulator_NI_CARD_indices=stimulator_card_indices;
											module_stimulator_NI_CARD_indices[
												module_number_of_stimulators]=module_number_of_NI_CARDS;
											module_number_of_stimulators++;
										}
									}
									ni_card->memory_object=(HGLOBAL)NULL;
									ni_card->hardware_buffer=(i16 *)NULL;
									ni_card->da_buffer=(i16 *)NULL;
									ni_card->da_buffer_size=(u32)0;
									ni_card->time_base=0;
									ni_card->sampling_interval=sampling_interval;
									ni_card->hardware_buffer_size=0;
									ni_card->input_mode=0;
									ni_card->polarity=0;
									if (((PCI6031E_AD_DA==ni_card->type)||
										(PCI6033E_AD==ni_card->type)||
										(PXI6031E_AD_DA==ni_card->type))&&(gain<(float)2))
									{
										/* lowest gain not used for 16 bit */
										gain=(float)2;
									}
									ni_card->gain=(i16)gain;
									/* initially the filter setting is undefined */
									ni_card->anti_aliasing_filter_taps= -1;
									ni_card->input_mode=input_mode;
									ni_card->polarity=polarity;
									ni_card->stimulation_end_callback_id=0;
									ni_card->stimulation_end_callback=
										(Unemap_stimulation_end_callback *)NULL;
									ni_card->stimulation_end_callback_data=(void *)NULL;
									if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
										(UnEmap_2V2==ni_card->unemap_hardware_version))
									{
										(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x0;
										(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x0;
										(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x0;
										for (i=9;i>=0;i--)
										{
											((ni_card->unemap_2vx).shift_register)[i]=
												(unsigned char)0x0;
										}
										for (j=0;j<10;j++)
										{
											(ni_card->unemap_2vx).shift_register[j]=
												shift_registers[j];
										}
										for (j=0;j<8;j++)
										{
											(ni_card->unemap_2vx).stimulate_channels[j]=
												shift_registers[j+2];
										}
									}
									/* to make the pinout for UnEmap_1V2 the same as for later
										versions */
									if (UnEmap_1V2==ni_card->unemap_hardware_version)
									{
										for (i=0;i<16;i++)
										{
											(ni_card->channel_reorder)[i]=55-i;
										}
										for (i=16;i<32;i++)
										{
											(ni_card->channel_reorder)[i]=31-(i/2)+(8*(i%2));
										}
										for (i=32;i<40;i++)
										{
											(ni_card->channel_reorder)[i]=95-i;
										}
										for (i=40;i<48;i++)
										{
											(ni_card->channel_reorder)[i]=79-i;
										}
										for (i=48;i<64;i++)
										{
											(ni_card->channel_reorder)[i]=39-(i/2)-(8*(i%2));
										}
									}
									else
									{
										for (i=NUMBER_OF_CHANNELS_ON_NI_CARD-1;i>=0;i--)
										{
											(ni_card->channel_reorder)[i]=i;
										}
									}
									if (0<=input_mode)
									{
										/* configure the card */
										status=AI_Clear(ni_card->device_number);
#if defined (USE_INTERRUPTS_FOR_AI)
										{
											FILE *use_interrupts;

											if (use_interrupts=fopen_UNEMAP_HARDWARE(
												"useint.txt","r"))
											{
												fclose(use_interrupts);
												/* use interrupts for transferring AI data */
												status=Set_DAQ_Device_Info(ni_card->device_number,
													ND_DATA_XFER_MODE_AI,ND_INTERRUPTS);
												if (0!=status)
												{
													display_message(ERROR_MESSAGE,"search_for_NI_cards.  "
														"Set_DAQ_Device_Info failed.  %d",status);
												}
											}
										}
#endif /* defined (USE_INTERRUPTS_FOR_AI) */
										/* analog input */
										status=AI_Configure(ni_card->device_number,
											/* all channels */(i16)(-1),input_mode,
											/* ignored (input range) */(i16)0,polarity,
											/* do not drive AISENSE to ground */(i16)0);
									}
									else
									{
										status=0;
									}
									if (0==status)
									{
										/* digital I/O */
										/* start counter used for digital I/O timing */
										GPCTR_Control(ni_card->device_number,DIGITAL_IO_COUNTER,
											ND_RESET);
										GPCTR_Set_Application(ni_card->device_number,
											DIGITAL_IO_COUNTER,ND_SIMPLE_EVENT_CNT);
										GPCTR_Change_Parameter(ni_card->device_number,
											DIGITAL_IO_COUNTER,ND_SOURCE,ND_INTERNAL_20_MHZ);
										GPCTR_Control(ni_card->device_number,DIGITAL_IO_COUNTER,
											ND_PROGRAM);
										/* configure digital I/O lines */
										switch (ni_card->unemap_hardware_version)
										{
											case UnEmap_1V2:
											{
												status=DIG_Line_Config(ni_card->device_number,0,
													chip_select_line_UnEmap1vx,(i16)1);
												if (0==status)
												{
													status=DIG_Line_Config(ni_card->device_number,0,
														up_down_line_UnEmap1vx,(i16)1);
													if (0==status)
													{
														status=DIG_Line_Config(ni_card->device_number,0,
															filter_increment_output_line,(i16)1);
														if (0==status)
														{
															status=DIG_Line_Config(ni_card->device_number,0,
																led_line_UnEmap1vx,(i16)1);
															if (0==status)
															{
																status=DIG_Line_Config(ni_card->device_number,0,
																	id_line_1_UnEmap1vx,(i16)0);
																if (0==status)
																{
																	status=DIG_Line_Config(ni_card->device_number,
																		0,id_line_2_UnEmap1vx,(i16)0);
																	if (0==status)
																	{
																		status=DIG_Line_Config(
																			ni_card->device_number,0,
																			id_line_3_UnEmap1vx,(i16)0);
																		if (0==status)
																		{
																			/* set cs high to disable programming */
																			status=DIG_Out_Line(
																				ni_card->device_number,0,
																				chip_select_line_UnEmap1vx,(i16)1);
																			if (0==status)
																			{
																				/* set the led on */
																				status=DIG_Out_Line(
																					ni_card->device_number,0,
																					led_line_UnEmap1vx,(i16)1);
																				if (0==status)
																				{
																					i16 state;

																					/* read the analog front-end card
																						id */
																					status=DIG_In_Line(
																						ni_card->device_number,0,
																						id_line_1_UnEmap1vx,&state);
																					if (0==status)
																					{
																						status=DIG_In_Line(
																							ni_card->device_number,0,
																							id_line_2_UnEmap1vx,&state);
																						if (0==status)
																						{
																							status=DIG_In_Line(
																								ni_card->device_number,0,
																								id_line_3_UnEmap1vx,&state);
																							if (0==status)
																							{
																							}
																							else
																							{
																								display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_3_UnEmap1vx");
																							}
																						}
																						else
																						{
																							display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_2_UnEmap1vx");
																						}
																					}
																					else
																					{
																						display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_In_Line failed for id_line_1_UnEmap1vx");
																					}
																				}
																				else
																				{
																					display_message(ERROR_MESSAGE,
						"search_for_NI_cards.  DIG_Out_Line failed for led_line_UnEmap1vx");
																				}
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
		"search_for_NI_cards.  DIG_Out_Line failed for chip_select_line_UnEmap1vx");
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_3_UnEmap1vx");
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_2_UnEmap1vx");
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for id_line_1_UnEmap1vx");
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
				"search_for_NI_cards.  DIG_Line_Config failed for led_line_UnEmap1vx");
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for filter_increment_output_line");
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
		"search_for_NI_cards.  DIG_Line_Config failed for up_down_line_UnEmap1vx");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for chip_select_line_UnEmap1vx");
												}
											} break;
											case UnEmap_2V1:
											case UnEmap_2V2:
											{
												status=DIG_Line_Config(ni_card->device_number,0,
													battery_good_input_line_UnEmap2vx,(i16)0);
												if (0==status)
												{
													status=DIG_Line_Config(ni_card->device_number,0,
														filter_increment_output_line,(i16)1);
													if (0==status)
													{
														status=DIG_Line_Config(ni_card->device_number,0,
															gain_0_output_line_UnEmap2vx,(i16)1);
														if (0==status)
														{
															status=DIG_Line_Config(ni_card->device_number,0,
																gain_1_output_line_UnEmap2vx,(i16)1);
															if (0==status)
															{
																status=DIG_Line_Config(ni_card->device_number,0,
																	shift_register_clock_output_line_UnEmap2vx,
																	(i16)1);
																if (0==status)
																{
																	status=DIG_Line_Config(ni_card->device_number,
																		0,
																		shift_register_data_output_line_UnEmap2vx,
																		(i16)1);
																	if (0==status)
																	{
																		status=DIG_Line_Config(
																			ni_card->device_number,0,
																			shift_register_strobe_output_line_UnEmap2vx,
																			(i16)1);
																		if (0==status)
																		{
																			if (0==module_number_of_NI_CARDS)
																			{
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							battery_A_line_UnEmap2v1,(i16)1);
																						if (0==status)
																						{
																							if (BattA_setting_UnEmap2vx)
																							{
																								DIG_Out_Line(
																									ni_card->device_number,
																									(i16)0,
																									battery_A_line_UnEmap2v1,
																									(i16)1);
																							}
																							else
																							{
																								DIG_Out_Line(
																									ni_card->device_number,
																									(i16)0,
																									battery_A_line_UnEmap2v1,
																									(i16)0);
																							}
																						}
																						/* set master high */
																						set_shift_register(ni_card,
																							Master_SHIFT_REGISTER_UnEmap2v1,1,
																							0);
																					} break;
																					case UnEmap_2V2:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							master_line_UnEmap2v2,(i16)1);
																						if (0==status)
																						{
																							/* set master low (only set high
																								when everything is
																								configured) */
																							DIG_Out_Line(
																								ni_card->device_number,
																								(i16)0,master_line_UnEmap2v2,
																								(i16)0);
																							if (BattA_setting_UnEmap2vx)
																							{
																								set_shift_register(ni_card,
																									BattA_SHIFT_REGISTER_UnEmap2vx,
																									1,0);
																							}
																							else
																							{
																								set_shift_register(ni_card,
																									BattA_SHIFT_REGISTER_UnEmap2vx,
																									0,0);
																							}
																						}
																					} break;
																				}
																			}
																			else
																			{
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							battery_A_line_UnEmap2v1,(i16)0);
																						/* set master low */
																						set_shift_register(ni_card,
																							Master_SHIFT_REGISTER_UnEmap2v1,0,
																							0);
																					} break;
																					case UnEmap_2V2:
																					{
																						status=DIG_Line_Config(
																							ni_card->device_number,0,
																							master_line_UnEmap2v2,(i16)0);
																					} break;
																				}
																			}
																			if (0==status)
																			{
																				/* set cs high to disable filter
																					programming */
																				set_shift_register(ni_card,
																					FCS_SHIFT_REGISTER_UnEmap2vx,1,0);
																				/* set the led */
																				switch (ni_card->
																					unemap_hardware_version)
																				{
																					case UnEmap_2V1:
																					{
																						if (BattA_setting_UnEmap2vx)
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								1,1);
																						}
																						else
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								0,1);
																						}
																					} break;
																					case UnEmap_2V2:
																					{
																						if (BattA_setting_UnEmap2vx)
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								0,1);
																						}
																						else
																						{
																							set_shift_register(ni_card,
																								PC_LED_SHIFT_REGISTER_UnEmap2vx,
																								1,1);
																						}
																					} break;
																				}
																				/* set the gain on the signal
																					conditioning card */
																				if (GA0_setting)
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_0=
																						(unsigned char)0x1;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,
																						gain_0_output_line_UnEmap2vx,
																						(i16)1);
																				}
																				else
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_0=
																						(unsigned char)0x0;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_0_output_line_UnEmap2vx,
																						(i16)0);
																				}
																				if (GA1_setting)
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_1=
																						(unsigned char)0x1;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_1_output_line_UnEmap2vx,
																						(i16)1);
																				}
																				else
																				{
																					(ni_card->unemap_2vx).
																						gain_output_line_1=
																						(unsigned char)0x0;
																					DIG_Out_Line(ni_card->device_number,
																						(i16)0,gain_1_output_line_UnEmap2vx,
																						(i16)0);
																				}
																				/* make sure that the card is in isolate
																					mode */
																				(ni_card->unemap_2vx).isolate_mode=
																					(unsigned char)0x0;
																			}
																			else
																			{
																				display_message(ERROR_MESSAGE,
	"search_for_NI_cards.  DIG_Line_Config failed for battery_A_line_UnEmap2v1");
																			}
																		}
																		else
																		{
																			display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_strobe_output_line_UnEmap2vx");
																		}
																	}
																	else
																	{
																		display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_data_output_line_UnEmap2vx");
																	}
																}
																else
																{
																	display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for shift_register_clock_output_line_UnEmap2vx");
																}
															}
															else
															{
																display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for gain_1_output_line_UnEmap2vx");
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for gain_0_output_line_UnEmap2vx");
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for filter_increment_output_line");
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
"search_for_NI_cards.  DIG_Line_Config failed for battery_good_input_line_UnEmap2vx");
												}
											} break;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"search_for_NI_cards.  AI_Configure failed.  %d",status);
									}
									/*???DB.  Moved from unemap_configure end */
									module_number_of_NI_CARDS++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"search_for_NI_cards.  Could not allocate ni_cards");
									module_number_of_NI_CARDS=0;
									DEALLOCATE(module_NI_CARDS);
									return_code=0;
								}
#if defined (OLD_CODE)
							}
							else
							{
								if (0==PXI6071E_DEVICE_CODE)
								{
									display_message(WARNING_MESSAGE,
	"Cannot mix 12-bit and 16-bit cards.  12-bit card, device number %d, ignored",
										card_number);
								}
								else
								{
									display_message(WARNING_MESSAGE,
	"Cannot mix 12-bit and 16-bit cards.  16-bit card, device number %d, ignored",
										card_number);
								}
							}
#endif /* defined (OLD_CODE) */
						}
						card_number++;
					}
					if (!(module_NI_CARDS&&(module_number_of_NI_CARDS>0)))
					{
						return_code=0;
					}
					if (return_code)
					{
						if (UnEmap_2V2==module_NI_CARDS->unemap_hardware_version)
						{
							/* set master high */
							DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
								master_line_UnEmap2v2,(i16)1);
						}
					}
				}
			}
			fclose(digital_io_lines);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open digital.txt\n");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* search_for_NI_cards */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static void scrolling_callback_NI(HWND handle,UINT message,WPARAM wParam,
	LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 7 August 2002

DESCRIPTION :
Always called so that <module_starting_sample_number> and
<module_sample_buffer_size> can be kept up to date.
==============================================================================*/
{
#if !defined (NO_SCROLLING_BODY)
#if !defined (SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY)
	int arm_module_buffer_full_callback,averaging_length,channel_number,
		*channel_numbers,*channel_numbers_2,i,j,k,number_of_bytes;
	i16 *hardware_buffer;
	long sum;
	SHORT *value,*value_array,*value_array_2;
	static int first_error=1;
	struct NI_card *ni_card;
	unsigned char *byte_array;
	unsigned long end1,end2,end3,offset,number_of_configured_channels,
		number_of_samples,sample_number;
#else /* !defined (SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY) */
	int arm_module_buffer_full_callback;
	unsigned long end1,end2,end3,number_of_samples,sample_number;
#endif /* !defined (SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY) */

	ENTER(scrolling_callback_NI);
#if defined (UNEMAP_THREAD_SAFE)
	if (try_lock_mutex(scrolling_callback_mutex))
	{
#endif /* defined (UNEMAP_THREAD_SAFE) */
	/* callback may come after the sampling has stopped or stopped and started
		again, in which case <module_starting_sample_number> and
		<module_sample_buffer_size> will have already been updated */
	sample_number=(unsigned long)lParam;
	if (0<module_NI_CARDS->number_of_configured_channels)
	{
		number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
			(module_NI_CARDS->number_of_configured_channels);
	}
	else
	{
		number_of_samples=module_NI_CARDS->hardware_buffer_size;
	}
	end1=(module_starting_sample_number+module_sample_buffer_size)%
		number_of_samples;
	end2=end1+module_scrolling_callback_period;
	end3=end2%number_of_samples;
	if (module_sampling_on&&(((end1<=sample_number)&&(sample_number<=end2))||
		((sample_number<=end3)&&(end3<end1))))
	{
		/* keep <module_starting_sample_number> and <module_sample_buffer_size> up
			to date */
		/* NIDAQ returns the number of the next sample */
		if (module_sample_buffer_size<number_of_samples)
		{
			if (module_sample_buffer_size<module_buffer_full_size)
			{
				arm_module_buffer_full_callback=1;
			}
			else
			{
				arm_module_buffer_full_callback=0;
			}
			if (sample_number<module_starting_sample_number+module_sample_buffer_size)
			{
				module_sample_buffer_size += (sample_number+number_of_samples)-
					(module_starting_sample_number+module_sample_buffer_size);
			}
			else
			{
				module_sample_buffer_size += sample_number-
					(module_starting_sample_number+module_sample_buffer_size);
			}
			if (module_sample_buffer_size>=number_of_samples)
			{
				module_sample_buffer_size=number_of_samples;
				module_starting_sample_number=sample_number%number_of_samples;
			}
			if (module_buffer_full_callback&&arm_module_buffer_full_callback&&
				((module_sample_buffer_size>=module_buffer_full_size)||
				(module_sample_buffer_size>=number_of_samples)))
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,
					"call module_buffer_full_callback %lu\n",module_sample_buffer_size);
#endif /* defined (DEBUG) */
				(*module_buffer_full_callback)(module_buffer_full_callback_data);
			}
		}
		else
		{
			module_starting_sample_number=sample_number%number_of_samples;
		}
#if !defined (SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY)
		sample_number += number_of_samples-1;
		sample_number %= number_of_samples;
		if (module_scrolling_on&&(0<module_number_of_scrolling_channels)&&
			(0<module_number_of_scrolling_values_per_channel)&&
			(module_scrolling_callback
#if defined (WIN32_USER_INTERFACE)
			||module_scrolling_window
#endif /* defined (WIN32_USER_INTERFACE) */
			))
		{
#if defined (WIN32_USER_INTERFACE)
			if (module_scrolling_window)
			{
				number_of_bytes=(module_number_of_scrolling_channels+2)*sizeof(int)+
					module_number_of_scrolling_channels*
					module_number_of_scrolling_values_per_channel*sizeof(SHORT)+
					sizeof(module_scrolling_callback_data);
				if (ALLOCATE(byte_array,unsigned char,number_of_bytes))
				{
					*((int *)byte_array)=module_number_of_scrolling_channels;
					channel_numbers=(int *)(byte_array+sizeof(int));
					*((int *)(byte_array+(module_number_of_scrolling_channels+1)*
						sizeof(int)))=module_number_of_scrolling_values_per_channel;
					value_array=(SHORT *)(byte_array+
						(module_number_of_scrolling_channels+2)*sizeof(int));
					*((void **)(byte_array+((module_number_of_scrolling_channels+2)*
						sizeof(int)+module_number_of_scrolling_channels*
						module_number_of_scrolling_values_per_channel*sizeof(SHORT))))=
						module_scrolling_callback_data;
				}
				else
				{
					if (first_error)
					{
						display_message(ERROR_MESSAGE,
							"scrolling_callback_NI.  Could not allocate byte_array");
						first_error=0;
					}
				}
			}
			else
			{
#endif /* defined (WIN32_USER_INTERFACE) */
				ALLOCATE(channel_numbers,int,module_number_of_scrolling_channels);
				ALLOCATE(value_array,SHORT,module_number_of_scrolling_channels*
					module_number_of_scrolling_values_per_channel);
				if (!(channel_numbers&&value_array))
				{
					DEALLOCATE(channel_numbers);
					DEALLOCATE(value_array);
					if (first_error)
					{
						display_message(ERROR_MESSAGE,
			"scrolling_callback_NI.  Could not allocate channel_numbers/value_array");
						first_error=0;
					}
				}
#if defined (WIN32_USER_INTERFACE)
			}
#endif /* defined (WIN32_USER_INTERFACE) */
			/* calculate the values */
			if (value=value_array)
			{
				for (k=0;k<module_number_of_scrolling_channels;k++)
				{
					channel_number=module_scrolling_channel_numbers[k];
					channel_numbers[k]=channel_number;
					ni_card=module_NI_CARDS+
						((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
					hardware_buffer=ni_card->hardware_buffer;
					number_of_configured_channels=ni_card->number_of_configured_channels;
					if ((0<number_of_configured_channels)&&
						(0<=(offset=(ni_card->hardware_buffer_offsets)[
						(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])))
					{
						offset += number_of_configured_channels*sample_number;
						averaging_length=module_scrolling_callback_period/
							module_number_of_scrolling_values_per_channel;
						for (j=module_number_of_scrolling_values_per_channel-1;j>=0;j--)
						{
							sum=0;
							for (i=averaging_length;i>0;i--)
							{
								sum += (long)(hardware_buffer[offset]);
								if (offset<number_of_configured_channels)
								{
									offset += number_of_configured_channels*(number_of_samples-1);
								}
								else
								{
									offset -= number_of_configured_channels;
								}
							}
							value[j]=(SHORT)(sum/averaging_length);
						}
					}
					value += module_number_of_scrolling_values_per_channel;
				}
				if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
					(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
				{
					/* Unemap_2V1 and UnEmap_2V2 invert */
					value=value_array;
					for (k=module_number_of_scrolling_channels*
						module_number_of_scrolling_values_per_channel;k>0;k--)
					{
						*value= -(*value);
						value++;
					}
				}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		for (i=0;i<module_number_of_scrolling_channels;i++)
		{
			fprintf(unemap_debug,"  %d ",module_scrolling_channel_numbers[i]);
			for (j=0;j<module_number_of_scrolling_values_per_channel;j++)
			{
				fprintf(unemap_debug," %d",
					value_array[i*module_number_of_scrolling_values_per_channel+j]);
			}
			fprintf(unemap_debug,"\n");
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#if !defined (NO_MODULE_SCROLLING_CALLBACK)
#if defined (WIN32_USER_INTERFACE)
				if (module_scrolling_window)
				{
					if (module_scrolling_callback)
					{
						ALLOCATE(channel_numbers_2,int,module_number_of_scrolling_channels);
						ALLOCATE(value_array_2,SHORT,module_number_of_scrolling_channels*
							module_number_of_scrolling_values_per_channel);
						if (channel_numbers_2&&value_array_2)
						{
							memcpy((char *)channel_numbers_2,(char *)channel_numbers,
								module_number_of_scrolling_channels*sizeof(int));
							memcpy((char *)value_array_2,(char *)value_array,
								module_number_of_scrolling_channels*
								module_number_of_scrolling_values_per_channel*sizeof(SHORT));
							(*module_scrolling_callback)(module_number_of_scrolling_channels,
								channel_numbers_2,
								(int)module_number_of_scrolling_values_per_channel,
								value_array_2,module_scrolling_callback_data);
						}
						else
						{
							DEALLOCATE(channel_numbers_2);
							DEALLOCATE(value_array_2);
							if (first_error)
							{
								display_message(ERROR_MESSAGE,
	"scrolling_callback_NI.  Could not allocate channel_numbers_2/value_array_2");
								first_error=0;
							}
						}
					}
					PostMessage(module_scrolling_window,module_scrolling_message,
						(WPARAM)byte_array,(ULONG)number_of_bytes);
				}
				else
				{
#endif /* defined (WIN32_USER_INTERFACE) */
					(*module_scrolling_callback)(module_number_of_scrolling_channels,
						channel_numbers,(int)module_number_of_scrolling_values_per_channel,
						value_array,module_scrolling_callback_data);
#if defined (WIN32_USER_INTERFACE)
				}
#endif /* defined (WIN32_USER_INTERFACE) */
#else /* !defined (NO_MODULE_SCROLLING_CALLBACK) */
				DEALLOCATE(value_array);
				DEALLOCATE(channel_numbers);
#endif /* !defined (NO_MODULE_SCROLLING_CALLBACK) */
			}
		}
#endif /* !defined (SCROLLING_UPDATE_BUFFER_POSITION_AND_SIZE_ONLY) */
	}
#if defined (UNEMAP_THREAD_SAFE)
		unlock_mutex(scrolling_callback_mutex);
	}
	else
	{
		scrolling_callback_mutex_locked_count++;
	}
#endif /* defined (UNEMAP_THREAD_SAFE) */
	LEAVE;
#endif /* !defined (NO_SCROLLING_BODY) */
} /* scrolling_callback_NI */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static void stimulation_end_callback_NI(HWND handle,UINT message,WPARAM wParam,
	LPARAM lParam)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Always called so that <module_starting_sample_number> and
<module_sample_buffer_size> can be kept up to date.
==============================================================================*/
{
	int i;

	ENTER(stimulation_end_callback_NI);
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		i=0;
		while ((i<module_number_of_NI_CARDS)&&
			((int)message!=(module_NI_CARDS[i]).stimulation_end_callback_id))
		{
			i++;
		}
		if (i<module_number_of_NI_CARDS)
		{
			if ((module_NI_CARDS[i]).stimulation_end_callback)
			{
				(*((module_NI_CARDS[i]).stimulation_end_callback))(
					(module_NI_CARDS[i]).stimulation_end_callback_data);
			}
			do
			{
				if ((int)message==(module_NI_CARDS[i]).stimulation_end_callback_id)
				{
					(module_NI_CARDS[i]).stimulation_end_callback_id=0;
					(module_NI_CARDS[i]).stimulation_end_callback=
						(Unemap_stimulation_end_callback *)NULL;
					(module_NI_CARDS[i]).stimulation_end_callback_data=(void *)NULL;
				}
				i++;
			} while (i<module_number_of_NI_CARDS);
		}
	}
	LEAVE;
} /* stimulation_end_callback_NI */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static int set_NI_gain(struct NI_card *ni_card,int gain)
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
Convenience function for setting the <ni_card> <gain>.  Doesn't change the gain
field of <ni_card>.
==============================================================================*/
{
	int return_code,sampling_on;
	i16 gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],i,j,number_of_scanned_channels,
		scan_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],status;
	struct NI_card *ni_card_temp;

	ENTER(set_NI_gain);
	return_code=0;
	if (ni_card)
	{
		if (sampling_on=module_sampling_on)
		{
			unemap_stop_sampling();
		}
		/* would like to be able to restart the rolling buffer for just <ni_card>,
			but because the rolling buffer can only be restarted at the beginning,
			all the cards have to be restarted, otherwise they'd be out of synch */
		module_sample_buffer_size=0;
		module_starting_sample_number=0;
		ni_card_temp=module_NI_CARDS;
		status=0;
		j=module_number_of_NI_CARDS;
		while ((0==status)&&(j>0))
		{
			status=DAQ_Clear(ni_card_temp->device_number);
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,"set_NI_gain.  DAQ_Clear(%d)=%d\n",
				ni_card_temp->device_number,status);
#endif /* defined (DEBUG) */
			if (0==status)
			{
				if (module_NI_CARDS==ni_card_temp)
				{
#if !defined (NO_SCROLLING_CALLBACK)
					status=Config_DAQ_Event_Message(ni_card_temp->device_number,
						/* add message */(i16)1,/* channel string */"AI0",
						/* send message every N scans */(i16)1,
						(i32)module_scrolling_callback_period,(i32)0,(u32)0,(u32)0,
						(u32)0,(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"set_NI_gain.  "
						"Config_DAQ_Event_Message(%d,1,AI0,1,%d,0,0,0,0,NULL,0,%p)=%d\n",
						ni_card_temp->device_number,module_scrolling_callback_period,
						scrolling_callback_NI,status);
#endif /* defined (DEBUG) */
#endif /* !defined (NO_SCROLLING_CALLBACK) */
					if (0!=status)
					{
						display_message(ERROR_MESSAGE,
							"set_NI_gain.  Config_DAQ_Event_Message 1 failed.  %d %d",status,
							module_scrolling_callback_period);
					}
				}
				status=DAQ_DB_Config(ni_card_temp->device_number,/* enable */(i16)1);
				if (0==status)
				{
					if (0<ni_card_temp->number_of_configured_channels)
					{
						number_of_scanned_channels=
							ni_card_temp->number_of_configured_channels;
						for (i=0;i<number_of_scanned_channels;i++)
						{
							scan_vector[i]=(ni_card_temp->channel_reorder)[
								((ni_card_temp->configured_channels)[i]-1)%
								NUMBER_OF_CHANNELS_ON_NI_CARD];
						}
					}
					else
					{
						number_of_scanned_channels=1;
						scan_vector[0]=0;
					}
					if (ni_card==ni_card_temp)
					{
						for (i=0;i<number_of_scanned_channels;i++)
						{
							gain_vector[i]=(i16)gain;
						}
					}
					else
					{
						for (i=0;i<number_of_scanned_channels;i++)
						{
							gain_vector[i]=(i16)(ni_card_temp->gain);
						}
					}
					status=SCAN_Setup(ni_card_temp->device_number,
						number_of_scanned_channels,scan_vector,gain_vector);
					if (0==status)
					{
						status=SCAN_Start(ni_card_temp->device_number,
							(i16 *)(ni_card_temp->hardware_buffer),
							(u32)(ni_card_temp->hardware_buffer_size),
							ni_card_temp->time_base,ni_card_temp->sampling_interval,(i16)0,
							(u16)0);
						if (0==status)
						{
							if (module_NI_CARDS==ni_card_temp)
							{
								/*???DB.  Start USE_INTERRUPTS_FOR_AI */
								/*???DB.  I don't understand why this is needed, but if it is
									not present, then get lots of extraneous callbacks (lParam is
									wrong) */
#if !defined (NO_SCROLLING_CALLBACK)
								status=Config_DAQ_Event_Message(
									module_NI_CARDS[0].device_number,
									/* clear all messages */(i16)0,
									/* channel string */(char *)NULL,
									/* send message every N scans */(i16)0,
									(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,(HWND)NULL,(i16)0,(u32)0);
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,"set_NI_gain 2.  "
									"Config_DAQ_Event_Message(%d,0,NULL,0,0,0,0,0,0,NULL,0,0)=%d\n",
									module_NI_CARDS[0].device_number,status);
#endif /* defined (DEBUG) */
#endif /* !defined (NO_SCROLLING_CALLBACK) */
								if (0!=status)
								{
									display_message(ERROR_MESSAGE,
										"set_NI_gain.  Config_DAQ_Event_Message 2 failed.  %d",
										status);
								}
#if !defined (USE_INTERRUPTS_FOR_AI)
#endif /* !defined (USE_INTERRUPTS_FOR_AI) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_NI_gain.  SCAN_Start failed.  %d",status);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_NI_gain.  SCAN_Setup failed.  %d",status);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_NI_gain.  DAQ_DB_Config failed.  %d",status);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"set_NI_gain.  DAQ_Clear failed.  %d",
					status);
			}
			j--;
			ni_card_temp++;
		}
		if (0==status)
		{
			return_code=1;
		}
		if (sampling_on)
		{
			unemap_start_sampling();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_NI_gain.  Invalid argument.  %p",
			ni_card);
	}
	LEAVE;

	return (return_code);
} /* set_NI_gain */
#endif /* defined (NI_DAQ) */

static int compare_float(const float *float_1,const float *float_2)
{
	int return_code;

	ENTER(compare_float);
	if (*float_1< *float_2)
	{
		return_code= -1;
	}
	else
	{
		if (*float_1> *float_2)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_float */

#if defined (NI_DAQ)
static int load_NI_DA(i16 da_channel,int number_of_channels,
	int *channel_numbers,int number_of_voltages,float voltages_per_second,
	float *voltages,unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_channels> is greater than 0 then the function applies to the NI
card(s) (channel_number-1) div 64 for all the <channel_numbers>.  If
<number_of_channels> is 0 then the function applies to all NI cards.  Otherwise,
the function fails.

Loads the WFM generator for the <da_channel> of the specified NI card(s) with
the the signal in <voltages> at the specified number of <voltages_per_second>.

The <voltages> are those desired (in volts).  The function sets <voltages> to
the actual values used.

If <number_of_cycles> is zero then the waveform is repeated until <stop_NI_DA>,
otherwise the waveform is repeated the <number_of_cycles> times or until
<stop_NI_DA>.
==============================================================================*/
{
	char *channel_string;
	f64 da_frequency,*da_voltage_buffer;
	int *channel_number,first_DA_card,i,j,k,*load_card,return_code,
		stimulation_end_callback_id;
	i16 *da_buffer,status;
	u32 number_of_cycles_local,number_of_da_points;
#if defined (USE_WFM_LOAD)
	i16 timebase;
	u32 update_interval;
#endif /* defined (USE_WFM_LOAD) */
#if defined (OLD_CODE)
	i16 stopped;
	u32 iterations,points;
#endif /* defined (OLD_CODE) */

	ENTER(load_NI_DA);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_voltages;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"enter load_NI_DA %d %d %p %d %g %p %u %p %p\n",
			da_channel,number_of_channels,channel_numbers,number_of_voltages,
			voltages_per_second,voltages,number_of_cycles,stimulation_end_callback,
			stimulation_end_callback_data);
		fclose(unemap_debug);
	}
#if defined (DEBUG)
	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		if ((0<number_of_channels)&&channel_numbers)
		{
			fprintf(unemap_debug,"  channels:");
			for (i=0;i<number_of_channels;i++)
			{
				fprintf(unemap_debug," %d",channel_numbers[i]);
			}
			fprintf(unemap_debug,"\n");
		}
		fprintf(unemap_debug,"  voltages:\n");
		maximum_voltages=10;
		if (number_of_voltages<maximum_voltages)
		{
			maximum_voltages=number_of_voltages;
		}
		for (i=0;i<maximum_voltages;i++)
		{
			fprintf(unemap_debug,"    %d %g\n",i,voltages[i]);
		}
		fclose(unemap_debug);
	}
#endif /* defined (DEBUG) */
}
#endif /* defined (DEBUG) */
	return_code=0;
	/* check arguments */
	if ((0==number_of_channels)||((0<number_of_channels)&&channel_numbers))
	{
		return_code=1;
		i=number_of_channels;
		channel_number=channel_numbers;
		while (return_code&&(i>0))
		{
			if ((0<(j= *channel_number))&&
				(((k=(j-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<module_number_of_NI_CARDS)&&
				(0<=(module_NI_CARDS[k].hardware_buffer_offsets)[(j-1)%
				NUMBER_OF_CHANNELS_ON_NI_CARD])))
			{
				i--;
				channel_number++;
			}
			else
			{
				return_code=0;
			}
		}
	}
	if (return_code&&((0==number_of_voltages)||
		((1==number_of_voltages)&&voltages)||((1<number_of_voltages)&&voltages&&
		(0<voltages_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			number_of_cycles_local=(u32)number_of_cycles;
#if defined (DA_INTERRUPTS_OFF)
			if (1<number_of_voltages)
			{
				number_of_da_points=(u32)number_of_voltages;
				da_frequency=(f64)voltages_per_second;
			}
			else
			{
				number_of_da_points=(u32)2;
				da_frequency=(f64)1;
			}
#else /* defined (DA_INTERRUPTS_OFF) */
			if (MINIMUM_NUMBER_OF_DA_POINTS<=number_of_voltages)
			{
				number_of_da_points=(u32)number_of_voltages;
				da_frequency=(f64)voltages_per_second;
			}
			else
			{
				if (0<number_of_cycles_local)
				{
					if (1<number_of_voltages)
					{
						number_of_da_points=
							(u32)(number_of_voltages*number_of_cycles);
						da_frequency=(f64)voltages_per_second;
					}
					else
					{
						number_of_da_points=(u32)2;
						da_frequency=(f64)1;
					}
					number_of_cycles_local=(u32)1;
				}
				else
				{
					if (1<number_of_voltages)
					{
						number_of_da_points=(u32)((MINIMUM_NUMBER_OF_DA_POINTS/
							number_of_voltages+1)*number_of_voltages);
						da_frequency=(f64)voltages_per_second;
					}
					else
					{
						number_of_da_points=(u32)2;
						da_frequency=(f64)1;
						number_of_cycles_local=(u32)1;
						/*???debug */
						number_of_da_points=(u32)MINIMUM_NUMBER_OF_DA_POINTS;
						number_of_cycles_local=(u32)0;
					}
				}
			}
#endif /* defined (DA_INTERRUPTS_OFF) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"  number_of_da_points=%lu\n",number_of_da_points);
		fprintf(unemap_debug,"  da_frequency=%g\n",da_frequency);
		fprintf(unemap_debug,"  number_of_cycles_local=%lu\n",
			number_of_cycles_local);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
			ALLOCATE(load_card,int,module_number_of_NI_CARDS);
			ALLOCATE(da_voltage_buffer,f64,number_of_da_points);
			if (load_card&&da_voltage_buffer)
			{
				if (0==number_of_channels)
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						if (0<module_NI_CARDS[i].number_of_configured_channels)
						{
							load_card[i]=1;
						}
						else
						{
							load_card[i]=0;
						}
					}
				}
				else
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						load_card[i]=0;
					}
					for (i=0;i<number_of_channels;i++)
					{
						load_card[(channel_numbers[i]-1)/NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
					}
				}
				stimulation_end_callback_id=0;
				if (stimulation_end_callback)
				{
					j=1;
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						if (load_card[i])
						{
							stimulation_end_callback_id += j;
						}
						j *= 2;
					}
				}
				switch (number_of_voltages)
				{
					case 0:
					{
						for (i=0;i<(int)number_of_da_points;i++)
						{
							da_voltage_buffer[i]=(f64)0;
						}
					} break;
					case 1:
					{
						for (i=0;i<(int)number_of_da_points;i++)
						{
							da_voltage_buffer[i]=(f64)(*voltages);
						}
					} break;
					default:
					{
						k=0;
						for (i=number_of_da_points/number_of_voltages;i>0;i--)
						{
							for (j=0;j<number_of_voltages;j++)
							{
								da_voltage_buffer[k]=(f64)voltages[j];
								k++;
							}
						}
					} break;
				}
				i=0;
				first_DA_card=1;
				while (return_code&&(i<module_number_of_NI_CARDS))
				{
					if (load_card[i]&&((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
					{
#if defined (OLD_CODE)
						/* check if waveform generation is already underway */
							/*???DB.  Just to try out stopping.  Might use for stimulation */
						status=WFM_Check((module_NI_CARDS[i]).device_number,da_channel,
							&stopped,&iterations,&points);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"  WFM_Check.  %d %d %d %lu %lu %d\n",
			(module_NI_CARDS[i]).device_number,da_channel,stopped,iterations,points,
			status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						if ((0==status)&&!stopped)
						{
#endif /* defined (OLD_CODE) */
							/* make sure that waveform generation is stopped */
								/*???DB.  Not sure if it is possible to have different signals
									for AO0 and AO1 or if we need it */
							status=WFM_Group_Control((module_NI_CARDS[i]).device_number,
								/*group*/1,/*Clear*/0);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"Load_NI_DA.  1.  WFM_Group_Control(%d,CLEAR)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
#if !defined (SYNCHRONOUS_STIMULATION)
#if defined (OLD_CODE)
							/* set the output voltage to 0 */
							AO_Write((module_NI_CARDS[i]).device_number,CALIBRATE_CHANNEL,
								(i16)0);
#endif /* defined (OLD_CODE) */
#endif /* !defined (SYNCHRONOUS_STIMULATION) */
						}
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
						/*???DB.  May not even have started */
						/*???DB.  Call the existing stimulation_end_callback ? */
						if ((module_NI_CARDS[i]).stimulation_end_callback)
						{
							stimulation_end_callback_NI((HWND)NULL,
								(UINT)(module_NI_CARDS[i]).stimulation_end_callback_id,
								(WPARAM)NULL,(LPARAM)NULL);
						}
#endif /* defined (OLD_CODE) */
						/* add the stimulation end callback */
						(module_NI_CARDS[i]).stimulation_end_callback_id=
							stimulation_end_callback_id;
						(module_NI_CARDS[i]).stimulation_end_callback=
							stimulation_end_callback;
						(module_NI_CARDS[i]).stimulation_end_callback_data=
							stimulation_end_callback_data;
						if (stimulation_end_callback)
						{
							if (1==da_channel)
							{
								channel_string="AO1";
							}
							else
							{
								channel_string="AO0";
							}
							status=Config_DAQ_Event_Message(module_NI_CARDS[i].device_number,
								/* add message */(i16)1,channel_string,/* completed */(i16)2,
								(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,(HWND)NULL,
								(i16)stimulation_end_callback_id,
								(u32)stimulation_end_callback_NI);
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,"load_NI_DA.  "
								"Config_DAQ_Event_Message(%d,1,%s,2,0,0,0,0,0,NULL,%d,%p)=%d\n",
								module_NI_CARDS[i].device_number,channel_string,
								stimulation_end_callback_id,stimulation_end_callback_NI,status);
#endif /* defined (DEBUG) */
						}
						/* configure the DA to bipolar +/-10 */
						status=AO_Configure((module_NI_CARDS[i]).device_number,da_channel,
							/*bipolar*/(i16)0,/*internal voltage reference*/(i16)0,
							/*reference voltage*/(f64)10,
							/*update DAC when written to*/(i16)0);
						if (0==status)
						{
#if defined (DA_INTERRUPTS_OFF)
/*???DB.  Causes problems when wraps ? */
							/* turn off the end of buffer interrupts */
							status=AO_Change_Parameter((module_NI_CARDS[i]).device_number,
								da_channel,ND_LINK_COMPLETE_INTERRUPTS,ND_OFF);
							if (0==status)
							{
#endif /* defined (DA_INTERRUPTS_OFF) */
								if (REALLOCATE(da_buffer,(module_NI_CARDS[i]).da_buffer,i16,
									number_of_da_points))
								{
									(module_NI_CARDS[i]).da_buffer=da_buffer;
								}
								if (da_buffer)
								{
									(module_NI_CARDS[i]).da_buffer_size=number_of_da_points;
									status=WFM_Scale((module_NI_CARDS[i]).device_number,
										da_channel,number_of_da_points,/*gain*/(f64)1,
										da_voltage_buffer,da_buffer);
									if (0==status)
									{
										if (first_DA_card)
										{
											first_DA_card=0;
											/* set the desired voltages to the actual voltages
												used */
											if (PXI6071E_AD_DA==(module_NI_CARDS[i]).type)
											{
												for (j=0;j<number_of_voltages;j++)
												{
													voltages[j]=
														(float)da_buffer[j]*(float)20/(float)4096;
												}
											}
											else
											{
												for (j=0;j<number_of_voltages;j++)
												{
													voltages[j]=
														(float)da_buffer[j]*(float)20/(float)65536;
												}
											}
										}
#if defined (SYNCHRONOUS_STIMULATION)
#if defined (USE_WFM_LOAD)
										status=WFM_Load((module_NI_CARDS[i]).device_number,
											/*number of DA channels*/1,&da_channel,da_buffer,
											number_of_da_points,
											/*iterations*/number_of_cycles_local,/*mode*/(i16)0);
										if (0==status)
										{
											status=WFM_Rate(da_frequency,/*units*/(i16)0,&timebase,
												&update_interval);
											if (0==status)
											{
												status=WFM_ClockRate(
													(module_NI_CARDS[i]).device_number,
													/*group*/(i16)1,/*update clock*/(i16)0,timebase,
													update_interval,/*mode*/(i16)0);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"  WFM_ClockRate(%d,1,0,%d,%lu,0)=%d\n",
			(module_NI_CARDS[i]).device_number,timebase,update_interval,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
												if (0==status)
												{
#endif /* defined (USE_WFM_LOAD) */
										if (start_stimulation_card)
										{
											/* use the waveform trigger on the RTSI bus */
											status=Select_Signal(
												(module_NI_CARDS[i]).device_number,
												ND_OUT_START_TRIGGER,ND_RTSI_1,ND_LOW_TO_HIGH);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"  Select_Signal(%d,ND_OUT_START_TRIGGER,ND_RTSI_1,ND_LOW_TO_HIGH)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
											if (0==status)
											{
#if defined (USE_WFM_LOAD)
												status=WFM_Group_Control(
													module_NI_CARDS[i].device_number,/*group*/1,
													/*Start*/1);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"load_NI_DA.  2.  WFM_Group_Control(%d,START)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#else /* defined (USE_WFM_LOAD) */
												status=WFM_Op((module_NI_CARDS[i]).device_number,
													/*number of DA channels*/1,&da_channel,da_buffer,
													number_of_da_points,
													/*iterations*/(u32)number_of_cycles_local,
													/*frequency*/da_frequency);
#endif /* defined (USE_WFM_LOAD) */
												if (0!=status)
												{
													display_message(ERROR_MESSAGE,
														"load_NI_DA.  WFM_Op failed");
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
"load_NI_DA.  Select_Signal(%d,ND_OUT_START_TRIGGER,ND_RTSI_1,ND_LOW_TO_HIGH) failed",
													i);
											}
										}
										else
										{
											/* set the waveform trigger on the RTSI bus */
											status=Select_Signal(
												(module_NI_CARDS[i]).device_number,
												ND_RTSI_1,ND_OUT_START_TRIGGER,ND_LOW_TO_HIGH);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"  Select_Signal(%d,ND_RTSI_1,ND_OUT_START_TRIGGER,ND_LOW_TO_HIGH)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
											if (0==status)
											{
												start_stimulation_card=module_NI_CARDS+i;
												start_stimulation_da_channel=da_channel;
												start_stimulation_da_frequency=da_frequency;
												start_stimulation_iterations=
													(u32)number_of_cycles_local;
											}
											else
											{
												display_message(ERROR_MESSAGE,
"load_NI_DA.  Select_Signal(%d,ND_RTSI_1,ND_OUT_START_TRIGGER,ND_LOW_TO_HIGH) failed",
													i);
											}
										}
#if defined (USE_WFM_LOAD)
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"load_NI_DA.  WFM_ClockRate failed (%d).  %d",i,
														status);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"load_NI_DA.  WFM_Rate failed (%d).  %d",i,status);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"load_NI_DA.  WFM_Load failed (%d).  %d",i,status);
										}
#endif /* defined (USE_WFM_LOAD) */
#else /* defined (SYNCHRONOUS_STIMULATION) */
										if (0==number_of_voltages)
										{
											/* set the output voltage to 0 */
											AO_Write((module_NI_CARDS[i]).device_number,da_channel,
												(i16)0);
										}
										else
										{
											if (1==number_of_voltages)
											{
												AO_Write((module_NI_CARDS[i]).device_number,
													da_channel,da_buffer[0]);
											}
											else
											{
												Timeout_Config((module_NI_CARDS[i]).device_number,
													(i32)0);
												status=WFM_Op((module_NI_CARDS[i]).device_number,
													/*number of DA channels*/1,&da_channel,da_buffer,
													number_of_da_points,
													/*iterations*/(u32)number_of_cycles_local,
													/*frequency*/da_frequency);
												if (0!=status)
												{
													display_message(ERROR_MESSAGE,
														"load_NI_DA.  WFM_Op failed");
												}
											}
										}
#endif /* defined (SYNCHRONOUS_STIMULATION) */
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"load_NI_DA.  WFM_Scale failed (%d).  %d",i,status);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"load_NI_DA.  Could not reallocate da_buffer (%d,%lu)",i,
										number_of_da_points);
								}
#if defined (DA_INTERRUPTS_OFF)
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"load_NI_DA.  AO_Change_Parameter failed");
							}
#endif /* defined (DA_INTERRUPTS_OFF) */
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"load_NI_DA.  AO_Configure failed");
						}
					}
					i++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"load_NI_DA.  Could not allocate load_card (%p) or da_voltage_buffer (%p)",
					load_card,da_voltage_buffer);
				return_code=0;
			}
			DEALLOCATE(da_voltage_buffer);
			DEALLOCATE(load_card);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"load_NI_DA.  Invalid configuration.  %d %p %d",module_configured,
				module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"load_NI_DA.  Invalid arguments %d %d %p %d %g %p",da_channel,
			number_of_channels,channel_numbers,number_of_voltages,voltages_per_second,
			voltages);
	}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"leave load_NI_DA %d\n",return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* load_NI_DA */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static int start_NI_DA(void)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts waveform generation for all the DAs that have been loaded (with
load_NI_DA) and have not yet started.
==============================================================================*/
{
	int return_code;
#if defined (SYNCHRONOUS_STIMULATION)
	int i;
	i16 status;
#endif /* defined (SYNCHRONOUS_STIMULATION) */

	ENTER(start_NI_DA);
	return_code=0;
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"enter start_NI_DA %d %d %p\n",
			module_configured,module_number_of_NI_CARDS,module_NI_CARDS);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
#if defined (SYNCHRONOUS_STIMULATION)
		if (start_stimulation_card)
		{
#if defined (USE_WFM_LOAD)
			status=WFM_Group_Control(start_stimulation_card->device_number,/*group*/1,
				/*Start*/1);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start_NI_DA.  WFM_Group_Control(%d,START)=%d\n",
			start_stimulation_card->device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#else /* defined (USE_WFM_LOAD) */
			Timeout_Config(start_stimulation_card->device_number,(i32)0);
			status=WFM_Op(start_stimulation_card->device_number,
				/*number of DA channels*/1,&start_stimulation_da_channel,
				start_stimulation_card->da_buffer,
				start_stimulation_card->da_buffer_size,
				/*iterations*/start_stimulation_iterations,
				/*frequency*/start_stimulation_da_frequency);
#endif /* defined (USE_WFM_LOAD) */
			status=Select_Signal(start_stimulation_card->device_number,ND_RTSI_1,
				ND_NONE,ND_DONT_CARE);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"  Select_Signal(%d,ND_RTSI_1,ND_NONE,ND_DONT_CARE)=%d\n",
			start_stimulation_card->device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
			start_stimulation_card=(struct NI_card *)NULL;
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				status=Select_Signal((module_NI_CARDS[i]).device_number,
					ND_OUT_START_TRIGGER,ND_AUTOMATIC,ND_LOW_TO_HIGH);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"  Select_Signal(%d,ND_OUT_START_TRIGGER,ND_AUTOMATIC,ND_LOW_TO_HIGH)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
				if (0!=status)
				{
					display_message(ERROR_MESSAGE,
"start_NI_DA.  Select_Signal(%d,ND_OUT_START_TRIGGER,ND_AUTOMATIC,ND_LOW_TO_HIGH)=%d",
						(module_NI_CARDS[i]).device_number,status);
				}
			}
		}
#endif /* defined (SYNCHRONOUS_STIMULATION) */
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"start_NI_DA.  Invalid configuration.  %d %p %d",module_configured,
			module_NI_CARDS,module_number_of_NI_CARDS);
	}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"leave start_NI_DA %d\n",return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* start_NI_DA */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static int stop_NI_DA(i16 da_channel,int channel_number)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the NI card with channels ((<channel_number>-1)
div 64)*64+1 to ((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0
then the function applies to all NI cards.  Otherwise, the function fails.

Stops the <da_channel> for the specified NI card(s).
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
	i16 status;
#if defined (OLD_CODE)
	i16 stopped;
	u32 iterations,points;
#endif /* defined (OLD_CODE) */
#endif /* defined (NI_DAQ) */

	ENTER(stop_NI_DA);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"enter stop_NI_DA %d %d\n",da_channel,channel_number);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		card_number= -1;
		if ((0<=channel_number)&&((0==channel_number)||
			(((card_number=(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[card_number].
			hardware_buffer_offsets)[(channel_number-1)%
			NUMBER_OF_CHANNELS_ON_NI_CARD]))))
		{
			return_code=1;
			i=0;
			while (i<module_number_of_NI_CARDS)
			{
				if (((-1==card_number)||(i==card_number))&&
					((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
				{
#if defined (OLD_CODE)
					/* check if waveform generation is already underway */
					status=WFM_Check((module_NI_CARDS[i]).device_number,da_channel,
						&stopped,&iterations,&points);
					if ((0==status)&&!stopped)
					{
#endif /* defined (OLD_CODE) */
						/* stop waveform generation */
						status=WFM_Group_Control((module_NI_CARDS[i]).device_number,
							/*group*/1,/*Clear*/0);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"stop_NI_DA.  WFM_Group_Control(%d,CLEAR)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"  WFM_Group_Control %d\n",i);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
							(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
						{
							/* make sure that constant voltage */
							set_shift_register(module_NI_CARDS+i,
								Stim_Source_SHIFT_REGISTER_UnEmap2vx,0,1);
						}
						/* set the output voltage to 0 */
						AO_Write((module_NI_CARDS[i]).device_number,da_channel,(i16)0);
#if defined (OLD_CODE)
					}
#endif /* defined (OLD_CODE) */
				}
				i++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"stop_NI_DA.  Invalid argument %d",channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"stop_NI_DA.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"leave stop_NI_DA %d\n",return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* stop_NI_DA */
#endif /* defined (NI_DAQ) */

#if defined (NI_DAQ)
static int calibration_callback(void *dummy)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Output a square wave which alternates between +/- a known voltage.  Wait for the
DC removal to settle.  Acquire.  Average for each channel.  Assume that the
measurements below the average are for minus the known voltage and that the
measurements above the average are for plus the known voltage.  Calculate
offsets and gains.  Over-write calibrate.dat

Only calibrate configured channels (because thats what hardware buffers are set
up for).

The D/A on the NI cards doesn't have enough resolution (+/-10V and 12 or 16-bit)
to cope with the full gain (100*11*8=8800), so the NI gain and the signal
conditioning gain are done separately and then combined.

stage 0 is setup
stage 1 is calculating the signal conditioning fixed gain
stage 2 is calculating the signal conditioning variable gain
stage 3 is calculating the NI gain
stage 4 is calculating the offset
==============================================================================*/
{
	float *calibrate_amplitude,*calibration_gains,*calibration_offsets,
		*filter_frequency,*gain,*offset,*previous_offset,temp;
	int *calibration_channels,card_number,channel_number,i,j,l,
		number_of_configured_channels,number_of_settled_channels,return_code;
	i16 da_channel,status;
	long int maximum_sample_value,minimum_sample_value;
	short int *hardware_buffer;
	static float *filter_frequencies,*gains=(float *)NULL,*offsets=(float *)NULL,
		*previous_offsets=(float *)NULL,*saturated=(float *)NULL;
	static float *calibrate_amplitudes,*calibrate_voltages;
	static int calibrate_stage=0,*channel_failed,number_of_waveform_points,
		settling_step,stage_flag,stage_mask;
	static unsigned long number_of_samples;
	struct NI_card *ni_card;
	unsigned long k,k1,k2;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
	float *sorted_signal_entry;
	static float *sorted_signal=(float *)NULL;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
	float temp_2;
	double two_pi;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */

	ENTER(calibration_callback);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"enter calibration_callback %d\n",calibrate_stage);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	return_code=0;
	status=0;
	if (0==calibrate_stage)
	{
		/* set up */
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS)&&
			(0<module_number_of_configured_channels))
		{
			stage_flag=0x1;
			stage_mask=0x1;
			if (0<module_NI_CARDS->number_of_configured_channels)
			{
				number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
					(module_NI_CARDS->number_of_configured_channels);
			}
			else
			{
				number_of_samples=module_NI_CARDS->hardware_buffer_size;
			}
			/* limit to 5 seconds sampling */
			if (0<module_sampling_frequency)
			{
				k=(unsigned long)((float)5*module_sampling_frequency);
				if ((0<k)&&(k<number_of_samples))
				{
					number_of_samples=k;
				}
			}
			ALLOCATE(calibrate_amplitudes,float,module_number_of_NI_CARDS);
#if defined (CALIBRATE_SIGNAL_SQUARE)
			number_of_waveform_points=2;
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
			number_of_waveform_points=500;
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
			ALLOCATE(calibrate_voltages,float,number_of_waveform_points);
			ALLOCATE(filter_frequencies,float,module_number_of_NI_CARDS);
			ALLOCATE(offsets,float,module_number_of_configured_channels);
			ALLOCATE(previous_offsets,float,module_number_of_configured_channels);
			ALLOCATE(gains,float,module_number_of_configured_channels);
			ALLOCATE(channel_failed,int,module_number_of_configured_channels);
			ALLOCATE(saturated,float,module_number_of_configured_channels);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
			ALLOCATE(sorted_signal,float,number_of_samples);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			if (calibrate_voltages&&filter_frequencies&&offsets&&previous_offsets&&
				gains&&channel_failed&&calibrate_amplitudes
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				&&sorted_signal
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
				)
			{
				/* stop sampling and put in isolate/calibrate mode */
				if ((return_code=unemap_stop_sampling())&&
					(return_code=unemap_set_isolate_record_mode(0,1)))
				{
					for (i=0;i<module_number_of_configured_channels;i++)
					{
						offsets[i]=(float)0;
						channel_failed[i]=0xf;
						unemap_get_sample_range(module_configured_channels[i],
							&minimum_sample_value,&maximum_sample_value);
						saturated[i]=(float)(-minimum_sample_value);
						if ((float)maximum_sample_value<saturated[i])
						{
							saturated[i]=(float)maximum_sample_value;
						}
					}
					card_number=0;
					ni_card=module_NI_CARDS;
					calibrate_amplitude=calibrate_amplitudes;
					filter_frequency=filter_frequencies;
					da_channel=CALIBRATE_CHANNEL;
					while (return_code&&(card_number<module_number_of_NI_CARDS))
					{
						if (0<ni_card->number_of_configured_channels)
						{
							channel_number=(ni_card->configured_channels)[0];
							/* change NI gain to 1 */
							return_code=set_NI_gain(ni_card,(int)1);
							if (return_code)
							{
								if ((UnEmap_1V2==ni_card->unemap_hardware_version)||
									(UnEmap_2V2==ni_card->unemap_hardware_version))
								{
									/* set the programmable gain */
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_0_output_line_UnEmap2vx,(i16)0);
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_1_output_line_UnEmap2vx,(i16)0);
								}
								/* set low pass filter as high as possible */
								unemap_get_antialiasing_filter_frequency(channel_number,
									filter_frequency);
								unemap_set_antialiasing_filter_frequency(channel_number,
									(float)10000);
								/* determine the calibration voltage */
								/* start with full-range voltage */
								if (PXI6071E_AD_DA==ni_card->type)
								{
									*calibrate_amplitude=(float)5;
								}
								else
								{
									*calibrate_amplitude=(float)10;
								}
								/* use half-range */
								*calibrate_amplitude /= 2;
								switch (ni_card->unemap_hardware_version)
								{
									case UnEmap_1V2:
									{
										/* fixed gain of 10 */
										*calibrate_amplitude /= 10;
									} break;
									case UnEmap_2V1:
									case UnEmap_2V2:
									{
										/* fixed gain of 11 */
										*calibrate_amplitude /= 11;
									} break;
								}
#if defined (CALIBRATE_SIGNAL_SQUARE)
								calibrate_voltages[0]= *calibrate_amplitude;
								calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* calculate cosine wave */
								two_pi=(double)8*atan((double)1);
								for (i=0;i<number_of_waveform_points;i++)
								{
									calibrate_voltages[i]=
										(*calibrate_amplitude)*(float)cos(two_pi*
										(double)i/(double)number_of_waveform_points);
								}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								if (return_code=load_NI_DA(CALIBRATE_CHANNEL,1,&channel_number,
									number_of_waveform_points,(float)number_of_waveform_points*
									CALIBRATE_FREQUENCY,calibrate_voltages,0,
									(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
								{
									*calibrate_amplitude=calibrate_voltages[0];
									ni_card++;
									card_number++;
									calibrate_amplitude++;
									filter_frequency++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  load_NI_DA failed 1");
									set_NI_gain(ni_card,(int)(ni_card->gain));
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"calibration_callback.  set_NI_gain failed 1");
							}
						}
						else
						{
							ni_card++;
							card_number++;
							calibrate_amplitude++;
							filter_frequency++;
						}
					}
					if (return_code)
					{
						module_buffer_full_callback=calibration_callback;
						module_buffer_full_callback_data=(void *)NULL;
						module_buffer_full_size=number_of_samples;
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,"module_buffer_full_size=%lu\n",
							module_buffer_full_size);
#endif /* defined (DEBUG) */
						settling_step=0;
						calibrate_stage=1;
						if ((return_code=start_NI_DA())&&
							(return_code=unemap_start_sampling()))
						{
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 1 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 1\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
						else
						{
							module_buffer_full_callback=(Buffer_full_callback *)NULL;
							module_buffer_full_callback_data=(void *)NULL;
						}
					}
					if (!return_code)
					{
						while (card_number>0)
						{
							card_number--;
							ni_card--;
							calibrate_amplitude--;
							filter_frequency--;
							if (0<ni_card->number_of_configured_channels)
							{
								channel_number=(ni_card->configured_channels)[0];
								unemap_set_antialiasing_filter_frequency(channel_number,
									*filter_frequency);
								/* stop waveform generation */
								status=WFM_Group_Control(ni_card->device_number,/*group*/1,
									/*Clear*/0);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"calibration_callback.  1.  WFM_Group_Control(%d,CLEAR)=%d\n",
			ni_card->device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
								/* clear start */
								status=Select_Signal(ni_card->device_number,
									ND_OUT_START_TRIGGER,ND_AUTOMATIC,ND_LOW_TO_HIGH);
								if (0!=status)
								{
									display_message(ERROR_MESSAGE,"calibration_callback.  "
										"Select_Signal(%d,ND_OUT_START_TRIGGER,ND_AUTOMATIC,"
										"ND_LOW_TO_HIGH)=%d",ni_card->device_number,status);
								}
								/* set the output voltage to 0 */
								AO_Write(ni_card->device_number,CALIBRATE_CHANNEL,(i16)0);
								set_NI_gain(ni_card,(int)(ni_card->gain));
								if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
									(UnEmap_2V2==ni_card->unemap_hardware_version))
								{
									/* programmable gain */
									if ((ni_card->unemap_2vx).gain_output_line_0)
									{
										DIG_Out_Line(ni_card->device_number,(i16)0,
											gain_0_output_line_UnEmap2vx,(i16)1);
									}
									if ((ni_card->unemap_2vx).gain_output_line_1)
									{
										DIG_Out_Line(ni_card->device_number,(i16)0,
											gain_1_output_line_UnEmap2vx,(i16)1);
									}
								}
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
			"calibration_callback.  Could not stop sampling or put in isolate mode");
				}
			}
			else
			{
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				display_message(ERROR_MESSAGE,
		"calibration_callback.  Could not allocate storage.  %p %p %p %p %p %p",
					calibrate_voltages,filter_frequencies,offsets,previous_offsets,gains,
					sorted_signal);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
				display_message(ERROR_MESSAGE,
				"calibration_callback.  Could not allocate storage.  %p %p %p %p %p",
					calibrate_voltages,filter_frequencies,offsets,previous_offsets,gains);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			}
			if (0==return_code)
			{
				DEALLOCATE(calibrate_amplitudes);
				DEALLOCATE(calibrate_voltages);
				DEALLOCATE(filter_frequencies);
				DEALLOCATE(offsets);
				DEALLOCATE(previous_offsets);
				DEALLOCATE(gains);
				DEALLOCATE(channel_failed);
				DEALLOCATE(saturated);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
				DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"calibration_callback.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
		if ((0==return_code)&&module_calibration_end_callback)
		{
			(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
				(float *)NULL,module_calibration_end_callback_data);
		}
	}
	else
	{
		/* calibrate stage 1, 2, 3 or 4 */
		unemap_stop_sampling();
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"After unemap_stop_sampling\n");
#endif /* defined (DEBUG) */
		/* average */
		offset=offsets;
		previous_offset=previous_offsets;
		for (i=0;i<module_number_of_configured_channels;i++)
		{
			*previous_offset= *offset;
			*offset=(float)0;
			previous_offset++;
			offset++;
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"After zeroing offsets %lu %lu %lu\n",
			module_sample_buffer_size,module_starting_sample_number,
			number_of_samples);
#endif /* defined (DEBUG) */
		if (0<module_NI_CARDS->number_of_configured_channels)
		{
			k=(module_NI_CARDS->hardware_buffer_size)/
				(module_NI_CARDS->number_of_configured_channels);
		}
		else
		{
			k=module_NI_CARDS->hardware_buffer_size;
		}
		if (module_sample_buffer_size+number_of_samples>k)
		{
			k1=k-module_sample_buffer_size;
			k2=module_sample_buffer_size+number_of_samples-k;
		}
		else
		{
			k1=number_of_samples;
			k2=0;
		}
		for (i=0;i<module_number_of_configured_channels;i++)
		{
			ni_card=module_NI_CARDS+(module_configured_channels[i]-1)/
				NUMBER_OF_CHANNELS_ON_NI_CARD;
			number_of_configured_channels=ni_card->number_of_configured_channels;
			j=0;
			while ((j<number_of_configured_channels)&&(module_configured_channels[i]!=
				(ni_card->configured_channels)[j]))
			{
				j++;
			}
			if (j<number_of_configured_channels)
			{
				hardware_buffer=(ni_card->hardware_buffer)+
					(module_starting_sample_number*number_of_configured_channels+j);
				temp=(float)0;
				for (k=k1;k>0;k--)
				{
					temp += (float)(*hardware_buffer);
					hardware_buffer += number_of_configured_channels;
				}
				hardware_buffer=(ni_card->hardware_buffer)+j;
				for (k=k2;k>0;k--)
				{
					temp += (float)(*hardware_buffer);
					hardware_buffer += number_of_configured_channels;
				}
				offsets[i]=temp/(float)number_of_samples;
			}
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"After calculating offsets\n");
#endif /* defined (DEBUG) */
		if (0<settling_step)
		{
			number_of_settled_channels=0;
			for (i=0;i<module_number_of_configured_channels;i++)
			{
				if (channel_failed[i]&stage_flag)
				{
					if (((float)fabs((double)(previous_offsets[i]))<saturated[i])&&
						((float)fabs((double)(offsets[i]))<saturated[i])&&
						((float)fabs((double)(offsets[i]-previous_offsets[i]))<
						tol_settling))
					{
						channel_failed[i] &= !stage_flag;
						number_of_settled_channels++;
					}
				}
				if (!(channel_failed[i]&stage_mask))
				{
					number_of_settled_channels++;
				}
			}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	float maximum,temp;
	int ii,jj;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		maximum=(float)-1;
		jj= -1;
		for (ii=0;ii<module_number_of_configured_channels;ii++)
		{
			temp=(float)fabs((double)(offsets[ii]-previous_offsets[ii]));
			if (temp>maximum)
			{
				maximum=temp;
				jj=ii;
			}
		}
		fprintf(unemap_debug,"settling %d %d %g %g %d %g\n",settling_step,i,
			saturated,tol_settling,jj,maximum);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
			if ((number_of_settled_channels>=module_number_of_configured_channels)||
				((number_of_settled_channels>0)&&(settling_step>=settling_step_max)))
			{
				/* settled */
				if ((1==calibrate_stage)||(2==calibrate_stage)||(3==calibrate_stage))
				{
					gain=gains;
					offset=offsets;
					for (i=0;i<module_number_of_configured_channels;i++)
					{
						j=(module_configured_channels[i]-1)/
							NUMBER_OF_CHANNELS_ON_NI_CARD;
						ni_card=module_NI_CARDS+j;
						calibrate_amplitude=calibrate_amplitudes+j;
						number_of_configured_channels=
							ni_card->number_of_configured_channels;
						j=0;
						while ((j<number_of_configured_channels)&&
							(module_configured_channels[i]!=
							(ni_card->configured_channels)[j]))
						{
							j++;
						}
						if (j<number_of_configured_channels)
						{
							hardware_buffer=(ni_card->hardware_buffer)+
								(module_starting_sample_number*number_of_configured_channels+j);
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							sorted_signal_entry=sorted_signal;
							for (k=k1;k>0;k--)
							{
								temp=(float)(*hardware_buffer)-(*offset);
								if (temp<(float)0)
								{
									temp= -temp;
								}
								*sorted_signal_entry=temp;
								sorted_signal_entry++;
								hardware_buffer += number_of_configured_channels;
							}
#else /* defined (SWITCHING_TIME) */
							temp=(float)0;
							for (k=k1;k>0;k--)
							{
								temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							temp=(float)0;
							for (k=k1;k>0;k--)
							{
								temp_2=(float)(*hardware_buffer)-(*offset);
								temp += temp_2*temp_2;
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
							hardware_buffer=(ni_card->hardware_buffer)+j;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							for (k=k2;k>0;k--)
							{
								temp=(float)(*hardware_buffer)-(*offset);
								if (temp<(float)0)
								{
									temp= -temp;
								}
								*sorted_signal_entry=temp;
								sorted_signal_entry++;
								hardware_buffer += number_of_configured_channels;
							}
#else /* defined (SWITCHING_TIME) */
							for (k=k2;k>0;k--)
							{
								temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							for (k=k2;k>0;k--)
							{
								temp_2=(float)(*hardware_buffer)-(*offset);
								temp += temp_2*temp_2;
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							qsort(sorted_signal,number_of_samples,sizeof(float),
								compare_float);
							temp=(float)0;
							for (k=(int)(switching_time*200*(float)number_of_samples);
								k<(int)number_of_samples;k++)
							{
								temp += sorted_signal[k];
							}
							temp /= (1-200*switching_time)*(float)number_of_samples;
#else /* defined (SWITCHING_TIME) */
							temp /= number_of_samples;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							temp /= number_of_samples;
							temp *= 2;
							temp=(float)sqrt((double)temp);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
							switch (calibrate_stage)
							{
								case 1:
								{
									*gain=(float)(*calibrate_amplitude)/temp;
								} break;
								case 2:
								{
									/* *gain is already the fixed gain */
									*gain=((float)(*calibrate_amplitude)/temp)/(*gain);
								} break;
								case 3:
								{
									/* *gain is already the programmable gain */
									*gain *= ((float)(*calibrate_amplitude)/temp);
								} break;
							}
						}
						gain++;
						offset++;
					}
#if defined (OLD_CODE)
					gain=gains;
					offset=offsets;
					ni_card=module_NI_CARDS;
					calibrate_amplitude=calibrate_amplitudes;
					for (i=module_number_of_NI_CARDS;i>0;i--)
					{
						number_of_configured_channels=
							ni_card->number_of_configured_channels;
						for (j=0;j<number_of_configured_channels;j++)
						{
							hardware_buffer=(ni_card->hardware_buffer)+
								(module_starting_sample_number*number_of_configured_channels+j);
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							sorted_signal_entry=sorted_signal;
							for (k=k1;k>0;k--)
							{
								temp=(float)(*hardware_buffer)-(*offset);
								if (temp<(float)0)
								{
									temp= -temp;
								}
								*sorted_signal_entry=temp;
								sorted_signal_entry++;
								hardware_buffer += number_of_configured_channels;
							}
#else /* defined (SWITCHING_TIME) */
							temp=(float)0;
							for (k=k1;k>0;k--)
							{
								temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							temp=(float)0;
							for (k=k1;k>0;k--)
							{
								temp_2=(float)(*hardware_buffer)-(*offset);
								temp += temp_2*temp_2;
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
							hardware_buffer=(ni_card->hardware_buffer)+j;
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							for (k=k2;k>0;k--)
							{
								temp=(float)(*hardware_buffer)-(*offset);
								if (temp<(float)0)
								{
									temp= -temp;
								}
								*sorted_signal_entry=temp;
								sorted_signal_entry++;
								hardware_buffer += number_of_configured_channels;
							}
#else /* defined (SWITCHING_TIME) */
							for (k=k2;k>0;k--)
							{
								temp += (float)fabs(((float)(*hardware_buffer)-(*offset)));
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							for (k=k2;k>0;k--)
							{
								temp_2=(float)(*hardware_buffer)-(*offset);
								temp += temp_2*temp_2;
								hardware_buffer += number_of_configured_channels;
							}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
#if defined (CALIBRATE_SIGNAL_SQUARE)
#if defined (SWITCHING_TIME)
							qsort(sorted_signal,number_of_samples,sizeof(float),
								compare_float);
							temp=(float)0;
							for (k=(int)(switching_time*200*(float)number_of_samples);
								k<(int)number_of_samples;k++)
							{
								temp += sorted_signal[k];
							}
							temp /= (1-200*switching_time)*(float)number_of_samples;
#else /* defined (SWITCHING_TIME) */
							temp /= number_of_samples;
#endif /* defined (SWITCHING_TIME) */
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
							temp /= number_of_samples;
							temp *= 2;
							temp=(float)sqrt((double)temp);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
							switch (calibrate_stage)
							{
								case 1:
								{
									*gain=(float)(*calibrate_amplitude)/temp;
								} break;
								case 2:
								{
									/* *gain is already the fixed gain */
									*gain=((float)(*calibrate_amplitude)/temp)/(*gain);
								} break;
								case 3:
								{
									/* *gain is already the programmable gain */
									*gain *= ((float)(*calibrate_amplitude)/temp);
								} break;
							}
							offset++;
							gain++;
						}
						calibrate_amplitude++;
						ni_card++;
					}
#endif /* defined (OLD_CODE) */
				}
				switch (calibrate_stage)
				{
					case 1:
					{
						/* signal conditioning fixed gain */
						/* set up for next stage */
						card_number=0;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						return_code=1;
						while (return_code&&(card_number<module_number_of_NI_CARDS))
						{
							if (0<ni_card->number_of_configured_channels)
							{
								channel_number=(ni_card->configured_channels)[0];
								/* determine the calibration voltage */
								/* start with full-range voltage */
								if (PXI6071E_AD_DA==ni_card->type)
								{
									*calibrate_amplitude=(float)5;
								}
								else
								{
									*calibrate_amplitude=(float)10;
								}
								/* use half-range */
								*calibrate_amplitude /= 2;
								switch (ni_card->unemap_hardware_version)
								{
									case UnEmap_1V2:
									{
										/* fixed gain of 10 */
										*calibrate_amplitude /= 10;
									} break;
									case UnEmap_2V1:
									case UnEmap_2V2:
									{
										/* fixed gain of 11 */
										*calibrate_amplitude /= 11;
										/* programmable gain */
										if ((ni_card->unemap_2vx).gain_output_line_0)
										{
											DIG_Out_Line(ni_card->device_number,(i16)0,
												gain_0_output_line_UnEmap2vx,(i16)1);
											*calibrate_amplitude /= 2;
										}
										if ((ni_card->unemap_2vx).gain_output_line_1)
										{
											DIG_Out_Line(ni_card->device_number,(i16)0,
												gain_1_output_line_UnEmap2vx,(i16)1);
											*calibrate_amplitude /= 4;
										}
									} break;
								}
#if defined (CALIBRATE_SIGNAL_SQUARE)
								calibrate_voltages[0]= *calibrate_amplitude;
								calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
								/* calculate cosine wave */
								two_pi=(double)8*atan((double)1);
								for (i=0;i<number_of_waveform_points;i++)
								{
									calibrate_voltages[i]=
										(*calibrate_amplitude)*(float)cos(two_pi*
										(double)i/(double)number_of_waveform_points);
								}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
								if (return_code=load_NI_DA(CALIBRATE_CHANNEL,1,&channel_number,
									number_of_waveform_points,(float)number_of_waveform_points*
									CALIBRATE_FREQUENCY,calibrate_voltages,0,
									(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
								{
									*calibrate_amplitude=calibrate_voltages[0];
									card_number++;
									ni_card++;
									calibrate_amplitude++;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  load_NI_DA failed 2");
									return_code=0;
								}
							}
							else
							{
								card_number++;
								ni_card++;
								calibrate_amplitude++;
							}
						}
						if (return_code&&(return_code=start_NI_DA()))
						{
							settling_step=0;
							calibrate_stage=2;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 2 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 2\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 2:
					{
						/* signal conditioning variable gain */
						/* set up for next stage */
						card_number=0;
						ni_card=module_NI_CARDS;
						calibrate_amplitude=calibrate_amplitudes;
						return_code=1;
						while (return_code&&(card_number<module_number_of_NI_CARDS))
						{
							if (0<ni_card->number_of_configured_channels)
							{
								channel_number=(ni_card->configured_channels)[0];
								/* set the gain */
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)0);
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)0);
								return_code=set_NI_gain(ni_card,(int)(ni_card->gain));
								if (return_code)
								{
									/* determine the calibration voltage */
									/* start with full-range voltage */
									if (PXI6071E_AD_DA==ni_card->type)
									{
										*calibrate_amplitude=(float)5;
									}
									else
									{
										*calibrate_amplitude=(float)10;
									}
									/* use half-range */
									*calibrate_amplitude /= 2;
									switch (ni_card->unemap_hardware_version)
									{
										case UnEmap_1V2:
										{
											/* fixed gain of 10 */
											*calibrate_amplitude /= 10;
										} break;
										case UnEmap_2V1:
										case UnEmap_2V2:
										{
											/* fixed gain of 11 */
											*calibrate_amplitude /= 11;
										} break;
									}
									/* NI gain of 11 */
									*calibrate_amplitude /= ni_card->gain;
#if defined (CALIBRATE_SIGNAL_SQUARE)
									calibrate_voltages[0]= *calibrate_amplitude;
									calibrate_voltages[1]= -(*calibrate_amplitude);
#else /* defined (CALIBRATE_SIGNAL_SQUARE) */
									/* calculate cosine wave */
									two_pi=(double)8*atan((double)1);
									for (i=0;i<number_of_waveform_points;i++)
									{
										calibrate_voltages[i]=
											(*calibrate_amplitude)*(float)cos(two_pi*
											(double)i/(double)number_of_waveform_points);
									}
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) */
									if (return_code=load_NI_DA(CALIBRATE_CHANNEL,1,
										&channel_number,number_of_waveform_points,
										(float)number_of_waveform_points*CALIBRATE_FREQUENCY,
										calibrate_voltages,0,
										(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
									{
										*calibrate_amplitude=calibrate_voltages[0];
										card_number++;
										ni_card++;
										calibrate_amplitude++;
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"calibration_callback.  load_NI_DA failed 3");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"calibration_callback.  set_NI_gain failed 2");
								}
							}
							else
							{
								card_number++;
								ni_card++;
								calibrate_amplitude++;
							}
						}
						if (return_code&&(return_code=start_NI_DA()))
						{
							settling_step=0;
							calibrate_stage=3;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 3 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 3\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 3:
					{
						/* NI gain */
						/* set up for next stage */
						return_code=stop_NI_DA(CALIBRATE_CHANNEL,0);
						ni_card=module_NI_CARDS;
						for (card_number=0;card_number<module_number_of_NI_CARDS;
							card_number++)
						{
							/* set the gain */
							if ((ni_card->unemap_2vx).gain_output_line_0)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_0_output_line_UnEmap2vx,(i16)1);
							}
							if ((ni_card->unemap_2vx).gain_output_line_1)
							{
								DIG_Out_Line(ni_card->device_number,(i16)0,
									gain_1_output_line_UnEmap2vx,(i16)1);
							}
							ni_card++;
						}
						if (return_code)
						{
							settling_step=0;
							calibrate_stage=4;
							return_code=unemap_start_sampling();
							display_message(INFORMATION_MESSAGE,
								"Calibration.  Phase 4 of 4\n");
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"start Phase 4\n");
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
						}
					} break;
					case 4:
					{
						/* offsets */
						calibrate_stage=0;
						module_buffer_full_callback=(Buffer_full_callback *)NULL;
						module_buffer_full_callback_data=(void *)NULL;
						module_buffer_full_size=0;
						stop_NI_DA(CALIBRATE_CHANNEL,0);
						ni_card=module_NI_CARDS;
						filter_frequency=filter_frequencies;
						for (i=module_number_of_NI_CARDS;i>0;i--)
						{
							if (0<ni_card->number_of_configured_channels)
							{
								channel_number=(ni_card->configured_channels)[0];
								unemap_set_antialiasing_filter_frequency(channel_number,
									*filter_frequency);
							}
							ni_card++;
							filter_frequency++;
						}
						return_code=1;
						if (module_calibration_end_callback)
						{
							if (REALLOCATE(calibration_channels,module_calibration_channels,
								int,number_of_settled_channels)&&REALLOCATE(calibration_offsets,
								module_calibration_offsets,float,number_of_settled_channels)&&
								REALLOCATE(calibration_gains,module_calibration_gains,float,
								number_of_settled_channels))
							{
								module_calibration_channels=calibration_channels;
								module_calibration_offsets=calibration_offsets;
								module_calibration_gains=calibration_gains;
								ni_card=module_NI_CARDS;
								l=0;
								k=0;
								for (i=module_number_of_NI_CARDS;i>0;i--)
								{
									if ((PCI6031E_AD_DA==ni_card->type)||
										(PXI6031E_AD_DA==ni_card->type)||
										(PXI6071E_AD_DA==ni_card->type))
									{
										number_of_configured_channels=
											ni_card->number_of_configured_channels;
										if (0<number_of_configured_channels)
										{
											for (j=0;j<number_of_configured_channels;j++)
											{
												if (!(channel_failed[l]&stage_mask))
												{
													module_calibration_offsets[k]=offsets[l];
													module_calibration_gains[k]=gains[l];
													module_calibration_channels[k]=
														(ni_card->configured_channels)[j];
													k++;
												}
												l++;
											}
										}
									}
									ni_card++;
								}
								module_calibration_number_of_channels=k;
								(*module_calibration_end_callback)(
									module_calibration_number_of_channels,
									module_calibration_channels,module_calibration_offsets,
									module_calibration_gains,
									module_calibration_end_callback_data);
							}
							else
							{
								display_message(ERROR_MESSAGE,"calibration_callback.  Could not"
									" reallocate calibration information.  %p %p %d",
									calibration_channels,calibration_offsets,calibration_gains);
								if (calibration_channels)
								{
									module_calibration_channels=calibration_channels;
									if (calibration_offsets)
									{
										module_calibration_offsets=calibration_offsets;
									}
								}
								(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
									(float *)NULL,module_calibration_end_callback_data);
							}
						}
						DEALLOCATE(calibrate_amplitudes);
						DEALLOCATE(calibrate_voltages);
						DEALLOCATE(filter_frequencies);
						DEALLOCATE(offsets);
						DEALLOCATE(previous_offsets);
						DEALLOCATE(gains);
						DEALLOCATE(channel_failed);
						DEALLOCATE(saturated);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
						DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
					} break;
				}
				stage_flag <<= 1;
				stage_mask <<= 1;
				stage_mask |= 0x1;
			}
			else
			{
				/* not settled */
				if (settling_step<settling_step_max)
				{
					/* try again */
					settling_step++;
					return_code=unemap_start_sampling();
				}
				else
				{
					/* give up */
					display_message(ERROR_MESSAGE,
						"calibration_callback.  Failed to settle for stage %d\n",
						calibrate_stage);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"Failed to settle for stage %d\n",calibrate_stage);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
					return_code=0;
				}
			}
		}
		else
		{
			settling_step=1;
			return_code=unemap_start_sampling();
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,"After checking settling %d\n",
			return_code);
#endif /* defined (DEBUG) */
		if (!return_code)
		{
			calibrate_stage=0;
			module_buffer_full_callback=(Buffer_full_callback *)NULL;
			module_buffer_full_callback_data=(void *)NULL;
			stop_NI_DA(CALIBRATE_CHANNEL,0);
			card_number=0;
			ni_card=module_NI_CARDS;
			filter_frequency=filter_frequencies;
			while (card_number<module_number_of_NI_CARDS)
			{
				if (0<ni_card->number_of_configured_channels)
				{
					channel_number=(ni_card->configured_channels)[0];
					unemap_set_antialiasing_filter_frequency(channel_number,
						*filter_frequency);
					set_NI_gain(ni_card,(int)(ni_card->gain));
					if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
						(UnEmap_2V2==ni_card->unemap_hardware_version))
					{
						/* programmable gain */
						if ((ni_card->unemap_2vx).gain_output_line_0)
						{
							DIG_Out_Line(ni_card->device_number,(i16)0,
								gain_0_output_line_UnEmap2vx,(i16)1);
						}
						if ((ni_card->unemap_2vx).gain_output_line_1)
						{
							DIG_Out_Line(ni_card->device_number,(i16)0,
								gain_1_output_line_UnEmap2vx,(i16)1);
						}
					}
				}
				card_number++;
				ni_card++;
				filter_frequency++;
			}
			DEALLOCATE(calibrate_amplitudes);
			DEALLOCATE(calibrate_voltages);
			DEALLOCATE(filter_frequencies);
			DEALLOCATE(offsets);
			DEALLOCATE(previous_offsets);
			DEALLOCATE(gains);
			DEALLOCATE(channel_failed);
			DEALLOCATE(saturated);
#if defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME)
			DEALLOCATE(sorted_signal);
#endif /* defined (CALIBRATE_SIGNAL_SQUARE) && defined (SWITCHING_TIME) */
			if (module_calibration_end_callback)
			{
				(*module_calibration_end_callback)(0,(int *)NULL,(float *)NULL,
					(float *)NULL,module_calibration_end_callback_data);
			}
		}
	}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"leave calibration_callback %d %d\n",calibrate_stage,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* calibration_callback */
#endif /* defined (NI_DAQ) */

#if defined (WIN32_SYSTEM)
#if !defined (NO_BATT_GOOD_WATCH)
DWORD WINAPI batt_good_thread_function(LPVOID stop_flag_address_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2006

DESCRIPTION :
==============================================================================*/
{
	DWORD return_code;
	int *stop_flag_address;
#if defined (NI_DAQ)
	i16 state;
#endif /* defined (NI_DAQ) */

	ENTER(batt_good_thread_function);
	return_code=0;
	if (stop_flag_address=(int *)stop_flag_address_void)
	{
#if defined (NI_DAQ)
		state=0;
		do
		{
			Sleep(500);
			if (module_NI_CARDS&&!(*stop_flag_address))
			{
				if ((0==DIG_In_Line(module_NI_CARDS->device_number,(i16)0,
					battery_good_input_line_UnEmap2vx,&state))&&!state)
				{
					display_message(INFORMATION_MESSAGE,
						"Setting power off because of BattGood line");
					unemap_set_power(0);
				}
			}
		} while (state&&module_NI_CARDS&&!(*stop_flag_address));
		*stop_flag_address=1;
#endif /* defined (NI_DAQ) */
	}
	LEAVE;

	return (return_code);
} /* batt_good_thread_function */
#endif /* !defined (NO_BATT_GOOD_WATCH) */
#endif /* defined (WIN32_SYSTEM) */

static int file_write_samples_acquired(short int *samples,int number_of_samples,
	void *file)
/*******************************************************************************
LAST MODIFIED : 12 November 2001

DESCRIPTION :
Used in conjunction with <unemap_transfer_samples_acquired> by
<unemap_write_samples_acquired>.
==============================================================================*/
{
	int return_code;
#if defined (WIN32_IO)
	DWORD number_of_bytes_written;
#endif /* defined (WIN32_IO) */

	ENTER(file_write_samples_acquired);
	return_code=0;
	if (samples&&(0<number_of_samples)&&file)
	{
#if defined (WIN32_IO)
		if (WriteFile((HANDLE)file,(LPCVOID)samples,(DWORD)(number_of_samples*
			sizeof(short int)),&number_of_bytes_written,(LPOVERLAPPED)NULL))
		{
			return_code=number_of_bytes_written/sizeof(short int);
		}
		else
		{
			return_code=0;
		}
#else /* defined (WIN32_IO) */
		return_code=fwrite((char *)samples,sizeof(short int),
			(size_t)number_of_samples,(FILE *)file);
#endif /* defined (WIN32_IO) */
	}
	LEAVE;

	return (return_code);
} /* file_write_samples_acquired */

static int memory_copy_samples_acquired(short int *samples,
	int number_of_samples,void *memory_address)
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Used in conjunction with <unemap_transfer_samples_acquired> by
<unemap_get_samples_acquired>.
==============================================================================*/
{
	int i,return_code;
	short int *destination,**destination_address,*source;

	ENTER(memory_copy_samples_acquired);
	return_code=0;
	if ((source=samples)&&(0<number_of_samples)&&
		(destination_address=(short int **)memory_address)&&
		(destination= *destination_address))
	{
		for (i=number_of_samples;i>0;i--)
		{
			*destination= *source;
			source++;
			destination++;
		}
		return_code=number_of_samples;
		*destination_address=destination;
	}
	LEAVE;

	return (return_code);
} /* memory_copy_samples_acquired */

/*
Global functions
----------------
*/
#if defined (STOP_WAVEFORM_IN_START_SAMPLING)
static int unemap_start_sampling_count=0;
#endif /* defined (STOP_WAVEFORM_IN_START_SAMPLING) */

int unemap_configure(int number_of_channels,int *channel_numbers,
	float sampling_frequency_slave,int number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
	HWND scrolling_window,UINT scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
#if defined (UNIX)
	struct Event_dispatcher *event_dispatcher,
#endif /* defined (UNIX) */
	Unemap_hardware_callback *scrolling_callback,void *scrolling_callback_data,
	float scrolling_frequency,float scrolling_callback_frequency,
	int synchronization_card)
/*******************************************************************************
LAST MODIFIED : 18 August 2003

DESCRIPTION :
Configures the hardware for sampling the specified channels
(<number_of_channels> and <channel_numbers>) at the specified
<sampling_frequency> and with the specified <number_of_samples_in_buffer>.
<sampling_frequency>, <number_of_samples_in_buffer>, <scrolling_frequency> and
<scrolling_callback_frequency> are requests and the actual values used should
be determined using unemap_get_sampling_frequency,
unemap_get_maximum_number_of_samples, unemap_get_scrolling_frequency and
unemap_get_scrolling_callback_frequency.

If <number_of_channels> is
- -1 then the hardware is configured with no channels
- 0 then all channels are configured
- otherwise, the <number_of_channels> listed in <channel_numbers> are
	configured.

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

For this local hardware version, the sign of <sampling_frequency_slave>
determines whether the hardware is configured as slave (<0) or master (>0)

???DB.  Have to restart hardware service if change synchronization_card
==============================================================================*/
{
	float sampling_frequency;
	int return_code;
#if defined (NI_DAQ)
	int all_12_bit_ADs,i,j,k,local_synchronization_card;
	i16 gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],
		maximum_number_of_scanned_channels_for_card,number_of_scanned_channels,
		scan_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],status,time_base,
		total_number_of_scanned_channels;
	u16 sampling_interval;
	u32 available_physical_memory,desired_physical_memory,hardware_buffer_size,
		physical_memory_tolerance;
#if defined (WIN32_SYSTEM)
	/* for getting the total physical memory */
	MEMORYSTATUS memory_status;
#if defined (USE_VIRTUAL_LOCK)
	SIZE_T working_set_size;
#endif /* defined (USE_VIRTUAL_LOCK) */
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX) && defined (UNEMAP_THREAD_SAFE)
	int error_code;
#endif /* defined (UNIX) && defined (UNEMAP_THREAD_SAFE) */
#endif /* defined (NI_DAQ) */

	ENTER(unemap_configure);
#if defined (UNIX)
	USE_PARAMETER(event_dispatcher);
#endif /* defined (UNIX) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_configure %g %d "
#if defined (WIN32_USER_INTERFACE)
		"%p %u "
#endif /* defined (WIN32_USER_INTERFACE) */
		"%p %p %g %g %d %d\n",sampling_frequency_slave,number_of_samples_in_buffer,
#if defined (WIN32_USER_INTERFACE)
		scrolling_window,scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
		scrolling_callback,scrolling_callback_data,scrolling_frequency,
		scrolling_callback_frequency,synchronization_card,number_of_channels);
#endif /* defined (DEBUG) */
#if defined (STOP_WAVEFORM_IN_START_SAMPLING)
	unemap_start_sampling_count=0;
#endif /* defined (STOP_WAVEFORM_IN_START_SAMPLING) */
	return_code=0;
	if (sampling_frequency_slave<0)
	{
		sampling_frequency= -sampling_frequency_slave;
		module_slave=1;
	}
	else
	{
		sampling_frequency=sampling_frequency_slave;
		module_slave=0;
	}
	/* check arguments */
	if (((-1==number_of_channels)||(0==number_of_channels)||
		((0<number_of_channels)&&channel_numbers))&&
		(0<sampling_frequency)&&(0<number_of_samples_in_buffer)&&
		(!scrolling_callback||((0<scrolling_callback_frequency)&&
		(scrolling_frequency>=scrolling_callback_frequency))))
	{
#if defined (NI_DAQ)
		if (0==module_configured)
		{
			if (search_for_NI_cards()&&module_NI_CARDS)
			{
				return_code=1;
				/* set up channels to be scanned/configured */
				if (-1==number_of_channels)
				{
					module_number_of_configured_channels=0;
					module_configured_channels=(int *)NULL;
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						module_NI_CARDS[i].number_of_configured_channels=
							NUMBER_OF_CHANNELS_ON_NI_CARD;
						for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
						{
							(module_NI_CARDS[i].hardware_buffer_offsets)[j]= -1;
						}
					}
				}
				else if (0==number_of_channels)
				{
					if (ALLOCATE(module_configured_channels,int,module_number_of_NI_CARDS*
						NUMBER_OF_CHANNELS_ON_NI_CARD))
					{
						module_number_of_configured_channels=module_number_of_NI_CARDS*
							NUMBER_OF_CHANNELS_ON_NI_CARD;
						k=1;
						for (i=0;i<module_number_of_NI_CARDS;i++)
						{
							module_NI_CARDS[i].number_of_configured_channels=
								NUMBER_OF_CHANNELS_ON_NI_CARD;
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								module_configured_channels[k-1]=k;
								(module_NI_CARDS[i].configured_channels)[j]=k;
								(module_NI_CARDS[i].hardware_buffer_offsets)[j]=j;
								k++;
							}
						}
						display_message(INFORMATION_MESSAGE,"unemap_configure.  "
							"Set up configured channels for number_of_channels=0.  "
							"%d %d\n",module_number_of_configured_channels,k);
					}
					else
					{
						display_message(ERROR_MESSAGE,"unemap_configure.  "
							"Could not allocated module_configured_channels %d",
							module_number_of_NI_CARDS*NUMBER_OF_CHANNELS_ON_NI_CARD);
						return_code=0;
					}
				}
				else
				{
					if (ALLOCATE(module_configured_channels,int,number_of_channels))
					{
						module_number_of_configured_channels=number_of_channels;
						for (i=0;i<module_number_of_NI_CARDS;i++)
						{
							module_NI_CARDS[i].number_of_configured_channels=0;
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								(module_NI_CARDS[i].hardware_buffer_offsets)[j]= -1;
							}
						}
						i=0;
						while ((i<number_of_channels)&&(0<channel_numbers[i])&&
							((j=(channel_numbers[i]-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
							module_number_of_NI_CARDS))
						{
							k=0;
							while ((k<module_NI_CARDS[j].number_of_configured_channels)&&
								(channel_numbers[i]!=
								(module_NI_CARDS[j].configured_channels)[k]))
							{
								k++;
							}
							if (k==module_NI_CARDS[j].number_of_configured_channels)
							{
								(module_NI_CARDS[j].configured_channels)[module_NI_CARDS[j].
									number_of_configured_channels]=channel_numbers[i];
								(module_NI_CARDS[j].hardware_buffer_offsets)[
									(channel_numbers[i]-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]=
									module_NI_CARDS[j].number_of_configured_channels;
								(module_NI_CARDS[j].number_of_configured_channels)++;
							}
							module_configured_channels[i]=channel_numbers[i];
							i++;
						}
						if (i!=number_of_channels)
						{
							display_message(ERROR_MESSAGE,"unemap_configure.  "
								"Invalid channel number %d",channel_numbers[i]);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"unemap_configure.  "
							"Could not allocated module_configured_channels %d",
							number_of_channels);
						return_code=0;
					}
				}
				maximum_number_of_scanned_channels_for_card=1;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					if (maximum_number_of_scanned_channels_for_card<
						module_NI_CARDS[i].number_of_configured_channels)
					{
						maximum_number_of_scanned_channels_for_card=
							module_NI_CARDS[i].number_of_configured_channels;
					}
				}
				if (return_code)
				{
					total_number_of_scanned_channels=0;
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						if (0<module_NI_CARDS[i].number_of_configured_channels)
						{
							total_number_of_scanned_channels +=
								module_NI_CARDS[i].number_of_configured_channels;
						}
						else
						{
							total_number_of_scanned_channels++;
						}
					}
					if (synchronization_card<1)
					{
						local_synchronization_card=0;
					}
					else
					{
						if (module_number_of_NI_CARDS<synchronization_card)
						{
							local_synchronization_card=module_number_of_NI_CARDS-1;
						}
						else
						{
							local_synchronization_card=synchronization_card-1;
						}
					}
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"synchronization_card=%d, local_synchronization_card=%d\n",
						synchronization_card,local_synchronization_card);
#endif /* defined (DEBUG) */
					/* drive the A/D conversions as fast as possible */
					/* use the 20 MHz (50 ns) clock */
					time_base= -3;
#if defined (OLD_CODE)
/*???DB.  Was to allow specifying in settings file, but not used now because of
	mixing 12/16-bit and because of sampling on a subset */
					sampling_interval=module_NI_CARDS->sampling_interval;
					if (sampling_interval<=0)
					{
#endif /* defined (OLD_CODE) */
						all_12_bit_ADs=1;
						i=0;
						while (all_12_bit_ADs&&(i<module_number_of_NI_CARDS))
						{
							if (PXI6071E_AD_DA!=module_NI_CARDS[i].type)
							{
								all_12_bit_ADs=0;
							}
							i++;
						}
						if (all_12_bit_ADs)
						{
							if (1==maximum_number_of_scanned_channels_for_card)
							{
								/* 1.25 MS/s */
								/* do a A/D conversion every 800 nano-seconds = 16 * 50 ns */
								sampling_interval=16;
							}
							else
							{
								/* if scanning can't go at 1.25 MS/s, limit to 0.5 MS/s */
								sampling_interval=40;
							}
						}
						else
						{
							/* 100 kS/s */
							/* do a A/D conversion every 10 micro-seconds = 200 * 50 ns */
							sampling_interval=200;
							/*???DB.  What is safe when scanning ? */
							/*???DB.  Because sampling slower, probably OK at 100kS/s */
						}
#if defined (OLD_CODE)
					}
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
					module_sampling_frequency=(float)1;
					for (i=(int)ceil(log10(
						(double)maximum_number_of_scanned_channels_for_card));i>0;i--)
					{
						module_sampling_frequency *= (float)10;
					}
					module_sampling_frequency=(float)1e9/((float)sampling_interval*
						(float)50*module_sampling_frequency);
#endif /* defined (OLD_CODE) */
					module_sampling_frequency=(float)1e9/((float)sampling_interval*
						(float)(50*maximum_number_of_scanned_channels_for_card));
					if ((0<sampling_frequency)&&
						(sampling_frequency<module_sampling_frequency))
					{
						module_sampling_frequency=sampling_frequency;
					}
					/*???debug */
					display_message(INFORMATION_MESSAGE,
						"module_sampling_frequency=%g, sampling_interval=%u, "
						"maximum_number_of_scanned_channels_for_card=%d\n",
						module_sampling_frequency,sampling_interval,
						maximum_number_of_scanned_channels_for_card);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
					module_sampling_high_count=2;
					module_sampling_low_count=
						(u32)((double)20000000/(double)module_sampling_frequency);
					module_sampling_frequency=
						(float)20000000./(float)module_sampling_low_count;
					module_sampling_low_count -= module_sampling_high_count;
					if ((module_sampling_low_count>0)&&(module_sampling_high_count>0))
					{
#if defined (UNEMAP_THREAD_SAFE)
#if defined (WIN32_SYSTEM)
						if (scrolling_callback_mutex=CreateMutex(
							/*no security attributes*/NULL,/*do not initially own*/FALSE,
							/*no name*/(LPCTSTR)NULL))
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
						if (0==(error_code=pthread_mutex_init(
							&(scrolling_callback_mutex_storage),
							(pthread_mutexattr_t *)NULL)))
#endif /* defined (UNIX) */
						{
#if defined (UNIX)
							scrolling_callback_mutex=
								&(scrolling_callback_mutex_storage);
#endif /* defined (UNIX) */
#endif /* defined (UNEMAP_THREAD_SAFE) */
						if ((0<scrolling_callback_frequency)&&(scrolling_callback
#if defined (WIN32_USER_INTERFACE)
							||scrolling_window
#endif /* defined (WIN32_USER_INTERFACE) */
							))
						{
							module_scrolling_callback_period=(int)(module_sampling_frequency/
								scrolling_callback_frequency+0.5);
							module_number_of_scrolling_values_per_channel=
								(int)(scrolling_frequency/scrolling_callback_frequency+0.5);
							/* have scrolling period divisible by 
								module_number_of_scrolling_values_per_channel */
							module_scrolling_callback_period=
								(module_scrolling_callback_period-1)/
								module_number_of_scrolling_values_per_channel;
							module_scrolling_callback_period=
								(module_scrolling_callback_period+1)*
								module_number_of_scrolling_values_per_channel;
#if defined (WIN32_USER_INTERFACE)
							module_scrolling_window=scrolling_window;
							module_scrolling_message=scrolling_message;
#endif /* defined (WIN32_USER_INTERFACE) */
							module_scrolling_callback=scrolling_callback;
							module_scrolling_callback_data=scrolling_callback_data;
						}
						else
						{
							/* have a callback to keep <module_starting_sample_number> and
								<module_sample_buffer_size> up to date */
							module_scrolling_callback_period=number_of_samples_in_buffer/2;
							module_number_of_scrolling_values_per_channel=1;
#if defined (WIN32_USER_INTERFACE)
							module_scrolling_window=(HWND)NULL;
							module_scrolling_message=(UINT)0;
#endif /* defined (WIN32_USER_INTERFACE) */
							module_scrolling_callback=(Unemap_hardware_callback *)NULL;
							module_scrolling_callback_data=(void *)NULL;
						}
						module_scrolling_channel_numbers=(int *)NULL;
						module_number_of_scrolling_channels=0;
#if !defined (NO_SCROLLING_CALLBACK)
						status=Config_DAQ_Event_Message(module_NI_CARDS[0].device_number,
							/* add message */(i16)1,/* channel string */"AI0",
							/* send message every N scans */(i16)1,
							(i32)module_scrolling_callback_period,(i32)0,(u32)0,(u32)0,(u32)0,
							(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,"unemap_configure.  "
							"Config_DAQ_Event_Message(%d,1,AI0,1,%d,0,0,0,0,NULL,0,%p)=%d\n",
							module_NI_CARDS[0].device_number,module_scrolling_callback_period,
							scrolling_callback_NI,status);
#endif /* defined (DEBUG) */
#else /* !defined (NO_SCROLLING_CALLBACK) */
						status=0;
#endif /* !defined (NO_SCROLLING_CALLBACK) */
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,
							"module_sampling_frequency=%g, sampling_interval=%u\n",
							module_sampling_frequency,sampling_interval);
#endif /* defined (DEBUG) */
						if (0!=status)
						{
							display_message(ERROR_MESSAGE,
								"unemap_configure.  Config_DAQ_Event_Message 1 failed.  %d %d",
								status,module_scrolling_callback_period);
						}
#if defined (WIN32_SYSTEM)
						hardware_buffer_size=(u32)number_of_samples_in_buffer*
							(u32)total_number_of_scanned_channels;
						memory_status.dwLength=sizeof(MEMORYSTATUS);
						GlobalMemoryStatus(&memory_status);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"total physical memory=%d, available physical memory=%d\n",
			memory_status.dwTotalPhys,memory_status.dwAvailPhys);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
/*???DB.  16 November 2001.  Works (?).  See Martyn's email */
						/* limit the total locked memory to a third of physical memory */
						if ((DWORD)(3*hardware_buffer_size)*sizeof(i16)>
							memory_status.dwTotalPhys)
						{
							hardware_buffer_size=(u32)(memory_status.dwTotalPhys/
								(3*sizeof(i16)));
						}
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
/*???DB.  16 November 2001.  Doesn't work.  See Martyn's email */
						/* limit the total locked memory to a "half" of available physical
							memory */
						if ((DWORD)((u32)(2.2*(float)hardware_buffer_size))*sizeof(i16)>
							memory_status.dwAvailPhys)
						{
							hardware_buffer_size=(u32)((float)memory_status.dwAvailPhys/
								(2.2*(float)sizeof(i16)));
						}
#endif /* defined (OLD_CODE) */
#if defined (OLD_CODE)
/*???DB.  17 November 2001.  Works with os_memory set to 25.  See Martyn's
	email */
						/* limit the total locked memory */
						{
							char *os_memory_string;
							u32 os_memory;

							if ((os_memory_string=getenv("UNEMAP_OS_MEMORY_MB"))&&
								(1==sscanf(os_memory_string,"%d",&os_memory)))
							{
								os_memory *= 1024*1024;
							}
							else
							{
								os_memory=(memory_status.dwTotalPhys)/3;
							}
							if ((DWORD)(2*hardware_buffer_size)*sizeof(i16)>
								memory_status.dwTotalPhys-os_memory)
							{
								hardware_buffer_size=
									(u32)((memory_status.dwTotalPhys-os_memory)/(2*sizeof(i16)));
							}
						}
#endif /* defined (OLD_CODE) */
						/* limit the total locked memory */
						{
							char *os_memory_string;
							u32 os_memory;

							if ((os_memory_string=getenv("UNEMAP_OS_MEMORY_MB"))&&
								(1==sscanf(os_memory_string,"%d",&os_memory)))
							{
								if (os_memory<=0)
								{
									/*???DB.  memory_status.dwAvailPhys is not correct, so use
										bi-section (later) to get most possible */
/*									os_memory=
										memory_status.dwTotalPhys-memory_status.dwAvailPhys;*/
									os_memory=0;
								}
								else
								{
									os_memory *= 1024*1024;
								}
							}
							else
							{
								os_memory=(memory_status.dwTotalPhys)/3;
							}
							if ((DWORD)(hardware_buffer_size)*
								sizeof(i16)>memory_status.dwTotalPhys-os_memory)
							{
								hardware_buffer_size=
									(u32)((memory_status.dwTotalPhys-os_memory)/sizeof(i16));
							}
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
#if defined (USE_VIRTUAL_LOCK)
	SIZE_T maximum,minimum;
#endif /* defined (USE_VIRTUAL_LOCK) */

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"total physical memory=%d, available physical memory=%d\n",
			memory_status.dwTotalPhys,memory_status.dwAvailPhys);
		fprintf(unemap_debug,
			"os_memory=%d, hardware_buffer_size=%d, total buffer memory=%d\n",
			os_memory,hardware_buffer_size,hardware_buffer_size*sizeof(i16));
#if defined (USE_VIRTUAL_LOCK)
		GetProcessWorkingSetSize(GetCurrentProcess(),&minimum,&maximum);
		fprintf(unemap_debug,
			"minimum working set=%d, maximum working set=%d\n",
			minimum,maximum);
#endif /* defined (USE_VIRTUAL_LOCK) */
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
#if defined (USE_VIRTUAL_LOCK)
						working_set_size=(11*memory_status.dwAvailPhys)/10;
						SetProcessWorkingSetSize(GetCurrentProcess(),working_set_size,
							working_set_size);
#endif /* defined (USE_VIRTUAL_LOCK) */
#if defined (DEBUG)
/*???debug */
#if defined (USE_VIRTUAL_LOCK)
{
	FILE *unemap_debug;
	SIZE_T maximum,minimum;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		GetProcessWorkingSetSize(GetCurrentProcess(),&minimum,&maximum);
		fprintf(unemap_debug,
			"minimum working set=%d, maximum working set=%d\n",
			minimum,maximum);
		fclose(unemap_debug);
	}
}
#endif /* defined (USE_VIRTUAL_LOCK) */
#endif /* defined (DEBUG) */
						}
#endif /* defined (WIN32_SYSTEM) */
						hardware_buffer_size /= total_number_of_scanned_channels;
						/* have number_of_samples_in_buffer divisible by
							module_scrolling_callback_period */
						hardware_buffer_size /= module_scrolling_callback_period;
						hardware_buffer_size *= module_scrolling_callback_period;
						/* configure cards */
						physical_memory_tolerance=1024*1024;
						available_physical_memory=0;
						desired_physical_memory=hardware_buffer_size*
							total_number_of_scanned_channels*sizeof(i16);
						do
						{
							/*???debug */
							display_message(INFORMATION_MESSAGE,
								"available_physical_memory=%lu, desired_physical_memory=%lu\n",
								available_physical_memory,desired_physical_memory);
#if defined (DEBUG)
#endif /* defined (DEBUG) */
							i=0;
							status=0;
							while ((0==status)&&(i<module_number_of_NI_CARDS))
							{
								(module_NI_CARDS[i]).time_base=time_base;
								(module_NI_CARDS[i]).sampling_interval=sampling_interval;
								if (0<(module_NI_CARDS[i]).number_of_configured_channels)
								{
									(module_NI_CARDS[i]).hardware_buffer_size=
										hardware_buffer_size*
										((module_NI_CARDS[i]).number_of_configured_channels);
								}
								else
								{
									(module_NI_CARDS[i]).hardware_buffer_size=
										hardware_buffer_size;
								}
								/* allocate buffer */
#if defined (USE_VIRTUAL_LOCK)
								if ((module_NI_CARDS[i]).hardware_buffer=VirtualAlloc(
									(LPVOID)NULL,(SIZE_T)(((module_NI_CARDS[i]).
									hardware_buffer_size)*sizeof(i16)),(DWORD)MEM_COMMIT,
									(DWORD)PAGE_READWRITE))
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_GMEM_FIXED)
								if ((module_NI_CARDS[i]).memory_object=GlobalAlloc(GMEM_FIXED,
									(DWORD)(((module_NI_CARDS[i]).hardware_buffer_size)*
									sizeof(i16))))
#else /* defined (USE_GMEM_FIXED) */
#if defined (USE_MALLOC)
								if ((module_NI_CARDS[i]).memory_object=
									malloc(((module_NI_CARDS[i]).hardware_buffer_size)*
									sizeof(i16)))
#else /* defined (USE_MALLOC) */
								if ((module_NI_CARDS[i]).memory_object=
									GlobalAlloc(GMEM_MOVEABLE,
									(DWORD)(((module_NI_CARDS[i]).hardware_buffer_size)*
									sizeof(i16))))
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_GMEM_FIXED) */
#endif /* defined (USE_VIRTUAL_LOCK) */
								{
#if defined (USE_VIRTUAL_LOCK)
									if (TRUE==VirtualLock((module_NI_CARDS[i]).hardware_buffer,
										(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
										sizeof(i16))))
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_GMEM_FIXED)
									if ((module_NI_CARDS[i]).hardware_buffer=
										(i16 *)((module_NI_CARDS[i]).memory_object))
#else /* defined (USE_GMEM_FIXED) */
#if defined (USE_MALLOC)
									if ((module_NI_CARDS[i]).hardware_buffer=
										(i16 *)((module_NI_CARDS[i]).memory_object))
#else /* defined (USE_MALLOC) */
									if ((module_NI_CARDS[i]).hardware_buffer=
										(i16 *)GlobalLock((module_NI_CARDS[i]).memory_object))
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_GMEM_FIXED) */
#endif /* defined (USE_VIRTUAL_LOCK) */
									{
										/* working from "Building Blocks" section, p.3-25 (pdf 65)
											in the "NI-DAQ User Manual" */
										/* configuration block.  Start with default settings except
											for changing to double buffering mode (DAQ_DB_Config).
											Could also use AI_Configure, AI_Max_Config, DAQ_Config,
											DAQ_StopTrigger_Config */
										status=DAQ_DB_Config((module_NI_CARDS[i]).device_number,
											/* enable */(i16)1);
										if (0==status)
										{
											/* set the scan sequence and gain */
											if (0<(module_NI_CARDS[i]).number_of_configured_channels)
											{
												number_of_scanned_channels=
													(module_NI_CARDS[i]).number_of_configured_channels;
												for (j=0;j<number_of_scanned_channels;j++)
												{
													gain_vector[j]=(module_NI_CARDS[i]).gain;
													scan_vector[j]=((module_NI_CARDS[i]).channel_reorder)[
														(((module_NI_CARDS[i]).configured_channels)[j]-1)%
														NUMBER_OF_CHANNELS_ON_NI_CARD];
												}
											}
											else
											{
												number_of_scanned_channels=1;
												gain_vector[0]=(module_NI_CARDS[i]).gain;
												scan_vector[0]=0;
											}
											status=SCAN_Setup((module_NI_CARDS[i]).device_number,
												number_of_scanned_channels,scan_vector,gain_vector);
											if (0==status)
											{
												if (0==i)
												{
													/* set the clock source on the RTSI bus */
													status=Select_Signal(
														(module_NI_CARDS[i]).device_number,ND_RTSI_CLOCK,
														ND_BOARD_CLOCK,ND_DONT_CARE);
													if (0!=status)
													{
														display_message(ERROR_MESSAGE,"unemap_configure.  "
															"Select_Signal(%d,ND_RTSI_CLOCK,ND_BOARD_CLOCK,"
															"ND_DONT_CARE)=%d.  %d",
															(module_NI_CARDS[i]).device_number,status,i);
													}
												}
												else
												{
													/* use the clock source on the RTSI bus */
													status=Select_Signal(
														(module_NI_CARDS[i]).device_number,ND_BOARD_CLOCK,
														ND_RTSI_CLOCK,ND_DONT_CARE);
													if (0!=status)
													{
														display_message(ERROR_MESSAGE,"unemap_configure.  "
															"Select_Signal(%d,ND_BOARD_CLOCK,ND_RTSI_CLOCK,"
															"ND_DONT_CARE)=%d.  %d",
															(module_NI_CARDS[i]).device_number,status,i);
													}
												}
												if (0==status)
												{
													if (module_slave&&(local_synchronization_card==i))
													{
														/* set to input the A/D conversion signal */
														status=Select_Signal(
															(module_NI_CARDS[i]).device_number,
															ND_IN_SCAN_START,ND_PFI_7,ND_LOW_TO_HIGH);
													}
													else
													{
														/* set to output the A/D conversion signal (for use
															on other crates */
														status=Select_Signal(
															(module_NI_CARDS[i]).device_number,ND_PFI_7,
															ND_IN_SCAN_IN_PROG,ND_LOW_TO_HIGH);
													}
													if (0==status)
													{
														/* configure RTSI */
														/* set to input the A/D conversion signal */
														if (module_slave&&(local_synchronization_card==i))
														{
															status=Select_Signal(
																(module_NI_CARDS[i]).device_number,
																ND_RTSI_0,ND_IN_SCAN_START,ND_LOW_TO_HIGH);
														}
														else
														{
															status=Select_Signal(
																(module_NI_CARDS[i]).device_number,
																ND_IN_SCAN_START,ND_RTSI_0,ND_LOW_TO_HIGH);
														}
														if (0==status)
														{
															/* acquisition won't actually start (controlled by
																conversion signal) */
															status=SCAN_Start(
																(module_NI_CARDS[i]).device_number,
																(i16 *)((module_NI_CARDS[i]).hardware_buffer),
																(u32)((module_NI_CARDS[i]).
																hardware_buffer_size),
																(module_NI_CARDS[i]).time_base,
																(module_NI_CARDS[i]).sampling_interval,(i16)0,
																(u16)0);
															if (0!=status)
															{
																display_message(ERROR_MESSAGE,
																	"unemap_configure.  "
																	"SCAN_Start failed.  %d.  %d.  %lu",status,i,
																	(u32)((module_NI_CARDS[i]).
																	hardware_buffer_size));
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"unemap_configure.  "
																"Select_Signal(%d,ND_IN_SCAN_START,"
																"ND_PFI_7/ND_RTSI_0,ND_LOW_TO_HIGH)=%d.  %d",
																(module_NI_CARDS[i]).device_number,status,i);
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,"unemap_configure.  "
															"Select_Signal(%d,ND_PFI_7,ND_IN_SCAN_START,"
															"ND_LOW_TO_HIGH)=%d",
															(module_NI_CARDS[i]).device_number,status);
													}
#if defined (OLD_CODE)
													/* set to output the A/D conversion signal (for use
														on other crates */
													if (!module_slave)
													{
														status=Select_Signal(
															(module_NI_CARDS[i]).device_number,ND_PFI_7,
															ND_IN_SCAN_IN_PROG,ND_LOW_TO_HIGH);
													}
													if (0==status)
													{
														/* configure RTSI */
														/* set to input the A/D conversion signal */
														if (module_slave&&(0==i))
														{
															status=Select_Signal(
																(module_NI_CARDS[i]).device_number,
																ND_IN_SCAN_START,ND_PFI_7,ND_LOW_TO_HIGH);
														}
														else
														{
															status=Select_Signal(
																(module_NI_CARDS[i]).device_number,
																ND_IN_SCAN_START,ND_RTSI_0,ND_LOW_TO_HIGH);
														}
														if (0==status)
														{
															/* acquisition won't actually start (controlled by
																conversion signal) */
															status=SCAN_Start(
																(module_NI_CARDS[i]).device_number,
																(i16 *)((module_NI_CARDS[i]).hardware_buffer),
																(u32)((module_NI_CARDS[i]).
																hardware_buffer_size),
																(module_NI_CARDS[i]).time_base,
																(module_NI_CARDS[i]).sampling_interval,(i16)0,
																(u16)0);
															if (0!=status)
															{
																display_message(ERROR_MESSAGE,
																	"unemap_configure.  SCAN_Start failed.  %d",
																	status);
															}
														}
														else
														{
															display_message(ERROR_MESSAGE,
																"unemap_configure.  "
																"Select_Signal(%d,ND_IN_SCAN_START,"
																"ND_PFI_7/ND_RTSI_0,ND_LOW_TO_HIGH)=%d",
																(module_NI_CARDS[i]).device_number,status);
														}
													}
													else
													{
														display_message(ERROR_MESSAGE,"unemap_configure.  "
															"Select_Signal(%d,ND_PFI_7,ND_IN_SCAN_START,"
															"ND_LOW_TO_HIGH)=%d",
															(module_NI_CARDS[i]).device_number,status);
													}
#endif /* defined (OLD_CODE) */
												}
												if (0==status)
												{
													i++;
												}
												else
												{
#if defined (USE_VIRTUAL_LOCK)
													VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
														(SIZE_T)(((module_NI_CARDS[i]).
														hardware_buffer_size)*sizeof(i16)));
													VirtualFree((module_NI_CARDS[i]).hardware_buffer,
														(SIZE_T)(((module_NI_CARDS[i]).
														hardware_buffer_size)*sizeof(i16)),
														(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
													free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
													GlobalUnlock((module_NI_CARDS[i]).memory_object);
													GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
													DAQ_DB_Config((module_NI_CARDS[i]).device_number,
														/* disable */(i16)0);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,"unemap_configure.  "
													"SCAN_Setup failed.  %d.  gain=%d.  %d",status,
													gain_vector[0],i);
#if defined (USE_VIRTUAL_LOCK)
												VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
													(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
													sizeof(i16)));
												VirtualFree((module_NI_CARDS[i]).hardware_buffer,
													(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
													sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
												free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
												GlobalUnlock((module_NI_CARDS[i]).memory_object);
												GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
												DAQ_DB_Config((module_NI_CARDS[i]).device_number,
													/* disable */(i16)0);
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,"unemap_configure.  "
												"DAQ_DB_Config failed.  %d.  %d",status,i);
#if defined (USE_VIRTUAL_LOCK)
											VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
												(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
												sizeof(i16)));
											VirtualFree((module_NI_CARDS[i]).hardware_buffer,
												(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
												sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
											free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
											GlobalUnlock((module_NI_CARDS[i]).memory_object);
											GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
#if defined (USE_VIRTUAL_LOCK)
											"unemap_configure.  VirtualLock failed.  %d",i);
										VirtualFree((module_NI_CARDS[i]).hardware_buffer,
											(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
											sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
											"unemap_configure.  GlobalLock failed.  %d",i);
#if defined (USE_MALLOC)
										free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
										GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
										status=1;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
#if defined (USE_VIRTUAL_LOCK)
										"unemap_configure.  VirtualAlloc failed.  %d",i);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
										"unemap_configure.  malloc failed.  %d",i);
#else /* defined (USE_MALLOC) */
										"unemap_configure.  GlobalAlloc failed.  %d",i);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
									status=1;
								}
							}
							if (0==status)
							{
								available_physical_memory=hardware_buffer_size*
									total_number_of_scanned_channels*sizeof(i16);
							}
							else
							{
								desired_physical_memory=hardware_buffer_size*
									total_number_of_scanned_channels*sizeof(i16);
							}
							if ((0!=status)||
								(desired_physical_memory-available_physical_memory>
								physical_memory_tolerance))
							{
								if (desired_physical_memory-available_physical_memory>
									physical_memory_tolerance)
								{
									hardware_buffer_size=(available_physical_memory+
										desired_physical_memory)/(2*sizeof(i16));
								}
								else
								{
									hardware_buffer_size=available_physical_memory/
										(sizeof(i16));
								}
								hardware_buffer_size /= total_number_of_scanned_channels;
								/* have number_of_samples_in_buffer divisible by
									module_scrolling_callback_period */
								hardware_buffer_size /= module_scrolling_callback_period;
								hardware_buffer_size *= module_scrolling_callback_period;
								while (i>0)
								{
									i--;
									DAQ_Clear((module_NI_CARDS[i]).device_number);
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,"unemap_configure.  "
										"DAQ_Clear(%d)=%d\n",(module_NI_CARDS[i]).device_number,
										status);
#endif /* defined (DEBUG) */
#if defined (USE_VIRTUAL_LOCK)
									VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
										(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
										sizeof(i16)));
									VirtualFree((module_NI_CARDS[i]).hardware_buffer,
										(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
										sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
									free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
									GlobalUnlock((module_NI_CARDS[i]).memory_object);
									GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
									DAQ_DB_Config((module_NI_CARDS[i]).device_number,
									/* disable */(i16)0);
								}
							}
						} while ((0<hardware_buffer_size)&&
							((desired_physical_memory-available_physical_memory>
							physical_memory_tolerance)||(0!=status)));
						module_sample_buffer_size=0;
						module_starting_sample_number=0;
						if (0==status)
						{
							return_code=1;
							/*???DB.  Start USE_INTERRUPTS_FOR_AI */
							/*???DB.  Needed, otherwise WFM_Op (waveform generation) fails */
							/*???DB.  Don't understand why.  See set_NI_gain */
							/*???DB.  Doesn't affect acquisition because after SCAN_Start */
#if !defined (NO_SCROLLING_CALLBACK)
							status=Config_DAQ_Event_Message(module_NI_CARDS[0].device_number,
								/* clear all messages */(i16)0,/* channel string */(char *)NULL,
								/* send message every N scans */(i16)0,
								(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,
								(HWND)NULL,(i16)0,(u32)0);
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,"unemap_configure 2.  "
								"Config_DAQ_Event_Message(%d,0,NULL,0,0,0,0,0,0,NULL,0,0)=%d\n",
								module_NI_CARDS[0].device_number,status);
#endif /* defined (DEBUG) */
#endif /* !defined (NO_SCROLLING_CALLBACK) */
#if !defined (USE_INTERRUPTS_FOR_AI)
#endif /* !defined (USE_INTERRUPTS_FOR_AI) */
							module_configured=1;
							unemap_set_isolate_record_mode(0,1);
							unemap_set_antialiasing_filter_frequency(0,
								initial_antialiasing_filter_frequency);
						}
#if defined (OLD_CODE)
						else
						{
							while (i>0)
							{
								i--;
								DAQ_Clear((module_NI_CARDS[i]).device_number);
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,"unemap_configure.  "
									"DAQ_Clear(%d)=%d\n",(module_NI_CARDS[i]).device_number,
									status);
#endif /* defined (DEBUG) */
#if defined (USE_VIRTUAL_LOCK)
								VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
									(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
									sizeof(i16)));
								VirtualFree((module_NI_CARDS[i]).hardware_buffer,
									(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
									sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
								free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
								GlobalUnlock((module_NI_CARDS[i]).memory_object);
								GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
								DAQ_DB_Config((module_NI_CARDS[i]).device_number,
								/* disable */(i16)0);
							}
							return_code=0;
						}
#endif /* defined (OLD_CODE) */
#if defined (UNEMAP_THREAD_SAFE)
					}
					else
					{
						display_message(ERROR_MESSAGE,"unemap_configure.  "
							"Could not create scrolling_callback_mutex.  Error code %d",
#if defined (WIN32_SYSTEM)
							GetLastError()
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
							error_code
#endif /* defined (UNIX) */
							);
						return_code=0;
					}
#endif /* defined (UNEMAP_THREAD_SAFE) */
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"unemap_configure.  Invalid sampling frequency.  %d %d",
							module_sampling_low_count,module_sampling_high_count);
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
				if (!module_configured)
				{
					DEALLOCATE(module_configured_channels);
					module_number_of_configured_channels=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_configure.  Missing module_NI_CARDS");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_configure.  Hardware is already configured");
			return_code=0;
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_configure.  "
			"Invalid argument(s).  %d %p %g %d %g %g",number_of_channels,
			channel_numbers,sampling_frequency,number_of_samples_in_buffer,
			scrolling_frequency,scrolling_callback_frequency);
		return_code=0;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_configure %d %g "
#if defined (WIN32_USER_INTERFACE)
		"%p %u "
#endif /* defined (WIN32_USER_INTERFACE) */
		"%p %p %d %d\n",return_code,module_sampling_frequency,
#if defined (WIN32_USER_INTERFACE)
		module_scrolling_window,module_scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
		module_scrolling_callback,module_scrolling_callback_data,
		module_scrolling_callback_period,module_NI_CARDS[0].hardware_buffer_size);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_configure */

int unemap_configured(void)
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Returns a non-zero if unemap is configured and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_configured);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured)
	{
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_configured */

int unemap_deconfigure(void)
/*******************************************************************************
LAST MODIFIED : 10 August 2003

DESCRIPTION :
Stops acquisition and signal generation.  Frees buffers associated with the
hardware.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i;
	i16 status;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_deconfigure);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_deconfigure %lu\n",
		module_sample_buffer_size);
	set_check_memory_output(1);
#endif /* defined (DEBUG) */
	return_code=1;
#if defined (NI_DAQ)
	if (module_configured)
	{
		if ((0<module_number_of_NI_CARDS)&&module_NI_CARDS)
		{
			/* stop continuous sampling */
			status=GPCTR_Control(module_NI_CARDS->device_number,SCAN_COUNTER,
				ND_RESET);
			/* make sure that in calibrate mode */
			unemap_set_isolate_record_mode(0,1);
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				status=DAQ_Clear((module_NI_CARDS[i]).device_number);
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"unemap_deconfigure.  "
					"DAQ_Clear(%d)=%d\n",(module_NI_CARDS[i]).device_number,status);
#endif /* defined (DEBUG) */
#if defined (USE_VIRTUAL_LOCK)
				VirtualUnlock((module_NI_CARDS[i]).hardware_buffer,
					(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
					sizeof(i16)));
				VirtualFree((module_NI_CARDS[i]).hardware_buffer,
					(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*
					sizeof(i16)),(DWORD)MEM_RELEASE);
#else /* defined (USE_VIRTUAL_LOCK) */
#if defined (USE_MALLOC)
				free((module_NI_CARDS[i]).memory_object);
#else /* defined (USE_MALLOC) */
				GlobalUnlock((module_NI_CARDS[i]).memory_object);
				GlobalFree((module_NI_CARDS[i]).memory_object);
#endif /* defined (USE_MALLOC) */
#endif /* defined (USE_VIRTUAL_LOCK) */
				/* turn off double buffering */
				status=DAQ_DB_Config((module_NI_CARDS[i]).device_number,
					/* disable */(i16)0);
				if ((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
					(PXI6071E_AD_DA==(module_NI_CARDS[i]).type))
				{
					/* stop waveform generation */
					status=WFM_Group_Control((module_NI_CARDS[i]).device_number,
						/*group*/1,/*Clear*/0);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"unemap_deconfigure.  1.  WFM_Group_Control(%d,CLEAR)=%d\n",
			(module_NI_CARDS[i]).device_number,status);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
					/* set the output voltage to 0 */
					AO_Write((module_NI_CARDS[i]).device_number,CALIBRATE_CHANNEL,
						(i16)0);
					AO_Write((module_NI_CARDS[i]).device_number,STIMULATE_CHANNEL,
						(i16)0);
				}
				DEALLOCATE((module_NI_CARDS[i]).da_buffer);
			}
			module_sample_buffer_size=0;
			module_starting_sample_number=0;
			module_scrolling_callback_period=1;
			module_number_of_scrolling_values_per_channel=1;
#if defined (WIN32_USER_INTERFACE)
			module_scrolling_window=(HWND)NULL;
			module_scrolling_message=(UINT)0;
#endif /* defined (WIN32_USER_INTERFACE) */
			module_scrolling_callback=(Unemap_hardware_callback *)NULL;
			module_scrolling_callback_data=(void *)NULL;
			module_number_of_scrolling_channels=0;
#if defined (UNEMAP_THREAD_SAFE)
			/* free mutex's */
			if (scrolling_callback_mutex)
			{
#if defined (WIN32_SYSTEM)
				CloseHandle(scrolling_callback_mutex);
#endif /* defined (WIN32_SYSTEM) */
#if defined (UNIX)
				pthread_mutex_destroy(scrolling_callback_mutex);
#endif /* defined (UNIX) */
			}
#endif /* defined (UNEMAP_THREAD_SAFE) */
			DEALLOCATE(module_scrolling_channel_numbers);
			DEALLOCATE(module_calibration_channels);
			DEALLOCATE(module_calibration_offsets);
			DEALLOCATE(module_calibration_gains);
		}
		DEALLOCATE(module_configured_channels);
		module_number_of_configured_channels=0;
		module_configured=0;
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	set_check_memory_output(0);
	display_message(INFORMATION_MESSAGE,"leave unemap_deconfigure %d %lu\n",
		return_code,module_sample_buffer_size);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_deconfigure */

int unemap_get_hardware_version(int *hardware_version)
/*******************************************************************************
LAST MODIFIED : 13 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

Returns the unemap <hardware_version>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_hardware_version);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if (hardware_version)
		{
			*hardware_version=module_NI_CARDS->unemap_hardware_version;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_hardware_version.  Missing hardware_version");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_hardware_version.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_hardware_version */

int unemap_get_software_version(int *software_version)
/*******************************************************************************
LAST MODIFIED : 2 June 2003

DESCRIPTION :
The function does not need the software to be configured.

Returns the unemap <software_version>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_software_version);
	return_code=0;
	if (software_version)
	{
		*software_version=SOFTWARE_VERSION;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_software_version.  Missing software_version");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_software_version */

int unemap_shutdown(void)
/*******************************************************************************
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Shuts down NT running on the signal conditioning unit computer.
???DB.  Not really anything to do with unemap hardware ?
==============================================================================*/
{
	int return_code;
#if defined (WIN32_SYSTEM)
	HANDLE token;
	TOKEN_PRIVILEGES token_privileges;
#endif /* defined (WIN32_SYSTEM) */

	ENTER(unemap_shutdown);
	return_code=0;
#if defined (WIN32_SYSTEM)
	/* get a token for this process */
	if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
		&token))
	{
		/* get LUID for the shutdown privilege */
		LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,
			&(token_privileges.Privileges[0].Luid));
		/* one privilege to set */
		token_privileges.PrivilegeCount=1;
		token_privileges.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		/* get the shut down privilege for this process */
		AdjustTokenPrivileges(token,FALSE,&token_privileges,0,
			(PTOKEN_PRIVILEGES)NULL,0);
		/* cannot test the return value of AdjustTokenPrivileges */
		if (ERROR_SUCCESS==GetLastError())
		{
			/* shut down the system and force all applications to close */
			if (ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE,0))
			{
				return_code=1;
			}
		}
	}
#endif /* defined (WIN32_SYSTEM) */
	LEAVE;

	return (return_code);
} /* unemap_shutdown */

int unemap_set_scrolling_channel(int channel_number)
/*******************************************************************************
LAST MODIFIED : 8 August 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Adds the <channel_number> to the list of channels for which scrolling
information is sent via the scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int *temp;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_scrolling_channel);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]))
		{
			if (REALLOCATE(temp,module_scrolling_channel_numbers,int,
				module_number_of_scrolling_channels+1))
			{
				module_scrolling_channel_numbers=temp;
				module_scrolling_channel_numbers[module_number_of_scrolling_channels]=
					channel_number;
				module_number_of_scrolling_channels++;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_set_scrolling_channel.  "
					"Could not reallocate module_scrolling_channel_numbers");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_set_scrolling_channel.  "
				"Invalid channel_number %d",channel_number);
		}
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_scrolling_channel */

int unemap_clear_scrolling_channels(void)
/*******************************************************************************
LAST MODIFIED : 19 July 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Clears the list of channels for which scrolling information is sent via the
scrolling_callback (see unemap_configure).
==============================================================================*/
{
	int return_code;

	ENTER(unemap_clear_scrolling_channels);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		DEALLOCATE(module_scrolling_channel_numbers);
		module_number_of_scrolling_channels=0;
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_clear_scrolling_channels */

int unemap_start_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Starts scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
#if defined (NI_DAQ)
	i16 status,stopped;
	unsigned long ending_sample_number,number_of_samples;
	u32 retrieved;
#endif /* defined (NI_DAQ) */
#endif /* defined (OLD_CODE) */

	ENTER(unemap_start_scrolling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		return_code=1;
		module_scrolling_on=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_start_scrolling */

int unemap_stop_scrolling(void)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

Stops scrolling messages/callbacks.  Also need to be sampling to get messages/
callbacks.  Allows sampling without scrolling.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_scrolling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		return_code=1;
		module_scrolling_on=0;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_stop_scrolling */

int unemap_calibrate(Unemap_calibration_end_callback *calibration_end_callback,
	void *calibration_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

When the calibration is completed <calibration_end_callback> is called with -
the number of channels calibrated, the channel numbers for the calibrated
channels, the offsets for the calibrated channels, the gains for the
calibrated channels and the <calibration_end_callback_data>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_calibrate);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_calibrate\n");
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured)
	{
		module_calibration_end_callback=calibration_end_callback;
		module_calibration_end_callback_data=calibration_end_callback_data;
		return_code=calibration_callback((void *)NULL);
	}
	else
	{
		if (calibration_end_callback)
		{
			(*calibration_end_callback)(0,(int *)NULL,(float *)NULL,(float *)NULL,
				calibration_end_callback_data);
		}
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_calibrate\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_calibrate */

#if defined (CLEAR_DAQ_FOR_START_SAMPLING)
int unemap_start_sampling(void)
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Configures the DAQ and starts the sampling.

The scrolling callback needs to be set up each time otherwise NT crashs (IRQ
problem, but not on first sample) when using large sampling buffers.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float frequency;
	int channel_number,i,j;
	i16 gain_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],number_of_scanned_channels,
		scan_vector[NUMBER_OF_CHANNELS_ON_NI_CARD],status;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_sampling);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_start_sampling\n");
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS)&&
		(0<module_sampling_low_count)&&(0<module_sampling_high_count))
	{
		if (UnEmap_1V2==module_NI_CARDS->unemap_hardware_version)
		{
			/* force a reset of the filter frequency (because power may have been
				turned on and off manually */
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if (0<module_NI_CARDS[i].number_of_configured_channels)
				{
					channel_number=(module_NI_CARDS[i].configured_channels)[0];
					unemap_get_antialiasing_filter_frequency(channel_number,&frequency);
					module_NI_CARDS[i].anti_aliasing_filter_taps= -1;
					unemap_set_antialiasing_filter_frequency(channel_number,frequency);
				}
			}
		}
		/* reconfigure the DAQ */
		/* stop any current sampling */
		status=GPCTR_Control(module_NI_CARDS->device_number,SCAN_COUNTER,ND_RESET);
		ni_card=module_NI_CARDS;
		j=module_number_of_NI_CARDS;
		while (j>0)
		{
			status=DAQ_Clear(ni_card->device_number);
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,"unemap_start_sampling.  "
				"DAQ_Clear(%d)=%d\n",ni_card->device_number,status);
#endif /* defined (DEBUG) */
			if (0==status)
			{
				if (module_NI_CARDS==ni_card)
				{
#if !defined (NO_SCROLLING_CALLBACK)
					status=Config_DAQ_Event_Message(module_NI_CARDS->device_number,
						/* add message */(i16)1,/* channel string */"AI0",
						/* send message every N scans */(i16)1,
						(i32)module_scrolling_callback_period,(i32)0,(u32)0,(u32)0,
						(u32)0,(HWND)NULL,(i16)0,(u32)scrolling_callback_NI);
					if (0!=status)
					{
						display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
							"Config_DAQ_Event_Message 1 failed.  %d %d",status,
							module_scrolling_callback_period);
					}
#endif /* !defined (NO_SCROLLING_CALLBACK) */
				}
				status=DAQ_DB_Config(ni_card->device_number,/* enable */(i16)1);
				if (0==status)
				{
					if (0<ni_card->number_of_configured_channels)
					{
						number_of_scanned_channels=
							ni_card->number_of_configured_channels;
						for (i=0;i<number_of_scanned_channels;i++)
						{
							gain_vector[i]=(i16)(ni_card->gain);
							scan_vector[i]=(ni_card->channel_reorder)[
								((ni_card->configured_channels)[i]-1)%
								NUMBER_OF_CHANNELS_ON_NI_CARD];
						}
					}
					else
					{
						number_of_scanned_channels=1;
						scan_vector[0]=0;
						gain_vector[0]=(i16)(ni_card->gain);
					}
					status=SCAN_Setup(ni_card->device_number,number_of_scanned_channels,
						scan_vector,gain_vector);
					if (0==status)
					{
						status=SCAN_Start(ni_card->device_number,
							(i16 *)(ni_card->hardware_buffer),
							(u32)(ni_card->hardware_buffer_size),ni_card->time_base,
							ni_card->sampling_interval,(i16)0,(u16)0);
						if (0==status)
						{
							if (module_NI_CARDS==ni_card)
							{
#if !defined (USE_INTERRUPTS_FOR AI)
								/*???DB.  I don't understand why this is needed, but if it is
									not present, then get lots of extraneous callbacks (lParam is
									wrong) */
#if !defined (NO_SCROLLING_CALLBACK)
								status=Config_DAQ_Event_Message(module_NI_CARDS->device_number,
									/* clear all messages */(i16)0,
									/* channel string */(char *)NULL,
									/* send message every N scans */(i16)0,
									(i32)0,(i32)0,(u32)0,(u32)0,(u32)0,(HWND)NULL,(i16)0,(u32)0);
								if (0!=status)
								{
									display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
										"Config_DAQ_Event_Message 2 failed.  %d",status);
								}
#endif /* !defined (NO_SCROLLING_CALLBACK) */
#endif /* !defined (USE_INTERRUPTS_FOR AI) */
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
								"SCAN_Start failed.  %d",status);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
							"SCAN_Setup failed.  %d",status);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
						"DAQ_DB_Config failed.  %d",status);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
					"DAQ_Clear failed.  %d",status);
			}
			ni_card++;
			j--;
		}
		module_sample_buffer_size=0;
		module_starting_sample_number=0;
		if (module_slave)
		{
			return_code=1;
			module_sampling_on=1;
		}
		else
		{
			/* set up the conversion signal */
				/*???DB.  Can most of this be moved into unemap_configure ? */
			status=GPCTR_Set_Application(module_NI_CARDS->device_number,
				SCAN_COUNTER,ND_PULSE_TRAIN_GNR);
			if (0==status)
			{
				status=GPCTR_Change_Parameter(module_NI_CARDS->device_number,
					SCAN_COUNTER,ND_COUNT_1,module_sampling_low_count);
				if (0==status)
				{
					status=GPCTR_Change_Parameter(module_NI_CARDS->device_number,
						SCAN_COUNTER,ND_COUNT_2,module_sampling_high_count);
					if (0==status)
					{
						status=Select_Signal(module_NI_CARDS->device_number,
							ND_RTSI_0,ND_GPCTR0_OUTPUT,ND_DONT_CARE);
						if (0==status)
						{
							module_sampling_on=1;
							/* start the data acquisition */
							status=GPCTR_Control(module_NI_CARDS->device_number,SCAN_COUNTER,
								ND_PROGRAM);
							if (0==status)
							{
								return_code=1;
							}
							else
							{
								module_sampling_on=0;
								display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
									"GPCTR_Control (ND_PROGRAM) failed");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
								"Select_Signal(%d,ND_RTSI_0,ND_GPCTR0_SOURCE,ND_DONT_CARE)=%d",
								module_NI_CARDS->device_number,status);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
							"GPCTR_Change_Parameter (high length) failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"unemap_start_sampling.  "
						"GPCTR_Change_Parameter (low length) failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  GPCTR_Set_Application failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_sampling.  Invalid configuration.  %d %p %d %d %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS,
			module_sampling_low_count,module_sampling_high_count);
	}
#endif /* defined (NI_DAQ) */
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_start_sampling\n");
#if defined (DEBUG)
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_sampling */
#else /* defined (CLEAR_DAQ_FOR_START_SAMPLING) */
int unemap_start_sampling(void)
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Starts the sampling.
???DB.  Check if already going
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float frequency;
	int channel_number,i;
	i16 status,stopped;
	u32 retrieved;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_sampling);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_start_sampling\n");
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS)&&
		(0<module_sampling_low_count)&&(0<module_sampling_high_count))
	{
#if defined (RECONFIGURE_FOR_START_SAMPLING)
		/*???debug.  deconfigure and configure before starting sampling */
		{
			float save_module_sampling_frequency;
			int save_hardware_buffer_size,save_module_scrolling_callback_period,
				save_module_number_of_scrolling_values_per_channel;
			Unemap_hardware_callback *save_module_scrolling_callback;
			void *save_module_scrolling_callback_data;
#if defined (WIN32_USER_INTERFACE)
			HWND save_module_scrolling_window;
			UINT save_module_scrolling_message;
#endif /* defined (WIN32_USER_INTERFACE) */

			save_module_sampling_frequency=module_sampling_frequency;
#if defined (WIN32_USER_INTERFACE)
			save_module_scrolling_window=module_scrolling_window;
			save_module_scrolling_message=module_scrolling_message;
#endif /* defined (WIN32_USER_INTERFACE) */
			save_module_scrolling_callback=module_scrolling_callback;
			save_module_scrolling_callback_data=module_scrolling_callback_data;
			save_module_scrolling_callback_period=module_scrolling_callback_period;
			save_module_number_of_scrolling_values_per_channel=
				module_number_of_scrolling_values_per_channel;
			if (0<module_NI_CARDS[0].number_of_configured_channels)
			{
				save_hardware_buffer_size=(module_NI_CARDS[0].hardware_buffer_size)/
					(module_NI_CARDS[0].number_of_configured_channels);
			}
			else
			{
				save_hardware_buffer_size=module_NI_CARDS[0].hardware_buffer_size;
			}
			unemap_deconfigure();
			unemap_configure(save_module_sampling_frequency,
				save_hardware_buffer_size,
#if defined (WIN32_USER_INTERFACE)
				save_module_scrolling_window,save_module_scrolling_message,
#endif /* defined (WIN32_USER_INTERFACE) */
				save_module_scrolling_callback,save_module_scrolling_callback_data,
				save_module_sampling_frequency/
				(float)save_module_scrolling_callback_period,
				(float)save_module_number_of_scrolling_values_per_channel*
				save_module_sampling_frequency/
				(float)save_module_scrolling_callback_period,1);
			unemap_start_scrolling();
			unemap_set_scrolling_channel(1);
		}
#endif /* defined (RECONFIGURE_FOR_START_SAMPLING) */
#if defined (STOP_WAVEFORM_IN_START_SAMPLING)
		unemap_start_sampling_count++;
		if (unemap_start_sampling_count>1)
		{
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				/* stop waveform generation */
				status=WFM_Group_Control((module_NI_CARDS[i]).device_number,
					/*group*/1,/*Clear*/0);
			}
		}
#endif /* defined (STOP_WAVEFORM_IN_START_SAMPLING) */
		if (UnEmap_1V2==module_NI_CARDS->unemap_hardware_version)
		{
			/* force a reset of the filter frequency (because power may have been
				turned on and off manually */
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if (0<module_NI_CARDS[i].number_of_configured_channels)
				{
					channel_number=(module_NI_CARDS[i].configured_channels)[0];
					unemap_get_antialiasing_filter_frequency(channel_number,&frequency);
					module_NI_CARDS[i].anti_aliasing_filter_taps= -1;
					unemap_set_antialiasing_filter_frequency(channel_number,frequency);
				}
			}
		}
#if defined (DEBUG)
/*???debug */
#if defined (VIRTUAL_LOCK)
for (i=0;i<module_number_of_NI_CARDS;i++)
{
	display_message(INFORMATION_MESSAGE,"VirtualLock %d %d\n",i,
		VirtualLock((module_NI_CARDS[i]).hardware_buffer,
		(SIZE_T)(((module_NI_CARDS[i]).hardware_buffer_size)*sizeof(i16))));
}
#endif /* defined (VIRTUAL_LOCK) */
#endif /* defined (DEBUG) */
		if (!module_slave)
		{
			/* stop any current sampling */
				/*???DB.  Check instead using GPCTR_Watch ? */
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,"before GPCTR_Control\n");
#endif /* defined (DEBUG) */
			status=GPCTR_Control(module_NI_CARDS->device_number,SCAN_COUNTER,
				ND_RESET);
#if defined (DEBUG)
			/*???debug */
			display_message(INFORMATION_MESSAGE,"GPCTR_Control=%d\n",status);
#endif /* defined (DEBUG) */
			/* set up the conversion signal */
				/*???DB.  Can most of this be moved into unemap_configure ? */
			if (0==status)
			{
				status=GPCTR_Set_Application(module_NI_CARDS->device_number,
					SCAN_COUNTER,ND_PULSE_TRAIN_GNR);
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"GPCTR_Set_Application=%d\n",
					status);
#endif /* defined (DEBUG) */
				if (0==status)
				{
					status=GPCTR_Change_Parameter(module_NI_CARDS->device_number,
						SCAN_COUNTER,ND_COUNT_1,module_sampling_low_count);
#if defined (DEBUG)
					/*???debug */
					display_message(INFORMATION_MESSAGE,"GPCTR_Change_Parameter=%d\n",
						status);
#endif /* defined (DEBUG) */
					if (0==status)
					{
						status=GPCTR_Change_Parameter(module_NI_CARDS->device_number,
							SCAN_COUNTER,ND_COUNT_2,module_sampling_high_count);
#if defined (DEBUG)
						/*???debug */
						display_message(INFORMATION_MESSAGE,"GPCTR_Change_Parameter 2=%d\n",
							status);
#endif /* defined (DEBUG) */
						if (0==status)
						{
							status=Select_Signal(module_NI_CARDS->device_number,
								ND_RTSI_0,ND_GPCTR0_OUTPUT,ND_DONT_CARE);
#if defined (DEBUG)
							/*???debug */
							display_message(INFORMATION_MESSAGE,"Select_Signal=%d\n",status);
#endif /* defined (DEBUG) */
							if (0==status)
							{
								status=DAQ_Check(module_NI_CARDS->device_number,&stopped,
									&retrieved);
#if defined (DEBUG)
								/*???debug */
								display_message(INFORMATION_MESSAGE,"DAQ_Check=%d\n",status);
#endif /* defined (DEBUG) */
								if (0==status)
								{
									if (0<module_NI_CARDS->number_of_configured_channels)
									{
										module_starting_sample_number=((unsigned long)retrieved/
											(unsigned long)(module_NI_CARDS->
											number_of_configured_channels))%
											(module_NI_CARDS->hardware_buffer_size/
											(unsigned long)(module_NI_CARDS->
											number_of_configured_channels));
									}
									else
									{
										module_starting_sample_number=((unsigned long)retrieved)%
											(module_NI_CARDS->hardware_buffer_size);
									}
									module_sample_buffer_size=0;
									module_sampling_on=1;
									/* start the data acquisition */
									status=GPCTR_Control(module_NI_CARDS->device_number,
										SCAN_COUNTER,ND_PROGRAM);
#if defined (DEBUG)
									/*???debug */
									display_message(INFORMATION_MESSAGE,"GPCTR_Control=%d\n",
										status);
#endif /* defined (DEBUG) */
									if (0==status)
									{
										return_code=1;
									}
									else
									{
										module_sampling_on=0;
										display_message(ERROR_MESSAGE,
									"unemap_start_sampling.  GPCTR_Control (ND_PROGRAM) failed");
									}
								}
								else
								{
									module_sampling_on=0;
									display_message(ERROR_MESSAGE,
										"unemap_start_sampling.  DAQ_Check failed");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
"unemap_start_sampling.  Select_Signal(%d,ND_RTSI_0,ND_GPCTR0_SOURCE,ND_DONT_CARE)=%d",
									module_NI_CARDS->device_number,status);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"unemap_start_sampling.  GPCTR_Change_Parameter (high length) failed");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  GPCTR_Change_Parameter (low length) failed");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_start_sampling.  GPCTR_Set_Application failed");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  GPCTR_Control (ND_RESET) failed");
			}
		}
		else
		{
			status=DAQ_Check(module_NI_CARDS->device_number,&stopped,&retrieved);
			if (0==status)
			{
				if (0<module_NI_CARDS->number_of_configured_channels)
				{
					module_starting_sample_number=((unsigned long)retrieved/
						(unsigned long)(module_NI_CARDS->number_of_configured_channels))%
						((module_NI_CARDS->hardware_buffer_size)/
						(unsigned long)(module_NI_CARDS->number_of_configured_channels));
				}
				else
				{
					module_starting_sample_number=((unsigned long)retrieved)%
						(module_NI_CARDS->hardware_buffer_size);
				}
				module_sample_buffer_size=0;
				return_code=1;
				module_sampling_on=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_start_sampling.  DAQ_Check failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_start_sampling.  Invalid configuration.  %d %p %d %d %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS,
			module_sampling_low_count,module_sampling_high_count);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_start_sampling\n");
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_sampling */
#endif /* defined (CLEAR_DAQ_FOR_START_SAMPLING) */

int unemap_stop_sampling(void)
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Stops the sampling.  Use <unemap_get_number_of_samples_acquired> to find out how
many samples were acquired.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	i16 status,stopped;
	u32 number_of_samples,sample_number;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_stop_sampling);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_stop_sampling %lu\n",
		module_sample_buffer_size);
#endif /* defined (DEBUG) */
	return_code=0;
	/* has to be at the beginning because scrolling_callback_NI is done in a
		different thread */
	module_sampling_on=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		module_sampling_on=0;
		/* stop continuous sampling */
		status=GPCTR_Control(module_NI_CARDS->device_number,SCAN_COUNTER,
			ND_RESET);
		if (0==status)
		{
			status=DAQ_Check(module_NI_CARDS->device_number,&stopped,&sample_number);
			if (0==status)
			{
#if defined (DEBUG)
				/*???debug */
				display_message(INFORMATION_MESSAGE,"%lu %lu %lu\n",sample_number,
					module_NI_CARDS->hardware_buffer_size,module_starting_sample_number);
#endif /* defined (DEBUG) */
				if (0<module_NI_CARDS->number_of_configured_channels)
				{
					sample_number /= module_NI_CARDS->number_of_configured_channels;
					number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
						(module_NI_CARDS->number_of_configured_channels);
				}
				else
				{
					number_of_samples=module_NI_CARDS->hardware_buffer_size;
				}
				if (module_sample_buffer_size<number_of_samples)
				{
					if (sample_number<
						module_starting_sample_number+module_sample_buffer_size)
					{
						module_sample_buffer_size += (sample_number+number_of_samples)-
							(module_starting_sample_number+module_sample_buffer_size);
					}
					else
					{
						module_sample_buffer_size += sample_number-
							(module_starting_sample_number+module_sample_buffer_size);
					}
					if (module_sample_buffer_size>number_of_samples)
					{
						module_sample_buffer_size=number_of_samples;
						module_starting_sample_number=sample_number%number_of_samples;
					}
				}
				else
				{
					module_starting_sample_number=sample_number%number_of_samples;
				}
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_stop_sampling.  DAQ_Check failed.  %d",status);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_stop_sampling.  GPCTR_Control failed.  %d",status);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_stop_sampling.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
#if defined (UNEMAP_THREAD_SAFE)
	display_message(INFORMATION_MESSAGE,"scrolling_callback_mutex_locked_count=%d\n",
		scrolling_callback_mutex_locked_count);
#endif /* defined (UNEMAP_THREAD_SAFE) */
#endif /* defined (DEBUG) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_stop_sampling %d %lu %lu\n",
		return_code,module_sample_buffer_size,module_starting_sample_number);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_sampling */

int unemap_get_sampling(void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Returns a non-zero if unemap is sampling and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_sampling);
	return_code=0;
#if defined (NI_DAQ)
	if (module_sampling_on)
	{
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_sampling */

int unemap_set_isolate_record_mode(int channel_number,int isolate)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

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
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i,j;
	i16 status;
	struct NI_card *ni_card;
	unsigned char stimulate_off;
	u32 counter,counter_save,counter_time;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_isolate_record_mode);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		card_number= -1;
		if ((0<=channel_number)&&((0==channel_number)||(((card_number=
			(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[card_number].
			hardware_buffer_offsets)[(channel_number-1)%
			NUMBER_OF_CHANNELS_ON_NI_CARD]))))
		{
			return_code=1;
			if (isolate)
			{
				/* isolate external circuits.  Since now through pdu and BattB, has
					to be crate by crate */
				if (((-1==card_number)||(0==card_number))&&
					(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version)&&
					!((module_NI_CARDS->unemap_2vx).isolate_mode))
				{
					set_shift_register(module_NI_CARDS,BattB_SHIFT_REGISTER_UnEmap2vx,0,
						0);
				}
				ni_card=module_NI_CARDS;
				for (j=0;j<module_number_of_NI_CARDS;j++)
				{
					if ((-1==card_number)||(j==card_number))
					{
						if (((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))&&
							!((ni_card->unemap_2vx).isolate_mode))
						{
							/* turn off stimulation channels */
							if (relay_power_on_UnEmap2vx)
							{
								stimulate_off=(unsigned char)0x0;
							}
							else
							{
								stimulate_off=(unsigned char)0xff;
							}
							for (i=0;i<8;i++)
							{
								((ni_card->unemap_2vx).stimulate_channels)[i]=
									((ni_card->unemap_2vx).shift_register)[i+2];
								((ni_card->unemap_2vx).shift_register)[i+2]=stimulate_off;
							}
							/* isolate external circuits */
								/*???DB.  In terms of relay_power_on_UnEmap2vx ? */
							set_shift_register(ni_card,EXT_ISO_SHIFT_REGISTER_UnEmap2vx,0,0);
							/* set calibrate mode */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,0);
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							/* use timer to make sure that don't go too quickly */
							/* set D_CAL high for 5 ms */
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								relay_power_on_UnEmap2vx,1);
							status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
								ND_COUNT,&counter_save);
							counter_time=0;
							do
							{
								status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
									ND_COUNT,&counter);
								if (counter<counter_save)
								{
									counter_save=0;
								}
								counter_time += counter-counter_save;
								counter_save=counter;
							} while ((0==status)&&(counter_time<(u32)100000));
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x1;
						}
					}
					ni_card++;
				}
			}
			else
			{
				/* connect external circuits.  Since now through pdu and BattB, has
					to be crate by crate */
				if (((-1==card_number)||(0==card_number))&&
					(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version)&&
					((module_NI_CARDS->unemap_2vx).isolate_mode))
				{
					set_shift_register(module_NI_CARDS,BattB_SHIFT_REGISTER_UnEmap2vx,1,
						0);
				}
				ni_card=module_NI_CARDS;
				for (j=0;j<module_number_of_NI_CARDS;j++)
				{
					if (((-1==card_number)||(j==card_number))&&
						(0<ni_card->number_of_configured_channels))
					{
						if (((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))&&
							((ni_card->unemap_2vx).isolate_mode))
						{
							/* turn on stimulation channels */
							for (i=0;i<8;i++)
							{
								((ni_card->unemap_2vx).shift_register)[i+2]=
									((ni_card->unemap_2vx).stimulate_channels)[i];
							}
							/* connect external circuits */
								/*???DB.  In terms of relay_power_on_UnEmap2vx ? */
							set_shift_register(ni_card,EXT_ISO_SHIFT_REGISTER_UnEmap2vx,1,0);
							/* set record mode */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,0);
							set_shift_register(ni_card,D_CAL_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							/* use timer to make sure that don't go too quickly */
							/* set D_REC high for 5 ms */
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								relay_power_on_UnEmap2vx,1);
							status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
								ND_COUNT,&counter_save);
							counter_time=0;
							do
							{
								status=GPCTR_Watch(ni_card->device_number,DIGITAL_IO_COUNTER,
									ND_COUNT,&counter);
								if (counter<counter_save)
								{
									counter_save=0;
								}
								counter_time += counter-counter_save;
								counter_save=counter;
							} while ((0==status)&&(counter_time<(u32)100000));
							set_shift_register(ni_card,D_REC_SHIFT_REGISTER_UnEmap2vx,
								!relay_power_on_UnEmap2vx,1);
							(ni_card->unemap_2vx).isolate_mode=(unsigned char)0x0;
						}
					}
					ni_card++;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_isolate_record_mode.  Invalid channel_number.  %d",
				channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_isolate_record_mode.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_isolate_record_mode */

int unemap_get_isolate_record_mode(int channel_number,int *isolate)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

If the group is in isolate mode, then <*isolate> is set to 1.  Otherwise
<*isolate> is set to 0.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_isolate_record_mode);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&isolate)
		{
			ni_card=module_NI_CARDS+(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
				(UnEmap_2V2==ni_card->unemap_hardware_version))
			{
				return_code=1;
				if (ni_card->unemap_2vx.isolate_mode)
				{
					*isolate=1;
				}
				else
				{
					*isolate=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_isolate_record_mode.  Invalid argument(s).  %d %p",
				channel_number,isolate);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_isolate_record_mode.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_isolate_record_mode */

int unemap_set_antialiasing_filter_frequency(int channel_number,
	float frequency)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the anti-aliasing filter to the specified <frequency> for all channels.
UnEmap_1V2
3dB cutoff frequency = 1.07/(100*R*C)
C = 10 nF
R = (99-tap) * 101 ohm
UnEmap_2V1 or UnEmap_2V2
3dB cutoff frequency = 50*tap
<frequency> is a request and the actual value used can be found with
unemap_get_antialiasing_filter.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i,j,tap;
	i16 status;
	u32 counter,counter_start;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	/* check configuration */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		card_number= -1;
#if defined (OLD_CODE)
/*???DB.  Channel doesn't have to be configured */
		if ((0<=channel_number)&&((0==channel_number)||(((card_number=
			(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[card_number].
			hardware_buffer_offsets)[(channel_number-1)%
			NUMBER_OF_CHANNELS_ON_NI_CARD]))))
#endif /* defined (OLD_CODE) */
		if ((0<=channel_number)&&((0==channel_number)||((card_number=
			(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)))
		{
			return_code=1;
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if ((-1==card_number)||(i==card_number))
				{
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
#if defined (OLD_CODE)
/*???DB.  Changed to new clock circuit */
						{
							tap=99-(int)floor((float)1.07e6/(frequency*(float)101)+0.5);
						} break;
#endif /* defined (OLD_CODE) */
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							tap=(int)floor(frequency/(float)50.0+0.5);
						} break;
					}
					if (tap<1)
					{
						tap=1;
					}
					else
					{
						if (tap>99)
						{
							tap=99;
						}
					}
					if (tap!=(module_NI_CARDS[i]).anti_aliasing_filter_taps)
					{
						(module_NI_CARDS[i]).anti_aliasing_filter_taps=tap;
						/* set chip select low to program */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									chip_select_line_UnEmap1vx,(i16)0);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FCS_SHIFT_REGISTER_UnEmap2vx,0,1);
							} break;
						}
						/* use timer to make sure that don't go too quickly */
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						/* tap to the bottom */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									up_down_line_UnEmap1vx,(i16)0);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FUD_SHIFT_REGISTER_UnEmap2vx,0,1);
							} break;
						}
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
							filter_increment_output_line,(i16)0);
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)40));
						for (j=100;j>0;j--)
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)1);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)0);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
						}
						/* tap to the required value */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									up_down_line_UnEmap1vx,(i16)1);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FUD_SHIFT_REGISTER_UnEmap2vx,1,1);
							} break;
						}
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
						do
						{
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter);
							if (counter<counter_start)
							{
								counter_start=0;
							}
						} while ((0==status)&&(counter-counter_start<(u32)2));
						for (j=tap;j>0;j--)
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)1);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								filter_increment_output_line,(i16)0);
							status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
								DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
							do
							{
								status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
									DIGITAL_IO_COUNTER,ND_COUNT,&counter);
								if (counter<counter_start)
								{
									counter_start=0;
								}
							} while ((0==status)&&(counter-counter_start<(u32)40));
						}
						/* set chip select high to stop programming */
						switch ((module_NI_CARDS[i]).unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
									chip_select_line_UnEmap1vx,(i16)1);
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								set_shift_register(module_NI_CARDS+i,
									FCS_SHIFT_REGISTER_UnEmap2vx,1,1);
							} break;
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_antialiasing_filter_frequency.  "
				"Invalid channel_number.  %d",channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_set_antialiasing_filter_frequency.  "
			"Invalid configuration.  %d %p %d",module_configured,module_NI_CARDS,
			module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_antialiasing_filter_frequency */

int unemap_set_powerup_antialiasing_filter_frequency(int channel_number)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Makes the current anti-aliasing filter frequency the power up value for the
group.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int card_number,i;
	i16 status;
	u32 counter,counter_start;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_powerup_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	/* check configuration */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		card_number= -1;
#if defined (OLD_CODE)
/*???DB.  Channel doesn't have to be configured */
		if ((0<=channel_number)&&((0==channel_number)||(((card_number=
			(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[card_number].
			hardware_buffer_offsets)[(channel_number-1)%
			NUMBER_OF_CHANNELS_ON_NI_CARD]))))
#endif /* defined (OLD_CODE) */
		if ((0<=channel_number)&&((0==channel_number)||((card_number=
			(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)))
		{
			return_code=1;
			for (i=0;i<module_number_of_NI_CARDS;i++)
			{
				if ((-1==card_number)||(i==card_number))
				{
					/* set chip select low to program */
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								chip_select_line_UnEmap1vx,(i16)0);
						} break;
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							set_shift_register(module_NI_CARDS+i,FCS_SHIFT_REGISTER_UnEmap2vx,
								0,1);
						} break;
					}
					/* use timer to make sure that don't go too quickly */
					status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
						DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)2));
					/* set increment high so that setting is saved in nvRAM */
					DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
						filter_increment_output_line,(i16)1);
					status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
						DIGITAL_IO_COUNTER,ND_COUNT,&counter_start);
					do
					{
						status=GPCTR_Watch((module_NI_CARDS[i]).device_number,
							DIGITAL_IO_COUNTER,ND_COUNT,&counter);
						if (counter<counter_start)
						{
							counter_start=0;
						}
					} while ((0==status)&&(counter-counter_start<(u32)40));
					/* set chip select high to stop programming */
					switch ((module_NI_CARDS[i]).unemap_hardware_version)
					{
						case UnEmap_1V2:
						{
							DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
								chip_select_line_UnEmap1vx,(i16)1);
						} break;
						case UnEmap_2V1:
						case UnEmap_2V2:
						{
							set_shift_register(module_NI_CARDS+i,FCS_SHIFT_REGISTER_UnEmap2vx,
								1,1);
						} break;
					}
					/* set increment low */
					DIG_Out_Line((module_NI_CARDS[i]).device_number,0,
						filter_increment_output_line,(i16)0);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_powerup_antialiasing_filter_frequency.  "
				"Invalid channel_number.  %d",channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_powerup_antialiasing_filter_frequency.  "
			"Invalid configuration.  %p %d",module_NI_CARDS,
			module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_powerup_antialiasing_filter_frequency */

int unemap_get_antialiasing_filter_frequency(int channel_number,
	float *frequency)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

<*frequency> is set to the frequency for the anti-aliasing filter.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int tap;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_antialiasing_filter_frequency);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
#if defined (OLD_CODE)
/*???DB.  Channel doesn't have to be configured */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&frequency)
#endif /* defined (OLD_CODE) */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&frequency)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			tap=ni_card->anti_aliasing_filter_taps;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
#if defined (OLD_CODE)
/*???DB.  Changed to new clock circuit */
				{
					*frequency=(float)1.07e6/((float)(99-tap)*(float)101);
					return_code=1;
				} break;
#endif /* defined (OLD_CODE) */
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					*frequency=(float)tap*(float)50.0;
					return_code=1;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_antialiasing_filter_frequency.  "
				"Invalid argument(s).  %d %p",channel_number,frequency);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_antialiasing_filter_frequency.  "
			"Invalid configuration.  %d %p %d",module_configured,module_NI_CARDS,
			module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_antialiasing_filter_frequency */

int unemap_get_number_of_channels(int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_channels);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_channels)
	{
		return_code=1;
		if (search_for_NI_cards()&&module_NI_CARDS)
		{
			*number_of_channels=(int)(module_number_of_NI_CARDS*
				NUMBER_OF_CHANNELS_ON_NI_CARD);
		}
		else
		{
			*number_of_channels=0;
		}
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"unemap_get_number_of_channels %d %d\n",return_code,
			*number_of_channels);
#endif /* defined (DEBUG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_channels.  Missing number_of_channels");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_channels */

int unemap_get_number_of_configured_channels(int *number_of_channels)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

The number of configured channels is assigned to <*number_of_channels>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_configured_channels);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_channels)
	{
		return_code=1;
		*number_of_channels=module_number_of_configured_channels;
#if defined (DEBUG)
		/*???debug */
		display_message(INFORMATION_MESSAGE,
			"unemap_get_number_of_configured_channels %d %d\n",return_code,
			*number_of_channels);
#endif /* defined (DEBUG) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_configured_channels.  Missing number_of_channels");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_configured_channels */

int unemap_get_sample_range(int channel_number,long int *minimum_sample_value,
	long int *maximum_sample_value)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The minimum possible sample value is assigned to <*minimum_sample_value> and the
maximum possible sample value is assigned to <*maximum_sample_value>.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_sample_range);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&minimum_sample_value&&
			maximum_sample_value)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			if (PXI6071E_AD_DA==ni_card->type)
			{
				*maximum_sample_value=(long int)2048;
				*minimum_sample_value=(long int)-2047;
			}
			else
			{
				*maximum_sample_value=(long int)32768;
				*minimum_sample_value=(long int)-32767;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sample_range.  Invalid argument(s).  %d %p %p",
				channel_number,minimum_sample_value,maximum_sample_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sample_range.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_sample_range */

int unemap_get_voltage_range(int channel_number,float *minimum_voltage,
	float *maximum_voltage)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The voltage range, allowing for gain, is returned via <*minimum_voltage> and
<*maximum_voltage>.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float pre_filter_gain,post_filter_gain,temp;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_voltage_range);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&minimum_voltage&&
			maximum_voltage)
		{
			if (unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain))
			{
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				if (PXI6071E_AD_DA==ni_card->type)
				{
					temp=(float)5;
				}
				else
				{
					temp=(float)10;
				}
				temp /= pre_filter_gain*post_filter_gain;
				*minimum_voltage= -temp;
				*maximum_voltage=temp;
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_voltage_range.  Invalid argument(s).  %d %p %p",
				channel_number,minimum_voltage,maximum_voltage);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_voltage_range.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_voltage_range */

int unemap_get_number_of_samples_acquired(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 12 March 2000

DESCRIPTION :
The function fails if the hardware is not configured.

The number of samples acquired per channel since <unemap_start_sampling> is
assigned to <*number_of_samples>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_samples_acquired);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_samples)
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*number_of_samples=module_sample_buffer_size;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"unemap_get_number_of_samples_acquired.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_samples_acquired.  Missing number_of_samples");
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_get_number_of_samples_acquired %d %lu\n",return_code,
		*number_of_samples);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_samples_acquired */

int unemap_transfer_samples_acquired(int channel_number,int number_of_samples,
	Unemap_transfer_samples_function *transfer_samples_function,
	void *transfer_samples_function_data,int *number_of_samples_transferred)
/*******************************************************************************
LAST MODIFIED : 14 August 2003

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
{
	int number_of_channels,number_transferred,return_code;
#if defined (NI_DAQ)
	int end,end2,i,j,k,k_phase,number_of_configured_channels,offset,samples_count,
		start,transfer_samples_function_result;
	short int sample,samples[NUMBER_OF_CHANNELS_ON_NI_CARD],*source;
	struct NI_card *ni_card;
	unsigned long maximum_number_of_samples,local_number_of_samples;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_transfer_samples_acquired);
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_transfer_samples_acquired %d %d\n",
		module_starting_sample_number,module_sample_buffer_size);
#endif /* defined (DEBUG) */
	return_code=0;
	number_transferred=0;
	number_of_channels=1;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<=channel_number)&&((0==channel_number)||
			((((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])))&&
			transfer_samples_function)
		{
			return_code=1;
			if (number_of_samples<=0)
			{
				local_number_of_samples=module_sample_buffer_size;
			}
			else
			{
				local_number_of_samples=(unsigned long)number_of_samples;
				if (local_number_of_samples>module_sample_buffer_size)
				{
					local_number_of_samples=module_sample_buffer_size;
				}
			}
			if (0==channel_number)
			{
				number_of_channels=0;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					number_of_channels +=
						module_NI_CARDS[i].number_of_configured_channels;
				}
			}
			else
			{
				number_of_channels=1;
			}
			if (0<module_NI_CARDS->number_of_configured_channels)
			{
				maximum_number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
					(module_NI_CARDS->number_of_configured_channels);
			}
			else
			{
				maximum_number_of_samples=module_NI_CARDS->hardware_buffer_size;
			}
			samples_count=0;
			start=module_starting_sample_number;
			if (module_starting_sample_number+local_number_of_samples>
				maximum_number_of_samples)
			{
				end=maximum_number_of_samples;
				end2=module_starting_sample_number+local_number_of_samples-
					maximum_number_of_samples;
			}
			else
			{
				end=module_starting_sample_number+local_number_of_samples;
				end2=0;
			}
			if (0==channel_number)
			{
				k=start;
				k_phase=0;
				while (return_code&&(((0==k_phase)&&(k>=start)&&(k<end))||
					((1==k_phase)&&(k>=0)&&(k<end2))))
				{
					i=0;
					while (return_code&&(i<module_number_of_configured_channels))
					{
						j=(module_configured_channels[i]-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
						if (j<module_number_of_NI_CARDS)
						{
							ni_card=module_NI_CARDS+j;
							number_of_configured_channels=
								ni_card->number_of_configured_channels;
							if ((0<number_of_configured_channels)&&(0<=(offset=
								(ni_card->hardware_buffer_offsets)[
								(module_configured_channels[i]-1)%
								NUMBER_OF_CHANNELS_ON_NI_CARD])))
							{
								source=(ni_card->hardware_buffer)+
									(k*number_of_configured_channels);
								sample=source[offset];
								if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
									(UnEmap_2V2==ni_card->unemap_hardware_version))
								{
									/* UnEmap_2V1 and UnEmap_2V2 invert */
									if (sample<SHRT_MAX)
									{
										sample= -sample;
									}
									else
									{
										sample=SHRT_MIN;
									}
								}
								samples[samples_count]=sample;
								samples_count++;
								if (samples_count>=NUMBER_OF_CHANNELS_ON_NI_CARD)
								{
									transfer_samples_function_result=(*transfer_samples_function)(
										samples,NUMBER_OF_CHANNELS_ON_NI_CARD,
										transfer_samples_function_data);
									number_transferred += transfer_samples_function_result;
									if (NUMBER_OF_CHANNELS_ON_NI_CARD!=
										transfer_samples_function_result)
									{
										return_code=0;
									}
									samples_count=0;
								}
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"unemap_transfer_samples_acquired.  Invalid configured "
									"channel offset %d %d %d %d",channel_number,
									module_configured_channels[i],number_of_configured_channels,
									(ni_card->hardware_buffer_offsets)[
									(module_configured_channels[i]-1)%
									NUMBER_OF_CHANNELS_ON_NI_CARD]);
							}
						}
						else
						{
							return_code=0;
							display_message(ERROR_MESSAGE,
								"unemap_transfer_samples_acquired.  Invalid configured channel "
								"%d %d %d %d %d",i,channel_number,module_configured_channels[i],
								j,module_number_of_NI_CARDS);
						}
						i++;
					}
					k++;
					if (k==end)
					{
						k_phase=1;
						k=0;
					}
				}
			}
			else
			{
				ni_card=module_NI_CARDS+
					((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
				number_of_configured_channels=ni_card->number_of_configured_channels;
				if ((0<number_of_configured_channels)&&(0<=(offset=
					(ni_card->hardware_buffer_offsets)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD])))
				{
					source=(ni_card->hardware_buffer)+
						(start*number_of_configured_channels+offset);
					k=start;
					k_phase=0;
					while (return_code&&(((0==k_phase)&&(k>=start)&&(k<end))||
						((1==k_phase)&&(k>=0)&&(k<end2))))
					{
						sample= *source;
						if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
							(UnEmap_2V2==ni_card->unemap_hardware_version))
						{
							/* UnEmap_2V1 and UnEmap_2V2 invert */
							if (sample<SHRT_MAX)
							{
								sample= -sample;
							}
							else
							{
								sample=SHRT_MIN;
							}
						}
						samples[samples_count]=sample;
						samples_count++;
						if (samples_count>=NUMBER_OF_CHANNELS_ON_NI_CARD)
						{
							transfer_samples_function_result=(*transfer_samples_function)(
								samples,NUMBER_OF_CHANNELS_ON_NI_CARD,
								transfer_samples_function_data);
							number_transferred += transfer_samples_function_result;
							if (NUMBER_OF_CHANNELS_ON_NI_CARD!=
								transfer_samples_function_result)
							{
								return_code=0;
							}
							samples_count=0;
						}
						k++;
						if (k==end)
						{
							k_phase=1;
							k=0;
							source=(ni_card->hardware_buffer)+offset;
						}
						else
						{
							source += number_of_configured_channels;
						}
					}
				}
				else
				{
					return_code=0;
					display_message(ERROR_MESSAGE,
						"unemap_transfer_samples_acquired.  Invalid configured "
						"channel offset %d %d %d %d",channel_number,
						number_of_configured_channels,
						(ni_card->hardware_buffer_offsets)[(channel_number-1)%
						NUMBER_OF_CHANNELS_ON_NI_CARD]);
				}
			}
			if (return_code&&(samples_count>0))
			{
				transfer_samples_function_result=(*transfer_samples_function)(
					samples,samples_count,transfer_samples_function_data);
				number_transferred += transfer_samples_function_result;
				if (samples_count!=transfer_samples_function_result)
				{
					return_code=0;
				}
				samples_count=0;
			}
			if (0==return_code)
			{
				display_message(ERROR_MESSAGE,
					"unemap_transfer_samples_acquired.  Error transferring samples");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_transfer_samples_acquired.  Invalid argument(s).  %p %d",
				transfer_samples_function,channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_transfer_samples_acquired.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	if (number_of_samples_transferred)
	{
		*number_of_samples_transferred=number_transferred/number_of_channels;
	}
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_transfer_samples_acquired %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_transfer_samples_acquired */

int unemap_write_samples_acquired(int channel_number,int number_of_samples,
	FILE *file,int *number_of_samples_written)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

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
{
	int return_code;
#if defined (NI_DAQ)
	int i,number_of_channels;
	unsigned long local_number_of_samples;
#endif /* defined (NI_DAQ) */
#if defined (WIN32_IO)
	DWORD number_of_bytes_written;
#endif /* defined (WIN32_IO) */

	ENTER(unemap_write_samples_acquired);
	return_code=0;
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"enter unemap_write_samples_acquired %d %d\n",
		module_starting_sample_number,module_sample_buffer_size);
#endif /* defined (DEBUG) */
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<=channel_number)&&((0==channel_number)||
			((((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])))&&file)
		{
			if (number_of_samples<=0)
			{
				local_number_of_samples=module_sample_buffer_size;
			}
			else
			{
				local_number_of_samples=(unsigned long)number_of_samples;
				if (local_number_of_samples>module_sample_buffer_size)
				{
					local_number_of_samples=module_sample_buffer_size;
				}
			}
			if (0==channel_number)
			{
				number_of_channels=0;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					number_of_channels +=
						module_NI_CARDS[i].number_of_configured_channels;
				}
				if (0==number_of_channels)
				{
					local_number_of_samples=0;
				}
			}
			else
			{
				number_of_channels=1;
			}
#if defined (WIN32_IO)
			WriteFile((HANDLE)file,(LPCVOID)&channel_number,
				(DWORD)sizeof(channel_number),&number_of_bytes_written,
				(LPOVERLAPPED)NULL);
			WriteFile((HANDLE)file,(LPCVOID)&number_of_channels,
				(DWORD)sizeof(number_of_channels),&number_of_bytes_written,
				(LPOVERLAPPED)NULL);
			WriteFile((HANDLE)file,(LPCVOID)&local_number_of_samples,
				(DWORD)sizeof(local_number_of_samples),&number_of_bytes_written,
				(LPOVERLAPPED)NULL);
#else /* defined (WIN32_IO) */
			fwrite((char *)&channel_number,sizeof(channel_number),1,file);
			fwrite((char *)&number_of_channels,sizeof(number_of_channels),1,file);
			fwrite((char *)&local_number_of_samples,sizeof(local_number_of_samples),1,
				file);
#endif /* defined (WIN32_IO) */
			return_code=unemap_transfer_samples_acquired(channel_number,
				number_of_samples,file_write_samples_acquired,(void *)file,
				number_of_samples_written);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_write_samples_acquired.  Invalid argument(s).  %p %d",file,
				channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_write_samples_acquired.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,
		"leave unemap_write_samples_acquired %d\n",return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_write_samples_acquired */

int unemap_get_samples_acquired(int channel_number,int number_of_samples,
	short int *samples,int *number_of_samples_got)
/*******************************************************************************
LAST MODIFIED : 13 January 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then the <samples> for that channel are returned.  If
<channel_number> is 0 then the <samples> for all channels are returned.
Otherwise the function fails.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are got.  Otherwise,
<number_of_samples> are got. If <number_of_samples_got> is not NULL, then it is
set to the number of samples got.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	short int *sample;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_samples_acquired);
	return_code=0;
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"enter unemap_get_samples_acquired\n");
#endif /* defined (DEBUG) */
#if defined (NI_DAQ)
	/* check arguments */
	if (sample=samples)
	{
		return_code=unemap_transfer_samples_acquired(channel_number,
			number_of_samples,memory_copy_samples_acquired,(void *)&sample,
			number_of_samples_got);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_samples_acquired.  Invalid argument.  %p",samples);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
	/*???debug */
	display_message(INFORMATION_MESSAGE,"leave unemap_get_samples_acquired %d\n",
		return_code);
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired */

int unemap_get_samples_acquired_background(int channel_number,
	int number_of_samples,Unemap_acquired_data_callback *callback,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <number_of_samples> is not positive or greater than the number of samples
available, then the number of samples available are got.  Otherwise,
<number_of_samples> are got.

The function gets the samples specified by the <channel_number> and calls the
<callback> with the <channel_number>, the number of samples, the samples and the
<user_data>.  The <callback> is responsible for deallocating the samples memory.

When the function returns, it is safe to call any of the other functions
(including unemap_start_sampling), but the <callback> may not have finished or
even been called yet.  This function allows data to be transferred in the
background in a client/server arrangement.

For this version (hardware local), it retrieves the samples and calls the
<callback> before returning.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i,local_number_of_samples,number_of_channels,number_of_samples_got;
	short *samples;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_samples_acquired_background);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<=channel_number)&&((0==channel_number)||
			((((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])))&&callback)
		{
			if (number_of_samples<=0)
			{
				local_number_of_samples=(int)module_sample_buffer_size;
			}
			else
			{
				local_number_of_samples=number_of_samples;
				if (local_number_of_samples>(int)module_sample_buffer_size)
				{
					local_number_of_samples=(int)module_sample_buffer_size;
				}
			}
			if (0==channel_number)
			{
				number_of_channels=0;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					number_of_channels +=
						module_NI_CARDS[i].number_of_configured_channels;
				}
			}
			else
			{
				number_of_channels=1;
			}
			if ((0<number_of_channels)&&(0<local_number_of_samples)&&
				ALLOCATE(samples,short,number_of_channels*local_number_of_samples))
			{
				if (unemap_get_samples_acquired(channel_number,number_of_samples,
					samples,&number_of_samples_got))
				{
					(*callback)(channel_number,number_of_samples_got,samples,user_data);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"unemap_get_samples_acquired_background.  Could not get samples");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_get_samples_acquired_background.  "
					"Could not allocate samples");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_samples_acquired_background.  "
				"Invalid argument(s).  %p %d",callback,channel_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_samples_acquired_background.  "
			"Invalid configuration.  %d %p %d",module_configured,module_NI_CARDS,
			module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_samples_acquired_background */

int unemap_get_maximum_number_of_samples(unsigned long *number_of_samples)
/*******************************************************************************
LAST MODIFIED : 7 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The size of the rolling buffer, in number of samples per channel, is assigned to
<*number_of_samples>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_maximum_number_of_samples);
	return_code=0;
	/* check arguments */
	if (number_of_samples)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			if (0<module_NI_CARDS->number_of_configured_channels)
			{
				*number_of_samples=(module_NI_CARDS->hardware_buffer_size)/
					(module_NI_CARDS->number_of_configured_channels);
			}
			else
			{
				*number_of_samples=module_NI_CARDS->hardware_buffer_size;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_maximum_number_of_samples.  Missing number_of_samples");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_maximum_number_of_samples */

int unemap_get_sampling_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 3 May 1999

DESCRIPTION :
The function fails if the hardware is not configured.

The sampling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_sampling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*frequency=module_sampling_frequency;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_sampling_frequency.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_sampling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_sampling_frequency */

int unemap_get_scrolling_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_scrolling_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*frequency=(float)module_number_of_scrolling_values_per_channel*
				module_sampling_frequency/(float)module_scrolling_callback_period;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_scrolling_frequency.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_scrolling_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_scrolling_frequency */

int unemap_get_scrolling_callback_frequency(float *frequency)
/*******************************************************************************
LAST MODIFIED : 4 June 2003

DESCRIPTION :
The function fails if the hardware is not configured.

The scrolling frequency is assigned to <*frequency>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_scrolling_callback_frequency);
	return_code=0;
	/* check arguments */
	if (frequency)
	{
#if defined (NI_DAQ)
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			*frequency=module_sampling_frequency/
				(float)module_scrolling_callback_period;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_scrolling_callback_frequency.  "
				"Invalid configuration.  %d %p %d",module_configured,module_NI_CARDS,
				module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_scrolling_callback_frequency.  Missing frequency");
	}
	LEAVE;

	return (return_code);
} /* unemap_get_scrolling_callback_frequency */

int unemap_set_gain(int channel_number,float pre_filter_gain,
	float post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Sets the gain before the band pass filter and the gain after the band pass
filter to the specified values.  If the <pre_filter_gain> or the
<post_filter_gain> is outside the available range, then the other gain will be
adjusted to try and achieve the product <pre_filter_gain>*<post_filter_gain>.

For UnEmap_1V2, there is no gain before the filter.
For UnEmap_2V1 and UnEmap_2V2, the gain before the filter can be 1, 2, 4 or 8.

For UnEmap_1V2, the post filter gain can be 10, 20, 50, 100, 200, 500 or 1000
(fixed gain of 10).
For UnEmap_2V1 and UnEmap_2V2, the post filter gain can be 11, 22, 55, 110, 220,
550 or 1100 (fixed gain of 11).
For the 16-bit NI cards, the lowest gain is not used because the input range for
it is +/-10V and the output range for the SCU is +/-5V.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float local_post_filter_gain,local_pre_filter_gain;
	int card_number,i;
	i16 gain;
	struct NI_card *ni_card;
	i16 GA0_setting,GA1_setting;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_gain);
	return_code=0;
	if ((0<pre_filter_gain)&&(0<post_filter_gain))
	{
#if defined (NI_DAQ)
		/* check configuration */
		if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			card_number= -1;
			if ((0<=channel_number)&&((0==channel_number)||(((card_number=
				(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
				module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[card_number].
				hardware_buffer_offsets)[(channel_number-1)%
				NUMBER_OF_CHANNELS_ON_NI_CARD]))))
			{
				return_code=1;
				ni_card=module_NI_CARDS;
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					if ((-1==card_number)||(i==card_number))
					{
						switch (ni_card->unemap_hardware_version)
						{
							case UnEmap_1V2:
							{
								local_post_filter_gain=pre_filter_gain*post_filter_gain;
								if (local_post_filter_gain<15)
								{
									switch (ni_card->type)
									{
										case PCI6031E_AD_DA:
										case PCI6033E_AD:
										case PXI6031E_AD_DA:
										{
											/* lowest gain not used for 16 bit */
											gain=2;
										} break;
										default:
										{
											gain=1;
										} break;
									}
								}
								else
								{
									if (local_post_filter_gain<35)
									{
										gain=2;
									}
									else
									{
										if (local_post_filter_gain<75)
										{
											gain=5;
										}
										else
										{
											if (local_post_filter_gain<150)
											{
												gain=10;
											}
											else
											{
												if (local_post_filter_gain<350)
												{
													gain=20;
												}
												else
												{
													if (local_post_filter_gain<750)
													{
														gain=50;
													}
													else
													{
														gain=100;
													}
												}
											}
										}
									}
								}
								if (gain!=ni_card->gain)
								{
									set_NI_gain(ni_card,gain);
									ni_card->gain=gain;
								}
							} break;
							case UnEmap_2V1:
							case UnEmap_2V2:
							{
								if (pre_filter_gain<1.5)
								{
									GA0_setting=0;
									GA1_setting=0;
									local_pre_filter_gain=1;
								}
								else
								{
									if (pre_filter_gain<3)
									{
										GA0_setting=1;
										GA1_setting=0;
										local_pre_filter_gain=2;
									}
									else
									{
										if (pre_filter_gain<6)
										{
											GA0_setting=0;
											GA1_setting=1;
											local_pre_filter_gain=4;
										}
										else
										{
											GA0_setting=1;
											GA1_setting=1;
											local_pre_filter_gain=8;
										}
									}
								}
								local_post_filter_gain=(pre_filter_gain*post_filter_gain)/
									local_pre_filter_gain;
								if (local_post_filter_gain<16)
								{
									switch (ni_card->type)
									{
										case PCI6031E_AD_DA:
										case PCI6033E_AD:
										case PXI6031E_AD_DA:
										{
											/* lowest gain not used for 16 bit */
											gain=2;
										} break;
										default:
										{
											gain=1;
										} break;
									}
								}
								else
								{
									if (local_post_filter_gain<38)
									{
										gain=2;
									}
									else
									{
										if (local_post_filter_gain<82)
										{
											gain=5;
										}
										else
										{
											if (local_post_filter_gain<165)
											{
												gain=10;
											}
											else
											{
												if (local_post_filter_gain<385)
												{
													gain=20;
												}
												else
												{
													if (local_post_filter_gain<825)
													{
														gain=50;
													}
													else
													{
														gain=100;
													}
												}
											}
										}
									}
								}
								local_pre_filter_gain=(pre_filter_gain*post_filter_gain)/
									local_post_filter_gain;
								if (local_pre_filter_gain<1.5)
								{
									GA0_setting=0;
									GA1_setting=0;
								}
								else
								{
									if (local_pre_filter_gain<3)
									{
										GA0_setting=1;
										GA1_setting=0;
									}
									else
									{
										if (local_pre_filter_gain<6)
										{
											GA0_setting=0;
											GA1_setting=1;
										}
										else
										{
											GA0_setting=1;
											GA1_setting=1;
										}
									}
								}
								if (GA0_setting)
								{
									(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x1;
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_0_output_line_UnEmap2vx,(i16)1);
								}
								else
								{
									(ni_card->unemap_2vx).gain_output_line_0=(unsigned char)0x0;
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_0_output_line_UnEmap2vx,(i16)0);
								}
								if (GA1_setting)
								{
									(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x1;
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_1_output_line_UnEmap2vx,(i16)1);
								}
								else
								{
									(ni_card->unemap_2vx).gain_output_line_1=(unsigned char)0x0;
									DIG_Out_Line(ni_card->device_number,(i16)0,
										gain_1_output_line_UnEmap2vx,(i16)0);
								}
								if (gain!=ni_card->gain)
								{
									set_NI_gain(ni_card,gain);
									ni_card->gain=gain;
								}
							} break;
						}
					}
					ni_card++;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_set_gain.  Invalid channel_number.  %d",channel_number);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_gain.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
#endif /* defined (NI_DAQ) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_set_gain.  Invalid gain(s).  %g %g",
			pre_filter_gain,post_filter_gain);
	}
	LEAVE;

	return (return_code);
} /* unemap_set_gain */

int unemap_get_gain(int channel_number,float *pre_filter_gain,
	float *post_filter_gain)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  Otherwise, the function fails.

The <*pre_filter_gain> and <*post_filter_gain> for the group are assigned.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float temp;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_gain);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&pre_filter_gain&&
			post_filter_gain)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			temp=(float)1;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
				{
					/* fixed gain of 10 */
					temp=(float)10*(float)(ni_card->gain);
				} break;
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					/* fixed gain of 11 */
					temp=(float)11*(float)(ni_card->gain);
				} break;
			}
			*post_filter_gain=temp;
			temp=(float)1;
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					if ((ni_card->unemap_2vx).gain_output_line_0)
					{
						temp *= 2;
					}
					if ((ni_card->unemap_2vx).gain_output_line_1)
					{
						temp *= 4;
					}
				} break;
			}
			*pre_filter_gain=temp;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_gain.  "
				"Invalid argument(s).  %d %p %p",channel_number,pre_filter_gain,
				post_filter_gain);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_gain.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_gain */

int unemap_load_voltage_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_voltages,float voltages_per_second,float *voltages,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

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
{
	int return_code;
#if defined (NI_DAQ)
	int *channel_number,i,*load_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_load_voltage_stimulating);
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"unemap_load_voltage_stimulating %d %p %d %g %p %u %p %p\n",
			number_of_channels,channel_numbers,number_of_voltages,voltages_per_second,
			voltages,number_of_cycles,stimulation_end_callback,
			stimulation_end_callback_data);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0==number_of_channels)||((0<number_of_channels)&&channel_numbers))
	{
		return_code=1;
		i=number_of_channels;
		channel_number=channel_numbers;
		while (return_code&&(i>0))
		{
			if ((0< *channel_number)&&
				((((*channel_number)-1)/NUMBER_OF_CHANNELS_ON_NI_CARD<
				module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[
				((*channel_number)-1)/NUMBER_OF_CHANNELS_ON_NI_CARD].
				hardware_buffer_offsets)[((*channel_number)-1)%
				NUMBER_OF_CHANNELS_ON_NI_CARD])))
			{
				i--;
				channel_number++;
			}
			else
			{
				return_code=0;
			}
		}
	}
	if (return_code&&((0==number_of_voltages)||
		((1==number_of_voltages)&&voltages)||((1<number_of_voltages)&&voltages&&
		(0<voltages_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			if (ALLOCATE(load_card,int,module_number_of_NI_CARDS))
			{
				if (0==number_of_channels)
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						if (0<module_NI_CARDS[i].number_of_configured_channels)
						{
							load_card[i]=1;
						}
						else
						{
							load_card[i]=0;
						}
					}
				}
				else
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						load_card[i]=0;
					}
					for (i=0;i<number_of_channels;i++)
					{
						load_card[(channel_numbers[i]-1)/NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
					}
				}
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					if (load_card[i]&&((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
					{
						if ((UnEmap_2V1==module_NI_CARDS[i].unemap_hardware_version)||
							(UnEmap_2V2==module_NI_CARDS[i].unemap_hardware_version))
						{
							/* make sure that constant voltage */
							set_shift_register(module_NI_CARDS+i,
								Stim_Source_SHIFT_REGISTER_UnEmap2vx,0,1);
						}
					}
				}
				return_code=load_NI_DA(STIMULATE_CHANNEL,number_of_channels,
					channel_numbers,number_of_voltages,voltages_per_second,voltages,
					number_of_cycles,stimulation_end_callback,
					stimulation_end_callback_data);
				DEALLOCATE(load_card);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_load_voltage_stimulating.  Could not allocate load_card");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_voltage_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_load_voltage_stimulating.  Invalid arguments %d %p %d %g %p",
			number_of_channels,channel_numbers,number_of_voltages,voltages_per_second,
			voltages);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_voltages;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,
			"unemap_load_voltage_stimulating %d %p %d %g %p %d %d\n",
			number_of_channels,channel_numbers,number_of_voltages,voltages_per_second,
			voltages,number_of_cycles,return_code);
		if ((0<number_of_channels)&&channel_numbers)
		{
			fprintf(unemap_debug,"  channels:");
			for (i=0;i<number_of_channels;i++)
			{
				fprintf(unemap_debug," %d",channel_numbers[i]);
			}
			fprintf(unemap_debug,"\n");
		}
		maximum_voltages=10;
		if (number_of_voltages<maximum_voltages)
		{
			maximum_voltages=number_of_voltages;
		}
		for (i=0;i<maximum_voltages;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,voltages[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_load_voltage_stimulating */

int unemap_load_current_stimulating(int number_of_channels,int *channel_numbers,
	int number_of_currents,float currents_per_second,float *currents,
	unsigned int number_of_cycles,
	Unemap_stimulation_end_callback *stimulation_end_callback,
	void *stimulation_end_callback_data)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

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
{
	int return_code;
#if defined (NI_DAQ)
	float current;
	int *channel_number,i,*load_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_load_current_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if ((0==number_of_channels)||((0<number_of_channels)&&channel_numbers))
	{
		return_code=1;
		i=number_of_channels;
		channel_number=channel_numbers;
		while (return_code&&(i>0))
		{
			if ((0< *channel_number)&&
				((((*channel_number)-1)/NUMBER_OF_CHANNELS_ON_NI_CARD<
				module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[
				((*channel_number)-1)/NUMBER_OF_CHANNELS_ON_NI_CARD].
				hardware_buffer_offsets)[((*channel_number)-1)%
				NUMBER_OF_CHANNELS_ON_NI_CARD])))
			{
				i--;
				channel_number++;
			}
			else
			{
				return_code=0;
			}
		}
	}
	if (return_code&&((0==number_of_currents)||
		((1==number_of_currents)&&currents)||((1<number_of_currents)&&currents&&
		(0<currents_per_second))))
	{
		if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			/* change the currents into voltages */
			for (i=0;i<number_of_currents;i++)
			{
				current=currents[i];
				if ((float)1<current)
				{
					current=(float)1;
				}
				else
				{
					if ((float)-1>current)
					{
						current=(float)-1;
					}
				}
				currents[i]=current*MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx;
			}
			if (ALLOCATE(load_card,int,module_number_of_NI_CARDS))
			{
				if (0==number_of_channels)
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						if (0<module_NI_CARDS[i].number_of_configured_channels)
						{
							load_card[i]=1;
						}
						else
						{
							load_card[i]=0;
						}
					}
				}
				else
				{
					for (i=0;i<module_number_of_NI_CARDS;i++)
					{
						load_card[i]=0;
					}
					for (i=0;i<number_of_channels;i++)
					{
						load_card[(channel_numbers[i]-1)/NUMBER_OF_CHANNELS_ON_NI_CARD]=1;
					}
				}
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					if (load_card[i]&&((PCI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6031E_AD_DA==(module_NI_CARDS[i]).type)||
						(PXI6071E_AD_DA==(module_NI_CARDS[i]).type)))
					{
						if ((UnEmap_2V1==module_NI_CARDS[i].unemap_hardware_version)||
							(UnEmap_2V2==module_NI_CARDS[i].unemap_hardware_version))
						{
							/* make sure that constant current */
							set_shift_register(module_NI_CARDS+i,
								Stim_Source_SHIFT_REGISTER_UnEmap2vx,1,1);
						}
					}
				}
				return_code=load_NI_DA(STIMULATE_CHANNEL,number_of_channels,
					channel_numbers,number_of_currents,currents_per_second,currents,
					number_of_cycles,stimulation_end_callback,
					stimulation_end_callback_data);
				/* set the desired currents to the actual currents used */
				for (i=0;i<number_of_currents;i++)
				{
					currents[i] /= MAXIMUM_CURRENT_STIMULATOR_VOLTAGE_UnEmap2vx;
				}
				DEALLOCATE(load_card);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"unemap_load_current_stimulating.  Could not allocate load_card");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_load_current_stimulating.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_load_current_stimulating.  Invalid arguments %d %p %d %g %p",
			number_of_channels,channel_numbers,number_of_currents,currents_per_second,
			currents);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_currents;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_load_current_stimulating %d %p %d %g %p %d\n",
			number_of_channels,channel_numbers,number_of_currents,currents_per_second,
			currents,return_code);
		maximum_currents=10;
		if (number_of_currents<maximum_currents)
		{
			maximum_currents=number_of_currents;
		}
		for (i=0;i<maximum_currents;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,currents[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_load_current_stimulating */

int unemap_start_stimulating(void)
/*******************************************************************************
LAST MODIFIED : 5 June 2000

DESCRIPTION :
The function fails if the hardware is not configured.

Starts stimulation for all channels that have been loaded (with
unemap_load_voltage_stimulating or unemap_load_current_stimulating) and have not
yet started.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_start_stimulating);
#if defined (NI_DAQ)
	return_code=start_NI_DA();
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_start_stimulating */

int unemap_stop_stimulating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops stimulating for the channels in the group.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_stimulating);
#if defined (NI_DAQ)
	return_code=stop_NI_DA(STIMULATE_CHANNEL,channel_number);
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_stop_stimulating %d %d\n",channel_number,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_stimulating */

int unemap_set_channel_stimulating(int channel_number,int stimulating)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Zero <stimulating> means off.  Non-zero <stimulating> means on.  If
<channel_number> is valid (between 1 and the total number of channels
inclusive), then <channel_number> is set to <stimulating>.  If <channel_number>
is 0, then all channels are set to <stimulating>.  Otherwise the function fails.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i,j,k;
	struct NI_card *ni_card;
	unsigned char stimulate_mode;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_channel_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&(ni_card=module_NI_CARDS)&&
		(0<module_number_of_NI_CARDS))
	{
		if ((0<=channel_number)&&((0==channel_number)||
			((((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD]))))
		{
			return_code=1;
			if (0==channel_number)
			{
				for (i=module_number_of_NI_CARDS;i>0;i--)
				{
					if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
						(UnEmap_2V2==ni_card->unemap_hardware_version))
					{
						if ((ni_card->unemap_2vx).isolate_mode)
						{
							for (j=0;j<NUMBER_OF_CHANNELS_ON_NI_CARD;j++)
							{
								k=(ni_card->channel_reorder)[j];
								stimulate_mode=(unsigned char)0x1;
								stimulate_mode <<= k%8;
								if (((ni_card->hardware_buffer_offsets)[j]>=0)&&
									((stimulating&&relay_power_on_UnEmap2vx)||
									(!stimulating&&!relay_power_on_UnEmap2vx)))
								{
									((ni_card->unemap_2vx).stimulate_channels)[k/8] |=
										stimulate_mode;
								}
								else
								{
									((ni_card->unemap_2vx).stimulate_channels)[k/8] &=
										~stimulate_mode;
								}
							}
						}
						else
						{
							for (j=NUMBER_OF_CHANNELS_ON_NI_CARD-1;j>0;j--)
							{
								k=Stim_Base_SHIFT_REGISTER_UnEmap2vx+
									(ni_card->channel_reorder)[j];
								if ((ni_card->hardware_buffer_offsets)[j]>=0)
								{
									set_shift_register(ni_card,k,!stimulating,0);
								}
								else
								{
									set_shift_register(ni_card,k,1,0);
								}
							}
							k=Stim_Base_SHIFT_REGISTER_UnEmap2vx+
								(ni_card->channel_reorder)[0];
							if ((ni_card->hardware_buffer_offsets)[0]>=0)
							{
								set_shift_register(ni_card,k,!stimulating,1);
							}
							else
							{
								set_shift_register(ni_card,k,1,1);
							}
						}
					}
					ni_card++;
				}
			}
			else
			{
				ni_card += (channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
				k=(ni_card->channel_reorder)[(channel_number-1)%
					NUMBER_OF_CHANNELS_ON_NI_CARD];
				if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
					(UnEmap_2V2==ni_card->unemap_hardware_version))
				{
					if ((ni_card->unemap_2vx).isolate_mode)
					{
						i=k/8;
						stimulate_mode=(unsigned char)0x1;
						stimulate_mode <<= k%8;
						if ((stimulating&&relay_power_on_UnEmap2vx)||
							(!stimulating&&!relay_power_on_UnEmap2vx))
						{
							((ni_card->unemap_2vx).stimulate_channels)[i] |= stimulate_mode;
						}
						else
						{
							((ni_card->unemap_2vx).stimulate_channels)[i] &= ~stimulate_mode;
						}
					}
					else
					{
						k += Stim_Base_SHIFT_REGISTER_UnEmap2vx;
						set_shift_register(ni_card,k,!stimulating,1);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_set_channel_stimulating.  Invalid channel_number");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_channel_stimulating.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_set_channel_stimulating %d %d %d\n",
			channel_number,stimulating,return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_set_channel_stimulating */

int unemap_get_channel_stimulating(int channel_number,int *stimulating)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is valid (between 1 and the total number of channels
inclusive), then <stimulating> is set to 1 if <channel_number> is stimulating
and 0 otherwise.  Otherwise the function fails.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int byte_number,k;
	struct NI_card *ni_card;
	unsigned char bit_mask;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_channel_stimulating);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&(ni_card=module_NI_CARDS)&&
		(0<module_number_of_NI_CARDS))
	{
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&stimulating)
		{
			return_code=1;
			ni_card += (channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD;
			k=Stim_Base_SHIFT_REGISTER_UnEmap2vx+(ni_card->channel_reorder)[
				(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD];
			byte_number=(k-1)/8;
			bit_mask=(unsigned char)0x1;
			bit_mask <<= (k-1)%8;
			if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
			{
				*stimulating=0;
			}
			else
			{
				*stimulating=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_channel_stimulating.  Invalid argument(s).  %d %p",
				channel_number,stimulating);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_channel_stimulating.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_channel_stimulating */

int unemap_start_calibrating(int channel_number,int number_of_voltages,
	float voltages_per_second,float *voltages)
/*******************************************************************************
LAST MODIFIED : 11 August 2003

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
{
	int return_code;
#if defined (NI_DAQ)
	int number_of_channels;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_start_calibrating);
	return_code=0;
#if defined (NI_DAQ)
	if (0==channel_number)
	{
		number_of_channels=0;
	}
	else
	{
		number_of_channels=1;
	}
	if (stop_NI_DA(CALIBRATE_CHANNEL,channel_number))
	{
		if (load_NI_DA(CALIBRATE_CHANNEL,number_of_channels,&channel_number,
			number_of_voltages,voltages_per_second,voltages,0,
			(Unemap_stimulation_end_callback *)NULL,(void *)NULL))
		{
			if (start_NI_DA())
			{
				return_code=1;
			}
		}
	}
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;
	int i,maximum_voltages;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_start_calibrating %d %d %g %p %d\n",
			channel_number,number_of_voltages,voltages_per_second,voltages,
			return_code);
		maximum_voltages=10;
		if (number_of_voltages<maximum_voltages)
		{
			maximum_voltages=number_of_voltages;
		}
		for (i=0;i<maximum_voltages;i++)
		{
			fprintf(unemap_debug,"  %d %g\n",i,voltages[i]);
		}
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_start_calibrating */

int unemap_stop_calibrating(int channel_number)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
The function fails if the hardware is not configured.

If <channel_number> is between 1 and the total number of channels inclusive,
then the function applies to the group ((<channel_number>-1) div 64)*64+1 to
((<channel_number>-1) div 64)*64+64.  If <channel_number> is 0 then the
function applies to the group of all channels.  Otherwise, the function fails.

Stops generating the calibration signal for the channels in the group.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_stop_calibrating);
#if defined (NI_DAQ)
	return_code=stop_NI_DA(CALIBRATE_CHANNEL,channel_number);
#else /* defined (NI_DAQ) */
	return_code=0;
#endif /* defined (NI_DAQ) */
#if defined (DEBUG)
/*???debug */
{
	FILE *unemap_debug;

	if (unemap_debug=fopen_UNEMAP_HARDWARE("unemap.deb","a"))
	{
		fprintf(unemap_debug,"unemap_stop_calibrating %d %d\n",channel_number,
			return_code);
		fclose(unemap_debug);
	}
}
#endif /* defined (DEBUG) */
	LEAVE;

	return (return_code);
} /* unemap_stop_calibrating */

int unemap_set_power(int on)
/*******************************************************************************
LAST MODIFIED : 15 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

If <on> is zero the hardware is powered off, otherwise the hardware is powered
on.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float frequency;
	int channel_number,i;
#if defined (WIN32_SYSTEM)
	DWORD batt_good_thread_id;
	HANDLE batt_good_thread;
	static int stop_flag=1;
#endif /* defined (WIN32_SYSTEM) */
#if defined (NEW_CODE)
	i16 status;
	u32 pattern_and_lines;
#endif /* defined (NEW_CODE) */
#endif /* defined (NI_DAQ) */

	ENTER(unemap_set_power);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
			(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
		{
			return_code=1;
			if (on)
			{
				if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
				{
					DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
						battery_A_line_UnEmap2v1,(i16)1);
				}
				else
				{
#if defined (OLD_CODE)
					/* turn on the mechanical relays */
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,1,
						1);
					/* then turn on the solid state relays */
					set_shift_register(module_NI_CARDS,BattB_SHIFT_REGISTER_UnEmap2vx,1,
						1);
#endif /* defined (OLD_CODE) */
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,1,
						1);
				}
#if defined (NEW_CODE)
				if (UnEmap_2V1!=module_NI_CARDS->unemap_hardware_version)
				{
					/* set up watching of BattGood */
					pattern_and_lines=0x1;
					pattern_and_lines <<= battery_good_input_line_UnEmap2vx;
					status=DIG_Trigger_Config(module_NI_CARDS->device_number,
						/*group*/(i16)1,
						/*software start trigger*/(i16)0,
						/*active high start polarity*/(i16)0,
						/*digital pattern stop trigger*/(i16)2,
						/*pattern not matched stop polarity*/(i16)3,
						/*points after stop trigger*/(u32)0,
						/*pattern to be matched*/pattern_and_lines,
						/*DIO lines to be compared with pattern*/pattern_and_lines);
					status=Config_DAQ_Event_Message(module_NI_CARDS->device_number,
						/*add message*/(i16)1,
						/*DIO port 0*/"DI0",
						/*digital pattern not matched*/(i16)7,
						/*DIO lines to be compared with pattern*/pattern_and_lines,
						/*pattern to be matched*/pattern_and_lines,
						/*number of triggers to skip*/(u32)0,
						/*number of scans before trigger event*/(u32)0,
						/*number of scans after trigger event*/(u32)0,
						/*handle*/(i16)0,
						/*message*/(i16)0,
						(u32)batt_good_callback);
				}
#endif /* defined (NEW_CODE) */
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					/* set power light on (and set other shift registers) */
					if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,1,1);
					}
					else
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,0,1);
					}
					if (module_configured)
					{
						channel_number=i*NUMBER_OF_CHANNELS_ON_NI_CARD+1;
						/* force a reset of the filter frequency */
						unemap_get_antialiasing_filter_frequency(channel_number,
							&frequency);
						module_NI_CARDS[i].anti_aliasing_filter_taps= -1;
						unemap_set_antialiasing_filter_frequency(channel_number,
							frequency);
					}
				}
				BattA_setting_UnEmap2vx=1;
				if (UnEmap_2V2==module_NI_CARDS->unemap_hardware_version)
				{
#if defined (WIN32_SYSTEM)
					/* start watching of BattGood */
					if (stop_flag)
					{
						stop_flag=0;
#if !defined (NO_BATT_GOOD_WATCH)
						batt_good_thread=CreateThread(/*no security attributes*/NULL,
							/*use default stack size*/0,batt_good_thread_function,
							(LPVOID)&stop_flag,/*use default creation flags*/0,
							&batt_good_thread_id);
#endif /* !defined (NO_BATT_GOOD_WATCH) */
					}
#endif /* defined (WIN32_SYSTEM) */
				}
			}
			else
			{
#if defined (WIN32_SYSTEM)
				/* stop watching of BattGood */
				if (!stop_flag)
				{
					stop_flag=1;
				}
#endif /* defined (WIN32_SYSTEM) */
				for (i=0;i<module_number_of_NI_CARDS;i++)
				{
					/* set power light off */
					if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,0,1);
					}
					else
					{
						set_shift_register(module_NI_CARDS+i,
							PC_LED_SHIFT_REGISTER_UnEmap2vx,1,1);
					}
				}
				if (UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)
				{
					DIG_Out_Line(module_NI_CARDS->device_number,(i16)0,
						battery_A_line_UnEmap2v1,(i16)0);
				}
				else
				{
#if defined (OLD_CODE)
					/* then turn off the solid state relays */
					set_shift_register(module_NI_CARDS,BattB_SHIFT_REGISTER_UnEmap2vx,0,
						1);
					/* turn off the mechanical relays */
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,0,
						1);
#endif /* defined (OLD_CODE) */
					set_shift_register(module_NI_CARDS,BattA_SHIFT_REGISTER_UnEmap2vx,0,
						1);
				}
				BattA_setting_UnEmap2vx=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_set_power.  Invalid configuration.  %d %p %d",module_configured,
			module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_set_power */

int unemap_get_power(int *on)
/*******************************************************************************
LAST MODIFIED : 16 September 1999

DESCRIPTION :
The function does not need the hardware to be configured.

If the hardware power is on then <*on> is set to 1, otherwise <*on> is set to 0.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_power);
	return_code=0;
#if defined (NI_DAQ)
	if (on)
	{
		if (search_for_NI_cards()&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
		{
			if ((UnEmap_2V1==module_NI_CARDS->unemap_hardware_version)||
				(UnEmap_2V2==module_NI_CARDS->unemap_hardware_version))
			{
				return_code=1;
				if (BattA_setting_UnEmap2vx)
				{
					*on=1;
				}
				else
				{
					*on=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_get_power.  Invalid configuration.  %d %p %d",
				module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unemap_get_power.  Missing on");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_power */

int unemap_read_waveform_file(FILE *in_file,char *waveform_file_name,
	int *number_of_values,float *values_per_second,float **values,
	int *constant_voltage)
/*******************************************************************************
LAST MODIFIED : 7 January 2001

DESCRIPTION :
The function does not need the hardware to be configured.

A waveform suitable for stimulation is read from a file.  If
<*constant_voltage>, then the <*values> are voltages otherwise the <*values> are
proportions of the maximum current.
???DB.  Same as in unemap_hardware.c .  Could read on remote machine ?
==============================================================================*/
{
	int return_code;
	FILE *waveform_file;
	float *value;
	int i;

	ENTER(unemap_read_waveform_file);
	return_code=0;
	/* check arguments */
	if (((in_file&&!waveform_file_name)||(!in_file&&waveform_file_name))&&
		number_of_values&&values&&values_per_second&&constant_voltage)
	{
		if (in_file)
		{
			waveform_file=in_file;
		}
		else
		{
			waveform_file=fopen(waveform_file_name,"r");
		}
		if (waveform_file)
		{
			fscanf(waveform_file,"Number of ");
			if (1==fscanf(waveform_file,"voltages = %d ",
				number_of_values))
			{
				*constant_voltage=1;
				return_code=1;
			}
			else
			{
				if (1==fscanf(waveform_file,"currents = %d ",
					number_of_values))
				{
					*constant_voltage=0;
					return_code=1;
				}
			}
			if (return_code)
			{
				return_code=0;
				if (0< *number_of_values)
				{
					if (1==fscanf(waveform_file,"Frequency (pts/s) = %f ",
						values_per_second))
					{
						if (0< *values_per_second)
						{
							if (ALLOCATE(*values,float,*number_of_values))
							{
								if (*constant_voltage)
								{
									fscanf(waveform_file," Voltages ");
								}
								else
								{
									fscanf(waveform_file," Currents ");
								}
								i= *number_of_values;
								return_code=1;
								value= *values;
								while (return_code&&(i>0))
								{
									if (1==fscanf(waveform_file," %f ",value))
									{
										i--;
										value++;
									}
									else
									{
										return_code=0;
									}
								}
								if (0==return_code)
								{
									display_message(ERROR_MESSAGE,"Error reading value from %s",
										waveform_file_name);
									DEALLOCATE(*values);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"unemap_read_waveform_file.  Could not allocate memory for values");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid frequency (%g) from %s",
								*values_per_second,waveform_file_name);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Error reading frequency from %s",
							waveform_file_name);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Invalid number of values (%d) from %s",*number_of_values,
						waveform_file_name);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Error reading number of values from %s",waveform_file_name);
			}
			if (!in_file)
			{
				fclose(waveform_file);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Could not open waveform file %s",
				waveform_file_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_read_waveform_file.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* unemap_read_waveform_file */

int unemap_get_number_of_stimulators(int *number_of_stimulators)
/*******************************************************************************
LAST MODIFIED : 12 July 1999

DESCRIPTION :
The function does not need the hardware to be configured.

The total number of hardware stimulators is assigned to
<*number_of_stimulators>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_get_number_of_stimulators);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (number_of_stimulators)
	{
		return_code=1;
		if (search_for_NI_cards()&&module_NI_CARDS)
		{
			*number_of_stimulators=module_number_of_stimulators;
		}
		else
		{
			*number_of_stimulators=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_number_of_stimulators.  Missing number_of_stimulators");
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_number_of_stimulators */

int unemap_channel_valid_for_stimulator(int stimulator_number,
	int channel_number)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function does not need the hardware to be configured.

Returns non-zero if <stimulator_number> can stimulate down <channel_number>.
==============================================================================*/
{
	int return_code;

	ENTER(unemap_channel_valid_for_stimulator);
	return_code=0;
#if defined (NI_DAQ)
	if (search_for_NI_cards()&&module_NI_CARDS&&(0<stimulator_number)&&
		(stimulator_number<=module_number_of_stimulators)&&(0<channel_number)&&
		(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
		module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
		NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
		(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&
		(module_stimulator_NI_CARD_indices[stimulator_number-1]==
		(channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD))
	{
		return_code=1;
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_channel_valid_for_stimulator */

/*
Diagnostic functions
--------------------
*/
int unemap_get_card_state(int channel_number,int *battA_state,
	int *battGood_state,float *filter_frequency,int *filter_taps,
	unsigned char shift_registers[10],int *GA0_state,int *GA1_state,
	int *NI_gain,int *input_mode,int *polarity,float *tol_settling_address,
	int *sampling_interval,int *settling_step_max_address)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Returns the current state of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int i;
	i16 state,status;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_get_card_state);
	return_code=0;
#if defined (NI_DAQ)
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		/* check arguments */
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&battA_state&&
			battGood_state&&filter_frequency&&filter_taps&&shift_registers&&
			GA0_state&&GA1_state&&NI_gain&&input_mode&&polarity&&
			tol_settling_address&&sampling_interval&&settling_step_max_address)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			*battA_state=BattA_setting_UnEmap2vx;
			status=DIG_In_Line(ni_card->device_number,0,
				battery_good_input_line_UnEmap2vx,&state);
			if (state)
			{
				*battGood_state=1;
			}
			else
			{
				*battGood_state=0;
			}
			for (i=0;i<10;i++)
			{
				shift_registers[i]=((ni_card->unemap_2vx).shift_register)[i];
			}
			if ((ni_card->unemap_2vx).gain_output_line_0)
			{
				*GA0_state=1;
			}
			else
			{
				*GA0_state=0;
			}
			if ((ni_card->unemap_2vx).gain_output_line_1)
			{
				*GA1_state=1;
			}
			else
			{
				*GA1_state=0;
			}
			*NI_gain=(int)(ni_card->gain);
			unemap_get_antialiasing_filter_frequency(channel_number,filter_frequency);
			*filter_taps=ni_card->anti_aliasing_filter_taps;
			*input_mode=ni_card->input_mode;
			*polarity=ni_card->polarity;
			*tol_settling_address=tol_settling;
			*sampling_interval=ni_card->sampling_interval;
			*settling_step_max_address=settling_step_max;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unemap_get_card_state.  "
				"Invalid argument(s).  %d %p %p %p %p %p %p %p %p %p %p %p %p %p",
				channel_number,battA_state,battGood_state,filter_frequency,filter_taps,
				shift_registers,GA0_state,GA1_state,NI_gain,input_mode,polarity,
				tol_settling,sampling_interval,settling_step_max);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_card_state.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_get_card_state */

int unemap_write_card_state(int channel_number,FILE *out_file)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Writes the current state of the signal conditioning card containing the
<channel_number> to the specified <out_file>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	float filter_frequency,pre_filter_gain,post_filter_gain;
	int i;
	struct NI_card *ni_card;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_write_card_state);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&out_file)
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			switch (ni_card->unemap_hardware_version)
			{
				case UnEmap_1V2:
				{
					fprintf(out_file,"cs = %d\n",chip_select_line_UnEmap1vx);
					fprintf(out_file,"u/d = %d\n",up_down_line_UnEmap1vx);
					fprintf(out_file,"inc = %d\n",filter_increment_output_line);
					fprintf(out_file,"led = %d\n",led_line_UnEmap1vx);
					fprintf(out_file,"id1 = %d\n",id_line_1_UnEmap1vx);
					fprintf(out_file,"id2 = %d\n",id_line_2_UnEmap1vx);
					fprintf(out_file,"id3 = %d\n",id_line_3_UnEmap1vx);
					unemap_get_antialiasing_filter_frequency(channel_number,
						&filter_frequency);
					fprintf(out_file,"filter = %g\n",filter_frequency);
					unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
					fprintf(out_file,"gain = %g\n",post_filter_gain/10.);
					fprintf(out_file,"input_mode = %d\n",ni_card->input_mode);
					fprintf(out_file,"polarity = %d\n",ni_card->polarity);
				} break;
				case UnEmap_2V1:
				case UnEmap_2V2:
				{
					fprintf(out_file,"BattGood = %d\n",battery_good_input_line_UnEmap2vx);
					fprintf(out_file,"Rstrobe = %d\n",
						shift_register_strobe_output_line_UnEmap2vx);
					fprintf(out_file,"GA1 = %d\n",gain_1_output_line_UnEmap2vx);
					fprintf(out_file,"Rdata = %d\n",
						shift_register_data_output_line_UnEmap2vx);
					fprintf(out_file,"FINC = %d\n",filter_increment_output_line);
					fprintf(out_file,"RCLK = %d\n",
						shift_register_clock_output_line_UnEmap2vx);
					fprintf(out_file,"GA0 = %d\n",gain_0_output_line_UnEmap2vx);
					unemap_get_antialiasing_filter_frequency(channel_number,
						&filter_frequency);
					fprintf(out_file,"filter = %g\n",filter_frequency);
					unemap_get_gain(channel_number,&pre_filter_gain,&post_filter_gain);
					fprintf(out_file,"gain = %g\n",post_filter_gain/11.);
					fprintf(out_file,"input_mode = %d\n",ni_card->input_mode);
					fprintf(out_file,"polarity = %d\n",ni_card->polarity);
					fprintf(out_file,"GA0 setting = ");
					if ((ni_card->unemap_2vx).gain_output_line_0)
					{
						fprintf(out_file,"1\n");
					}
					else
					{
						fprintf(out_file,"0\n");
					}
					fprintf(out_file,"GA1 setting = ");
					if ((ni_card->unemap_2vx).gain_output_line_1)
					{
						fprintf(out_file,"1\n");
					}
					else
					{
						fprintf(out_file,"0\n");
					}
					fprintf(out_file,"shift registers = ");
					for (i=0;i<10;i++)
					{
						fprintf(out_file,"%02x",
							(unsigned short)((ni_card->unemap_2vx).shift_register)[i]);
					}
					fprintf(out_file,"\n");
					fprintf(out_file,"settling magnitude = %g\n",tol_settling);
					fprintf(out_file,"relay power on = %d\n",relay_power_on_UnEmap2vx);
					fprintf(out_file,"number of valid ni channels = %d\n",
						number_of_valid_ni_channels);
					if (UnEmap_2V1==ni_card->unemap_hardware_version)
					{
						fprintf(out_file,"BattA = %d\n",battery_A_line_UnEmap2v1);
					}
					else
					{
						fprintf(out_file,"Master = %d\n",master_line_UnEmap2v2);
					}
					fprintf(out_file,"BattA setting = %d\n",BattA_setting_UnEmap2vx);
					fprintf(out_file,"sampling interval = %d\n",
						ni_card->sampling_interval);
					fprintf(out_file,"max settling steps = %d\n",settling_step_max);
				} break;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_write_card_state.  Invalid argument(s).  %d %p",
				channel_number,out_file);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_get_card_state.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_write_card_state */

int unemap_toggle_shift_register(int channel_number,int register_number)
/*******************************************************************************
LAST MODIFIED : 8 August 2003

DESCRIPTION :
The function fails if the hardware is not configured.

Toggles the <shift_register> of the signal conditioning card containing the
<channel_number>.

Intended for diagnostic use only.
==============================================================================*/
{
	int return_code;
#if defined (NI_DAQ)
	int byte_number;
	struct NI_card *ni_card;
	unsigned char bit_mask;
#endif /* defined (NI_DAQ) */

	ENTER(unemap_toggle_shift_register);
	return_code=0;
#if defined (NI_DAQ)
	/* check arguments */
	if (module_configured&&module_NI_CARDS&&(0<module_number_of_NI_CARDS))
	{
		if ((0<channel_number)&&(((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD)<
			module_number_of_NI_CARDS)&&(0<=(module_NI_CARDS[(channel_number-1)/
			NUMBER_OF_CHANNELS_ON_NI_CARD].hardware_buffer_offsets)[
			(channel_number-1)%NUMBER_OF_CHANNELS_ON_NI_CARD])&&(0<register_number)&&
			(register_number<=80))
		{
			ni_card=module_NI_CARDS+
				((channel_number-1)/NUMBER_OF_CHANNELS_ON_NI_CARD);
			if ((UnEmap_2V1==ni_card->unemap_hardware_version)||
				(UnEmap_2V2==ni_card->unemap_hardware_version))
			{
				bit_mask=0x1;
				bit_mask <<= (register_number-1)%8;
				byte_number=(register_number-1)/8;
				if (((ni_card->unemap_2vx).shift_register)[byte_number]&bit_mask)
				{
					set_shift_register(ni_card,register_number,0,1);
				}
				else
				{
					set_shift_register(ni_card,register_number,1,1);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"unemap_toggle_shift_register.  Invalid argument(s).  %d %d",
				channel_number,register_number);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"unemap_toggle_shift_register.  Invalid configuration.  %d %p %d",
			module_configured,module_NI_CARDS,module_number_of_NI_CARDS);
	}
#endif /* defined (NI_DAQ) */
	LEAVE;

	return (return_code);
} /* unemap_toggle_shift_register */
