/*******************************************************************************
FILE : unemap_hardware_service.h

LAST MODIFIED : 4 June 2003

DESCRIPTION :
Instruction codes for the unemap service which runs under NT and talks to unemap
via sockets.

Biggest code is 0x31
==============================================================================*/
#if !defined (UNEMAP_HARDWARE_SERVICE)
#define UNEMAP_HARDWARE_SERVICE

/*
Global constants
----------------
*/

/* instruction codes */
#define UNEMAP_CALIBRATE_CODE ((unsigned char)0x01)
#define UNEMAP_CHANNEL_VALID_FOR_STIMULATOR_CODE ((unsigned char)0x02)
#define UNEMAP_CLEAR_SCROLLING_CHANNELS_CODE ((unsigned char)0x03)
#define UNEMAP_CONFIGURE_CODE ((unsigned char)0x04)
#define UNEMAP_CONFIGURED_CODE ((unsigned char)0x05)
#define UNEMAP_DECONFIGURE_CODE ((unsigned char)0x06)
#define UNEMAP_GET_ANTIALIASING_FILTER_FREQUENCY_CODE ((unsigned char)0x07)
#define UNEMAP_GET_CHANNEL_STIMULATING_CODE ((unsigned char)0x08)
#define UNEMAP_GET_GAIN_CODE ((unsigned char)0x09)
#define UNEMAP_GET_HARDWARE_VERSION_CODE ((unsigned char)0x0a)
#define UNEMAP_GET_ISOLATE_RECORD_MODE_CODE ((unsigned char)0x0b)
#define UNEMAP_GET_MAXIMUM_NUMBER_OF_SAMPLES_CODE ((unsigned char)0x0c)
#define UNEMAP_GET_NUMBER_OF_CHANNELS_CODE ((unsigned char)0x0d)
#define UNEMAP_GET_NUMBER_OF_SAMPLES_ACQUIRED_CODE ((unsigned char)0x0e)
#define UNEMAP_GET_NUMBER_OF_STIMULATORS_CODE ((unsigned char)0x0f)
#define UNEMAP_GET_POWER_CODE ((unsigned char)0x10)
#define UNEMAP_GET_SAMPLE_RANGE_CODE ((unsigned char)0x11)
#define UNEMAP_GET_SAMPLES_ACQUIRED_CODE ((unsigned char)0x12)
#define UNEMAP_GET_SAMPLES_ACQUIRED_BACKGROUND_CODE ((unsigned char)0x2d)
#define UNEMAP_GET_SAMPLING_CODE ((unsigned char)0x2e)
#define UNEMAP_GET_SAMPLING_FREQUENCY_CODE ((unsigned char)0x13)
#define UNEMAP_GET_SCROLLING_CALLBACK_FREQUENCY_CODE ((unsigned char)0x30)
#define UNEMAP_GET_SCROLLING_FREQUENCY_CODE ((unsigned char)0x31)
#define UNEMAP_GET_SOFTWARE_VERSION_CODE ((unsigned char)0x2f)
/*???DB.  Keep old name (SERVICE) so that old service/client will still
	compile */
#define UNEMAP_GET_SERVICE_VERSION_CODE ((unsigned char)0x2f)
#define UNEMAP_GET_VOLTAGE_RANGE_CODE ((unsigned char)0x14)
#define UNEMAP_LOAD_CURRENT_STIMULATING_CODE ((unsigned char)0x2a)
#define UNEMAP_LOAD_VOLTAGE_STIMULATING_CODE ((unsigned char)0x2b)
#define UNEMAP_READ_WAVEFORM_FILE_CODE ((unsigned char)0x15)
#define UNEMAP_SET_ANTIALIASING_FILTER_FREQUENCY_CODE ((unsigned char)0x16)
#define UNEMAP_SET_CHANNEL_STIMULATING_CODE ((unsigned char)0x17)
#define UNEMAP_SET_GAIN_CODE ((unsigned char)0x18)
#define UNEMAP_SET_ISOLATE_RECORD_MODE_CODE ((unsigned char)0x19)
#define UNEMAP_SET_POWER_CODE ((unsigned char)0x1a)
#define UNEMAP_SET_POWERUP_ANTIALIASING_FILTER_FREQUENCY_CODE ((unsigned char)0x1b)
#define UNEMAP_SET_SCROLLING_CHANNEL_CODE ((unsigned char)0x1c)
#define UNEMAP_SHUTDOWN_CODE ((unsigned char)0x1d)
#define UNEMAP_START_CALIBRATING_CODE ((unsigned char)0x1e)
#if defined (OLD_CODE)
#define UNEMAP_START_CURRENT_STIMULATING_CODE ((unsigned char)0x1f)
#endif /* defined (OLD_CODE) */
#define UNEMAP_START_SAMPLING_CODE ((unsigned char)0x20)
#define UNEMAP_START_SCROLLING_CODE ((unsigned char)0x21)
#define UNEMAP_START_STIMULATING_CODE ((unsigned char)0x2c)
#if defined (OLD_CODE)
#define UNEMAP_START_VOLTAGE_STIMULATING_CODE ((unsigned char)0x22)
#endif /* defined (OLD_CODE) */
#define UNEMAP_STOP_CALIBRATING_CODE ((unsigned char)0x23)
#define UNEMAP_STOP_SAMPLING_CODE ((unsigned char)0x24)
#define UNEMAP_STOP_SCROLLING_CODE ((unsigned char)0x25)
#define UNEMAP_STOP_STIMULATING_CODE ((unsigned char)0x26)

/* diagnostic codes.  Not guaranteed to remain */
#define UNEMAP_GET_CARD_STATE_CODE ((unsigned char)0x27)
#define UNEMAP_TOGGLE_SHIFT_REGISTER_CODE ((unsigned char)0x28)
#define UNEMAP_WRITE_CARD_STATE_CODE ((unsigned char)0x29)

#endif /* !defined (UNEMAP_HARDWARE_SERVICE) */
