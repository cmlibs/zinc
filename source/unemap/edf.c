/*******************************************************************************
FILE : edf.c

LAST MODIFIED :11 December 2001

DESCRIPTION :
Functions for reading EDF (European Data Format) and BDF (Biosemi Data Format)
data files, as output by the Biosemi rig.
BDF files are similar EDF files, but with the following changes: 
-The data is stored as 24 bit integers rather than as 16 bit integers as in EDFs.
-The version information in the header record (the first 8 bytes) must contain
255, then the string "BIOSEMI".

==============================================================================*/
#include "unemap/edf.h"

/*
Module Constants
----------------
*/

/*
Module types
------------
*/
struct EDF_to_rig_device_map
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
Stores the corresponding array indices of the edf and rig devices.
==============================================================================*/
{
	int edf_device_index,rig_device_index;
};/* EDF_to_rig_device_map*/

struct EDF_to_rig_map
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
Stores an array of EDF_to_rig_device_maps, and it's number of elements.
==============================================================================*/
{
	int number_of_devices;
	struct EDF_to_rig_device_map *edf_to_rig_device_map;
};

struct EDF_device
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Structure for storing EDF (European Data Format) signals, in edf_header.
Stings are 1 char longer than in edf file to allow space for termination.
==============================================================================*/
{
	char label[17],transducer_type[81],physical_dim[9],prefiltering[81];
	float physical_min,physical_max;
	int digital_min,digital_max,number_of_signals,number_of_samples;
}; /*EDF_device */

struct EDF_header_record
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Structure for storing EDF (European Data Format) header.
In the edf file, everything in the header is stored  as an ascii value,
but we convert to numberical values (floats, int, etc) where appropriate.
Stings are 1 char longer than in edf file to allow space for termination.
==============================================================================*/
{
	char version[9],local_patient_id[81],local_recording_id[81],
		recording_start_time[9],recording_start_date[9];
	int bytes_in_header,number_of_records,number_of_signals;
	float record_duration;
	struct EDF_device *edf_device;
};/* EDF_header */

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
static int destroy_EDF_header_record(struct EDF_header_record **edf_header_record)
/*******************************************************************************
LAST MODIFIED : 12 October 2001.

DESCRIPTION 
Destroys an EDF_header_record,
==============================================================================*/
{
	int return_code;
	ENTER(destroy_EDF_header_record);
	if(*edf_header_record)
	{
		return_code=1;
		DEALLOCATE((*edf_header_record)->edf_device);
		DEALLOCATE(*edf_header_record);
		*edf_header_record=(struct EDF_header_record *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_EDF_header_record. Invalid argument");
		return_code=0;
	}
	return(return_code);
	LEAVE;
}/* destroy_EDF_header_record */

static struct EDF_header_record *create_EDF_header_record(int number_of_devices)
/*******************************************************************************
LAST MODIFIED : 12 October 2001.

DESCRIPTION 
Creates and returns an EDF_header_record, with <number_of_devices> devices.
If number_of_devices=0, create an empty one.
==============================================================================*/
{
	struct EDF_header_record *edf_header_record;

	ENTER(create_EDF_header_record);
	edf_header_record=(struct EDF_header_record *)NULL;
	if (ALLOCATE(edf_header_record,struct EDF_header_record,1))
	{
		edf_header_record->bytes_in_header=0;
		edf_header_record->number_of_records=0;		
		edf_header_record->record_duration=0.0;
		edf_header_record->number_of_signals=number_of_devices;		
		if(edf_header_record->number_of_signals)
		{
			if(!ALLOCATE(edf_header_record->edf_device,struct EDF_device,
				edf_header_record->number_of_signals))			
			{
				display_message(ERROR_MESSAGE,
					"create_EDF_header_record. ALLOCATE edf_devices failed");
				destroy_EDF_header_record(&edf_header_record);
			}
		}
		else
		{
			edf_header_record->edf_device=(struct EDF_device *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_EDF_header_record. ALLOCATE failed");
		edf_header_record=(struct EDF_header_record *)NULL;
	}
	LEAVE;
	return(edf_header_record);
} /*create_EDF_header_record */

static int destroy_EDF_to_rig_map(struct EDF_to_rig_map **edf_to_rig_map)
/*******************************************************************************
LAST MODIFIED : 12 October 2001.

DESCRIPTION 
Destroys an EDF_to_rig_map,
==============================================================================*/
{
	int return_code;
	ENTER(destroy_EDF_to_rig_map);
	if(*edf_to_rig_map)
	{
		return_code=1;
		DEALLOCATE((*edf_to_rig_map)->edf_to_rig_device_map);
		DEALLOCATE(*edf_to_rig_map);
		*edf_to_rig_map=(struct EDF_to_rig_map *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_EDF_to_rig_map. Invalid argument");
		return_code=0;
	}
	return(return_code);
	LEAVE;
}/* destroy_EDF_to_rig_map */

static struct EDF_to_rig_map *create_EDF_to_rig_map(int number_of_maps)
/*******************************************************************************
LAST MODIFIED : 12 October 2001.

DESCRIPTION 
Creates and returns an EDF_to_rig_map, with <number_of_maps> 
EDF_to_rig_device_maps
If number_of_maps=0, create an empty one.
==============================================================================*/
{
	struct EDF_to_rig_map *edf_to_rig_map;

	ENTER(create_EDF_to_rig_map);
	edf_to_rig_map=(struct EDF_to_rig_map *)NULL;
	if (ALLOCATE(edf_to_rig_map,struct EDF_to_rig_map,1))
	{		
		edf_to_rig_map->number_of_devices=number_of_maps;		
		if(edf_to_rig_map->number_of_devices)
		{
			if(!ALLOCATE(edf_to_rig_map->edf_to_rig_device_map,
				struct EDF_to_rig_device_map,edf_to_rig_map->number_of_devices))			
			{
				display_message(ERROR_MESSAGE,
					"create_EDF_to_rig_map. ALLOCATE edf_to_rig_device_map failed");
				destroy_EDF_to_rig_map(&edf_to_rig_map);
			}
		}
		else
		{
			edf_to_rig_map->edf_to_rig_device_map=(struct EDF_to_rig_device_map *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_EDF_to_rig_map. ALLOCATE failed");
		edf_to_rig_map=(struct EDF_to_rig_map *)NULL;
	}
	LEAVE;
	return(edf_to_rig_map);
} /*create_EDF_to_rig_map */

int print_edf_header_record(struct EDF_header_record *edf_header_record)
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Prints the passed struct EDF_header_record
==============================================================================*/
{
	char *string_ptr;
	int i,return_code;

	ENTER(print_edf_header_record);
	string_ptr=(char *)NULL;
	if(edf_header_record)
	{
		return_code=1;
		printf("version: "); 
		printf(edf_header_record->version); 
		printf("\n"); 
		printf("local patient id: "); 
		printf(edf_header_record->local_patient_id); 
		printf("\n"); 	
		printf("local recording id: ");
		printf(edf_header_record->local_recording_id);
		printf("\n");
		printf("start date of recording: ");
		printf(edf_header_record->recording_start_date);
		printf("\n");
		printf("start time of recording: ");
		printf(edf_header_record->recording_start_time);  
		printf("\n");
		printf("num bytes in header record: %d",edf_header_record->bytes_in_header);		
		printf("\n");
		printf("number_of_records: %d \n",edf_header_record->number_of_records); 
		printf("sampling_duration: %f \n",edf_header_record->record_duration);
		printf("number_of_signals: %d\n",edf_header_record->number_of_signals);
		printf("\n\n");
		for(i=0;i<edf_header_record->number_of_signals;i++)
		{	
			printf("Signal number: %d\n",i);
			printf("label: ");				
			string_ptr=edf_header_record->edf_device[i].label;			
			printf(string_ptr);
			printf("\n"); 
			printf("transducer type:");
			string_ptr=edf_header_record->edf_device[i].transducer_type;
			printf(string_ptr);		
			printf("\n"); 
			printf("physical dimension:"); 
			string_ptr=edf_header_record->edf_device[i].physical_dim;
			printf(string_ptr);
			printf("\n"); 
			printf("physical minimum: %f\n",edf_header_record->edf_device[i].physical_min);
			printf("physical maximum: %f\n",edf_header_record->edf_device[i].physical_max);
			printf("digital minimum: %d\n",edf_header_record->edf_device[i].digital_min);
			printf("digital maximum: %d\n",edf_header_record->edf_device[i].digital_max);
			printf("prefiltering: "); 
			string_ptr=edf_header_record->edf_device[i].prefiltering;
			printf(string_ptr);	
			printf("\n");		
			printf("number_of_samples: %d\n",
				edf_header_record->edf_device[i].number_of_samples);
			printf("\n");	
		}
		printf("End of Header\n");	
	}
	else
	{
		display_message(ERROR_MESSAGE,
					"print_edf_header_record. invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* print_edf_header */

static struct EDF_to_rig_map *match_edf_and_rig_devices(struct Rig *rig ,
	struct EDF_header_record *edf_header_record)
/*******************************************************************************
LAST MODIFIED : 10 October 2001

DESCRIPTION :
creates, Fills in and returns an  Edf_to_rig_map, with devices that match 
from <rig> and <edf_header_record>.
If any devices are unmatched, gives a warning and prints (to stdio) names and 
numbers of unmatched devices.
==============================================================================*/
{
	char *rig_device_name,*edf_device_name,*trimmed_edf_device_name,
		*trimmed_rig_device_name;
	int i,index,j,matched,number_of_edf_devices,number_of_rig_devices,success;
	struct Device **device;
	struct EDF_to_rig_device_map *edf_to_rig_device_map;
	struct EDF_to_rig_map *edf_to_rig_map;

	ENTER(match_edf_and_rig_devices);
	device=(struct Device **)NULL;
	edf_to_rig_map=(struct EDF_to_rig_map *)NULL;
	edf_to_rig_device_map=(struct EDF_to_rig_device_map *)NULL;
	rig_device_name=(char)NULL;
	edf_device_name=(char)NULL;
	trimmed_edf_device_name=(char)NULL;
	trimmed_rig_device_name=(char)NULL;
	if(rig&&edf_header_record&&
		(number_of_rig_devices=rig->number_of_devices)&&
		(number_of_edf_devices=edf_header_record->number_of_signals))
	{
		success=0;
		device=rig->devices;		
		if(edf_to_rig_map=create_EDF_to_rig_map(0))
		{
			success=1;
			/* for all the rig_devices*/
			i=0;
			while((i<number_of_rig_devices)&&(success))
			{
				rig_device_name=(*device)->description->name;
				trimmed_rig_device_name=trim_string(rig_device_name);
				/* for all the edf_devices*/
				j=0;
				while((j<number_of_edf_devices)&&(success))
				{							
					edf_device_name=edf_header_record->edf_device[j].label;	
					trimmed_edf_device_name=trim_string(edf_device_name);
					/*see if the names match */			
					if(!strcmp(trimmed_edf_device_name,trimmed_rig_device_name))
					{
						/*we've found a match*/				
						edf_to_rig_map->number_of_devices++;
						index=edf_to_rig_map->number_of_devices-1;
						/* allocate another map*/
						if(REALLOCATE(edf_to_rig_device_map,
							edf_to_rig_map->edf_to_rig_device_map,
							struct EDF_to_rig_device_map,edf_to_rig_map->number_of_devices))
						{
							success=1;
							edf_to_rig_map->edf_to_rig_device_map=edf_to_rig_device_map;
							/* set up the map*/
							edf_to_rig_map->edf_to_rig_device_map[index].rig_device_index=i;
							edf_to_rig_map->edf_to_rig_device_map[index].edf_device_index=j;
						}
						else
						{
							success=0;
							display_message(ERROR_MESSAGE,
								"match_edf_and_rig_devices. REALLOCATE(edf_to_rig_device_map) failed");
						}
					}
					DEALLOCATE(trimmed_edf_device_name);
					j++;
				}
				DEALLOCATE(trimmed_rig_device_name);
				i++;
				device++;
			}
			/* If necessary print a list of any unmatched devices */	 		
			if(success&&(edf_to_rig_map->number_of_devices!=number_of_rig_devices)||
				(edf_to_rig_map->number_of_devices!=number_of_edf_devices))				
			{
				display_message(WARNING_MESSAGE,
					"Some unmatched devices between EDF and CNFG files");
				/* print the list to stdio*/
				printf("WARNING!\n");
				printf("Unmatched devices between EDF and CNFG files\n");
				/* find  any unmatched rig devices*/
				printf("Unmatched devices from CNFG file are: \n");
				device=rig->devices;
				for(i=0;i<number_of_rig_devices;i++)
				{
					matched=0;
					j=0;
					while((j<edf_to_rig_map->number_of_devices)&&(!matched))
					{
						if(i==edf_to_rig_map->edf_to_rig_device_map[j].rig_device_index)
						{
							matched=1;
						}
						j++;
					}	
					if(!matched)
					{
						printf("    Rig device number %d . Device name ",i);
						printf((*device)->description->name);
						printf("\n");
					}
					device++;
				}
				/* find  any unmatched edf devices*/
				printf("Unmatched devices from EDF file are: \n");
				for(i=0;i<number_of_edf_devices;i++)
				{
					matched=0;
					j=0;
					while((j<edf_to_rig_map->number_of_devices)&&(!matched))
					{
						if(i==edf_to_rig_map->edf_to_rig_device_map[j].edf_device_index)
						{
							matched=1;
						}
						j++;
					}	
					if(!matched)
					{
						printf("    EDF device number %d . Device name ",i);
						printf(edf_header_record->edf_device[i].label);
						printf("\n");
					}			
				}
			}
#if defined(DEBUG)
			for(i=0;i<edf_to_rig_map->number_of_devices;i++)
			{
				printf("Match %d: rig_device_index %d edf_device_index %d\n",i,
					edf_to_rig_map->edf_to_rig_device_map[i].rig_device_index,
					edf_to_rig_map->edf_to_rig_device_map[i].edf_device_index);
				fflush(NULL);
			}
#endif /* defined(DEBUG) */
		}
		else
		{
			success=0;
		}
		if(!success)
		{
			display_message(ERROR_MESSAGE,
				"match_edf_and_rig_devices. error!");
			destroy_EDF_to_rig_map(&edf_to_rig_map);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"match_edf_and_rig_devices. invalid argument");
		success=0;
	}
	LEAVE;
	return(edf_to_rig_map);
}/* match_edf_and_rig_devices */

static int destroy_unmatched_rig_devices(struct Rig *rig,
	struct EDF_to_rig_map *edf_to_rig_map)
/*******************************************************************************
LAST MODIFIED : 9 October 2001

DESCRIPTION :
Alter <rig>s list of devices so that only the devices referenced in 
<edf_to_rig_map> are retained.
To do this will possibly destory some of the device rigs (any unmatched ones).
==============================================================================*/
{
	int i,j,index,matched,return_code;
	struct Device **device,**config_device,**matched_device;
	struct Region *current_region;

	ENTER(destroy_unmatched_rig_devices);
	device=(struct Device **)NULL;
	config_device=(struct Device **)NULL;
	matched_device=(struct Device **)NULL;
	current_region=(struct Region *)NULL;
	if(rig&&edf_to_rig_map&&(rig->number_of_devices)&&
		(edf_to_rig_map->number_of_devices))
	{
		return_code=0;	
		/*store ptr to devices from config file */
		config_device=rig->devices;
		/*allocate mem for matched device ptrs */
		if(ALLOCATE(matched_device,struct Device *,edf_to_rig_map->number_of_devices))
		{
			return_code=1;
			/*put all the matched rig devices in place, and renumber them */
			for(i=0;i<edf_to_rig_map->number_of_devices;i++)
			{
				index=edf_to_rig_map->edf_to_rig_device_map[i].rig_device_index;
				matched_device[i]=config_device[index];
				matched_device[i]->number=i;
			}
			/*Destroy any unmatched rig devices */
			device=rig->devices;
			for(i=0;i<rig->number_of_devices;i++)
			{
				matched=0;
				j=0;
				while((j<edf_to_rig_map->number_of_devices)&&(!matched))
				{
					if(*device==matched_device[j])
					{
						matched=1;
					}
					j++;
				}	
				if(!matched)
				{
#if defined(DEBUG)
					printf("Destroying unmatched rig device: ");
					printf((*device)->description->name);
					printf("\n");
#endif /* defined(DEBUG) */
					destroy_Device(device);
				}
				device++;					
			}				
			/*make the rig devices = the matched devices*/
			if(rig->devices)
			{
				/*free old list of pointers to devices*/
				DEALLOCATE(rig->devices);
			}
			rig->devices=matched_device;
			rig->number_of_devices=edf_to_rig_map->number_of_devices;
			if(current_region=get_Rig_current_region(rig))
			{
				current_region->number_of_devices=rig->number_of_devices;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"destroy_unmatched_rig_devices. failed to allocate matched_device"); 
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_unmatched_rig_devices. invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* destroy_unmatched_rig_devices */

static float get_multiplier_from_physical_dim_string(char *physical_dim_string)
/*******************************************************************************
LAST MODIFIED : 11 October 2001

DESCRIPTION :
Look at the first (and possibly later) character  of <physical_dim_string> 
to determine and returna multiplier. This multiplier will convert to 
milli-units, as milli(V) is the base unit for unemap. 
Eg if physical_dim_string[0] = 'm' multiplier=1, 'u' -> 1/1000 etc
==============================================================================*/
{
	int success;
	float multiplier;

	ENTER(get_multiplier_from_physical_dim_string);
	success=0;
	if(physical_dim_string)
	{
		/*determine the SI multiplier to convert to mV */	
		if(physical_dim_string)
		{
			switch(physical_dim_string[0])
			{
				case 'u':
				{
					/* conv uV to mV */
					multiplier=1/1000.0;
					success=1;
				}break;
				case 'm':
				{
					/* already mV */
					multiplier=1;
					success=1;
				}break;
				case 'V':
				{
					/* conv V to mV */
					multiplier=1000;
					success=1;
				}break;
				case 'B':
				{
					if(string_matches_without_whitespace(physical_dim_string,"Boolean"))
					{
						/* ??JW what shall multiplier we use for Boolean?*/
						multiplier=1;
						success=1;
					}
					else
					{
						success=0;
					}
				}break;
				default:
				{				
					success=0;				
				}break;
			}
			if(!success)
			{
				display_message(WARNING_MESSAGE,
						"get_multiplier_from_physical_dim_string.  unknown physical dimension %s",
						physical_dim_string);
			}
		}/*if(string)*/
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"get_multiplier_from_physical_dim_string. invalid argument");
		multiplier=1.0;
	}
	LEAVE;
	return(multiplier);
}/* get_multiplier_from_physical_dim_string*/

static int make_default_edf_rig(struct EDF_header_record *edf_header_record,
	struct Rig **rig_pointer
#if defined (UNEMAP_USE_3D)
	,struct Unemap_package *unemap_package
#endif
																)
/*******************************************************************************
LAST MODIFIED : 12 October 2001

DESCRIPTION :
Make a default rig in <rig_pointer> based on the information in 
<edf_header_record>. Have no electrode position information, so it's 
invented via create_standard_Rig.
This function is used when reading in an EDF file, but don't have a suitable 
CNFG file.
==============================================================================*/
{
	char *name;
	int *electrodes_in_row,i,index,number_of_edf_signals,number_of_rows,
		return_code;
	struct Device **device;
	struct Rig *rig;

	ENTER(make_default_edf_rig);
	name=(char *)NULL;
	electrodes_in_row=(int *)NULL;
	rig=(struct Rig *)NULL;
	device=(struct Device **)NULL;	
	if(edf_header_record&&rig_pointer
#if defined (UNEMAP_USE_3D)
		&&unemap_package
#endif
		 )
	{	
		return_code=0;		
		number_of_edf_signals=edf_header_record->number_of_signals;			
		display_message(INFORMATION_MESSAGE,"Creating default rig");
		number_of_rows=((int)(number_of_edf_signals)-1)/8+1;
		if (ALLOCATE(electrodes_in_row,int,number_of_rows))
		{
			index=number_of_rows-1;
			electrodes_in_row[index]=
				(int)(number_of_edf_signals)-8*index;
			while (index>0)
			{
				index--;
				electrodes_in_row[index]=8;
			}
			if (*rig_pointer=create_standard_Rig("default",
				TORSO,MONITORING_OFF,EXPERIMENT_OFF,number_of_rows,
				electrodes_in_row,1,0,(float)1
#if defined (UNEMAP_USE_3D)
				,unemap_package
#endif /* defined (UNEMAP_USE_3D) */
																					 ))
			{				
				return_code=1;				
				rig=*rig_pointer;
				/*rename rig_devices from edf */
				device=rig->devices;
				i=0;
				while((i<rig->number_of_devices)&&(return_code))
				{
					/* free up the default name */
					DEALLOCATE((*device)->description->name);
					/*set the new rig device name from the edf device name*/
					name=trim_string(edf_header_record->edf_device[i].label);								
					if (name)
					{
						(*device)->description->name=name;
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"make_default_edf_rig. Could not allocate memory for name");
					}															
					device++;
					i++;
				}
			}
			else
			{
				return_code=0;
				display_message(ERROR_MESSAGE,
					"make_default_edf_rig.  Error creating default rig");
			}
			DEALLOCATE(electrodes_in_row);
		}
		else
		{
			return_code=0;
			display_message(ERROR_MESSAGE,
				"make_default_edf_rig.  Could not allocate electrodes_in_row");
		}	
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"make_default_edf_rig. invalid argument");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* make_default_edf_rig */

static struct EDF_header_record *read_bdf_or_edf_header_record(
	FILE *bdf_or_edf_file,int bdf)
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
creates an EDF_header_record and reads in the bdf or edf header record in 
<bdf_or_edf_file> into it.
When we return (if successful!) we'll be positioned at the beginning of the 
data record in <bdf_or_edf_file>.
<bdf> is a flag, 0=edf, 1=bdf.
DOESN'T read in any of the data record!
==============================================================================*/
{
	
	char *biostr,string[9],string2[81],*string_ptr,version[9],local_patient_id[81],
		local_recording_id[81],recording_start_time[9],recording_start_date[9];
	int i,number_of_signals,success,bytes_in_header,number_of_records;
	float record_duration;
	struct EDF_header_record *edf_header_record;

	ENTER(read_edf_header_record);
	string_ptr=(char *)NULL;
	biostr=(char *)NULL;
	edf_header_record=(struct EDF_header_record *)NULL;
	if(bdf_or_edf_file)
	{
		success=1;		
		/*Read in the header record*/
		/*version*/
		if(8==fread(version,sizeof(char),8,bdf_or_edf_file))
		{			
			version[8]='\0';
			if(bdf)
			{
				/*must have 255 then "BIOSEMI"*/
				if((unsigned char)(version[0])==255)
				{					
					biostr=&(version[1]);
					if(!strcmp(biostr,"BIOSEMI"))
					{
						success=1;
					}
					else
					{
						success=0;
					}				
				}
				else
				{
					success=0;
				}
			}
			else
			{
				/*edf, contents of version[] unimportant*/
				success=1;
			}
		}
		else
		{
			success=0;
		}
		/*local patient id */
		if(success)
		{
			if(80==fread(local_patient_id,sizeof(char),80,bdf_or_edf_file))
			{
				local_patient_id[80]='\0';			
			}
			else
			{
				success=0;
			}

		}
		/*local recording id */
		if(success)
		{
			if(80==fread(local_recording_id,sizeof(char),80,bdf_or_edf_file))
			{
				local_recording_id[80]='\0';					
			}
			else
			{
				success=0;
			}
		}
		/*start date of recording*/
		if(success)
		{
			if(8==fread(recording_start_date,sizeof(char),8,bdf_or_edf_file))
			{
				recording_start_date[8]='\0';						
			}
			else
			{
				success=0;
			}
		}
		/*start time of recording*/
		if(success)
		{
			if(8==fread(recording_start_time,sizeof(char),8,bdf_or_edf_file))
			{
				recording_start_time[8]='\0';					
			}
			else
			{
				success=0;
			}
		}
		/*num bytes in header record */
		if(success)
		{
			if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
			{
				string[8]='\0';
				sscanf(string,"%d",&bytes_in_header);						
			}
			else
			{
				success=0;
			}
		}
		/*44 bytes reserved */
		if(success)
		{
			if(44==fread(string2,sizeof(char),44,bdf_or_edf_file))
			{
				;
			}
			else
			{
				success=0;
			}	
		}		
		/*num data records */
		if(success)
		{
			if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
			{
				string[8]='\0';
				sscanf(string,"%d",&number_of_records);			
			}
			else
			{
				success=0;
			}			
		}
		/*duration of data record, in seconds */	
		if(success)
		{
			if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
			{
				string[8]='\0';
				sscanf(string,"%f",&record_duration); 				
			}
			else
			{
				success=0;
			}
		}
		/*number of signals */
		if(success)
		{
			if(4==fread(string,sizeof(char),4,bdf_or_edf_file))
			{
				string[4]='\0';			
				sscanf(string,"%d",&number_of_signals);								
			}
			else
			{
				success=0;
			}
		}
		if(success)
		{
			/* an edf signal structure for each signal*/
			if(edf_header_record=create_EDF_header_record(number_of_signals))
			{	
				edf_header_record->number_of_signals=number_of_signals;	
				edf_header_record->bytes_in_header=bytes_in_header;
				edf_header_record->number_of_records=number_of_records;
				edf_header_record->record_duration=record_duration;
				strcpy(edf_header_record->version,version);
				strcpy(edf_header_record->local_patient_id,local_patient_id);
				strcpy(edf_header_record->local_recording_id,local_recording_id);
				strcpy(edf_header_record->recording_start_time,recording_start_time);
				strcpy(edf_header_record->recording_start_date,recording_start_date);
				/*labels*/	
				i=0;
				while((i<number_of_signals)&&(success))
				{									
					string_ptr=edf_header_record->edf_device[i].label;				
					if(16==fread(string_ptr,sizeof(char),16,bdf_or_edf_file))
					{
						string_ptr[16]='\0';					
					}
					else
					{
						success=0;
					}
					i++;								
				}
				/*transducer type*/
				i=0;
				while((i<number_of_signals)&&(success))
				{					
					string_ptr=edf_header_record->edf_device[i].transducer_type;
					if(80==fread(string_ptr,sizeof(char),80,bdf_or_edf_file))
					{
						string_ptr[80]='\0';						
					}
					else
					{
						success=0;
					}
					i++;
				}
				/*physical dimension*/
				i=0;			
				while((i<number_of_signals)&&(success))
				{
					string_ptr=edf_header_record->edf_device[i].physical_dim;
					if(8==fread(string_ptr,sizeof(char),8,bdf_or_edf_file))
					{
						string_ptr[9]='\0';					
					}
					else
					{
						success=0;
					}
					i++;			
				}
				/*physical minimum*/
				i=0;			
				while((i<number_of_signals)&&(success))
				{
					if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
					{
						string[9]='\0';
						sscanf(string,"%f",&edf_header_record->edf_device[i].physical_min);
					}
					else
					{
						success=0;
					}
					i++;			
				}
				/*physical maximum*/		
				i=0;
				while((i<number_of_signals)&&(success))
				{
					if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
					{
						string[9]='\0';
						sscanf(string,"%f",&edf_header_record->edf_device[i].physical_max);	
					}
					else
					{
						success=0;
					}
					i++;		
				}
				/*digital minimum*/	
				i=0;	
				while((i<number_of_signals)&&(success))
				{
					if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
					{
						string[9]='\0';
						sscanf(string,"%d",&edf_header_record->edf_device[i].digital_min);
					}
					else
					{
						success=0;
					}
					i++;			
				}
				/*digital maximum*/			
				i=0;
				while((i<number_of_signals)&&(success))
				{
					if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
					{
						string[9]='\0';
						sscanf(string,"%d",&edf_header_record->edf_device[i].digital_max);
					}
					else
					{
						success=0;
					}
					i++;			
				}
				/*prefiltering */	
				i=0;	
				while((i<number_of_signals)&&(success))
				{
					string_ptr=edf_header_record->edf_device[i].prefiltering;
					if(80==fread(string_ptr,sizeof(char),80,bdf_or_edf_file))
					{
						string_ptr[80]='\0';
					}
					else
					{
						success=0;
					}
					i++;			
				}	
				/* number  of samples in data record*/		
				i=0;
				while((i<number_of_signals)&&(success))
				{				
					if(8==fread(string,sizeof(char),8,bdf_or_edf_file))
					{
						string[9]='\0';
						sscanf(string,"%d",&edf_header_record->edf_device[i].number_of_samples);
					}
					else
					{
						success=0;
					}
					i++;			
				}		
				/* reserved*/
				i=0;
				while((i<number_of_signals)&&(success))
				{
					if(32==fread(string2,sizeof(char),32,bdf_or_edf_file))
					{
						;
					}
					else
					{
						success=0;
					}
					i++;
				}
				if(!success)
				{
					display_message(ERROR_MESSAGE,
						"read_edf_header_record. error reading signal info "); 	
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"read_edf_header_record. failed to create edf_header_record"); 
				success=0;
			}
		}	
		else
		{
			display_message(ERROR_MESSAGE,
				"read_edf_header_record. error reading info "); 		
		}
	}
	else
	{	
		display_message(ERROR_MESSAGE,
			"read_edf_header_record. invalid argument");
		success=0;
	}
	LEAVE;
	return(edf_header_record);
}/* read_edf_header_record*/

/*
Global functions
----------------
*/
int read_bdf_or_edf_file(char *file_name,struct Rig **rig_pointer,
	struct User_interface *user_interface
#if defined (UNEMAP_USE_3D)
			,struct Unemap_package *unemap_package
#endif /* defined (UNEMAP_USE_NODES)*/
				,int bdf)
/*******************************************************************************
LAST MODIFIED : 11 December 2001

DESCRIPTION :
Reads the signal data from a BDF or EDF file specified by <file_name> and 
creates a rig with it in <rig_pointer>.
Optionally reads in a config file, or create a defalut one.
<bdf> is a flag, 0=edf, 1=bdf.
==============================================================================*/
{
	char *config_file_name,*bdf_or_edf_data,*source_base,*source;
	enum Signal_value_type signal_value_type;
	FILE *bdf_or_edf_file;
	float *float_dest,frequency,sampling_duration,prefix_multiplier,phys_min,phys_max,
		dig_min,dig_max,gain,offset;
	int count,data_32bit,bdf_or_edf_data_size,failed,i,index,j,k,
		number_of_devices,number_of_samples,number_of_edf_signals,number_of_records,
		return_code,word_size;
	short *dest;

	struct Device **device;
	struct EDF_to_rig_map *edf_to_rig_map;
	struct EDF_header_record *edf_header_record;
	struct Rig *rig;
	struct Region *current_region;
	struct Region_list_item *region_item;
	struct Signal_buffer *buffer;
	unsigned char *data_24bit,*twos_comp;
	ENTER(read_bdf_or_edf_file);
	float_dest=(float *)NULL;
	return_code=0;
	region_item=(struct Region_list_item *)NULL;
	current_region=(struct Region *)NULL;
	edf_header_record=(struct EDF_header_record *)NULL;
	config_file_name=(char *)NULL;
	bdf_or_edf_data=(char *)NULL;
	source_base=(char *)NULL;
	source=(char *)NULL;
	bdf_or_edf_file=(FILE *)NULL;
	dest=(short *)NULL;
	device=(struct Device **)NULL;
	rig=(struct Rig *)NULL;
	buffer=(struct Signal_buffer *)NULL;
	twos_comp=(unsigned char *)NULL;
	edf_to_rig_map=(struct EDF_to_rig_map *)NULL;
	if (file_name&&rig_pointer&&user_interface
#if defined (UNEMAP_USE_3D)
			&&unemap_package			
#endif
			)
	{ 	
		if (bdf_or_edf_file=fopen(file_name,"ra"))
		{		
			/* read the header information */
			if(edf_header_record=read_bdf_or_edf_header_record(bdf_or_edf_file,bdf))
			{
				return_code=1;
				number_of_edf_signals=edf_header_record->number_of_signals;	
#if defined (DEBUG)
				print_edf_header_record(edf_header_record);
#endif /*defined (DEBUG) */
				/* read the rig configuration file name */
				if (config_file_name=confirmation_get_read_filename(".cnfg",user_interface))
				{
					return_code=read_configuration_file(config_file_name,rig_pointer
#if defined (UNEMAP_USE_3D)	
						,unemap_package
#endif /* defined (UNEMAP_USE_3D) */
							);								
				}
				else
				{
					/*pressed cancel on confirmation_get_read_file name dialogue*/ 
					/* make up a default rig*/
					return_code=make_default_edf_rig(edf_header_record,rig_pointer
#if defined (UNEMAP_USE_3D)
						,unemap_package
#endif
						 );
				}
				if(return_code)
				{
					rig=*rig_pointer;
					/*if no current region, make the first region the current region*/
					current_region=get_Rig_current_region(rig);
					if (!current_region)
					{
						region_item=get_Rig_region_list(rig);
						current_region=get_Region_list_item_region(region_item);
						set_Rig_current_region(rig,current_region);
					}
					number_of_edf_signals=edf_header_record->number_of_signals;
					number_of_records=edf_header_record->number_of_records;
					sampling_duration=edf_header_record->record_duration;									
					/* for unemap, all signals should have the same number of samples */				
					number_of_samples=edf_header_record->edf_device[0].number_of_samples;
					i=0;
					failed=0;
					while((i<number_of_edf_signals)&&(!failed))
					{
						if(number_of_samples!=
							edf_header_record->edf_device[i].number_of_samples)
						{
							failed=1;	
							return_code=0;
							display_message(ERROR_MESSAGE,
								"read_bdf_or_edf_file. EDF signals have different number of samples. Unemaps requires them to be the same");
						}
						i++;
					}						
					if(return_code)
					{
						/* match up the devices in rig from the config file to the edf file  */
						if(edf_to_rig_map=match_edf_and_rig_devices(rig,edf_header_record))
						{
							/*convert the rig to use only the matched devices */
							return_code=destroy_unmatched_rig_devices(rig,edf_to_rig_map);
						}	
						/*read in all the data records*/

						if(bdf)
						{							
							/*bdf 24 bit = 3 bytes*/
							word_size=3;
						}
						else
						{
							/* each item of edf data is 2 bytes*/
							word_size=2;
						}
						bdf_or_edf_data_size=word_size*number_of_records*number_of_edf_signals*number_of_samples;
					}									
					if(return_code&&bdf_or_edf_data_size&&ALLOCATE(bdf_or_edf_data,char,bdf_or_edf_data_size))
					{
						if(bdf_or_edf_data_size=fread(bdf_or_edf_data,sizeof(char),bdf_or_edf_data_size,bdf_or_edf_file))
						{	
							if(bdf)
							{	
								/*store bdf in floats*/
								signal_value_type=FLOAT_VALUE;
							}
							else
							{
								/*edf files are always short ints*/
								signal_value_type=SHORT_INT_VALUE;
							}
							frequency=((float)number_of_samples)/sampling_duration;
							/* create the signal buffer */
							if (buffer=create_Signal_buffer(signal_value_type,rig->number_of_devices,
								number_of_samples*number_of_records,frequency))
							{				
								/*set buffer->times*/
								for(i=0;i<number_of_samples*number_of_records;i++)
								{
									buffer->times[i]=i;
								}
								/*set signal values from the edf data*/ 
								count=0;
								if(bdf)
								{
									float_dest=buffer->signals.float_values;
								}
								else
								{
									/*edf*/
									dest=buffer->signals.short_int_values;	
								}
								for(i=0;i<number_of_records;i++)
								{	
									source_base=bdf_or_edf_data+i*number_of_edf_signals*number_of_samples*word_size;				
									for(j=0;j<number_of_samples;j++)
									{					
										for(k=0;k<rig->number_of_devices;k++)
										{
											/*get index of current edf signal*/
											index=edf_to_rig_map->edf_to_rig_device_map[k].edf_device_index;
											source=source_base+(index*number_of_samples+j)*word_size;									
											if(bdf)
											{
												data_24bit=(unsigned char *)source;											
												/*convert 24 bit to 32 bit*/
												data_32bit=0;
												data_32bit= 
													((unsigned int)(*(data_24bit+2))<<24)+
													((unsigned int)(*(data_24bit+1))<<16)+
													((unsigned int)(*(data_24bit))<<8);
												data_32bit/=256;
												/*convert in to float*/
												*float_dest=(float)(data_32bit);												
												float_dest++; 
											}
											else
											{	
												/*edf*/
												twos_comp=(unsigned char *)source;
												/*copy the 2 bytes*/
												*dest= ((unsigned short)(*(twos_comp+1))<<8)+
													(unsigned short)(*twos_comp);						
												dest++; 
											}
											count++;
										}/* for(k=0;k<rig->number_of_devices;k++) */
									}/* for(j=0;j<number_of_samples;j++) */
								}/*	for(i=0;i<number_of_records;i++) */								
								DEALLOCATE(bdf_or_edf_data);	
								/* set other signal information from the edf data */
								number_of_devices=rig->number_of_devices;
								device=rig->devices;
								for(i=0;i<number_of_devices;i++)
								{						
									if ((*device)->channel)
									{
										/*determine the gain and offset*/
										prefix_multiplier=get_multiplier_from_physical_dim_string(
											edf_header_record->edf_device[i].physical_dim);					
										phys_min=edf_header_record->edf_device[i].physical_min; 
										phys_min*=prefix_multiplier;/* conv to mV */
										phys_max=edf_header_record->edf_device[i].physical_max;
										phys_max*=prefix_multiplier; /* conv to mV */
										dig_min=edf_header_record->edf_device[i].digital_min;
										dig_max=edf_header_record->edf_device[i].digital_max;
										gain=(phys_max-phys_min)/(dig_max-dig_min);
										offset=dig_min-(phys_min/gain);
										(*device)->channel->gain_correction=(float)1;
										(*device)->channel->gain=gain;
										(*device)->channel->offset=offset;
#if defined (NEW_CODE)
										/*??JW*/
										/* this is correct,setting the display min and max to the */
										/* resolution limits, however, many signals are much smaller than  */
										/* this (at least until we get active electrodes) so we'll */
										/* make it find it's own, by setting minimum=1,maximum=0 */
										(*device)->signal_display_minimum=gain*(dig_min-offset);
										(*device)->signal_display_maximum=gain*(dig_max-offset);
#else
										(*device)->signal_display_minimum=1;
										(*device)->signal_display_maximum=0;
#endif /* defined (NEW_CODE) */
										if (ELECTRODE==(*device)->description->type)
										{
											if (!((*device)->signal=
												create_Signal(i,buffer,UNDECIDED,0)))
											{
												return_code=0;
												destroy_Signal_buffer(&buffer);
												destroy_Rig(&rig);
												return_code=0;
											}
										}
										else
										{
											/* AUXILIARY */
											if (!((*device)->signal=
												create_Signal(i,buffer,REJECTED,0)))
											{
												return_code=0;
												destroy_Signal_buffer(&buffer);
												destroy_Rig(&rig);
												return_code=0;
											}
										} /* if (ELECTRODE==(*device)->description->type) */
										
									}/* if ((*device)->channel) */
									device++;
								}/* for(i=0;i<number_of_devices;i++) */
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_bdf_or_edf_file.  Could not create signal buffer");
								destroy_Rig(&rig);
								return_code=0;
							}		
						}
						else
						{	
							display_message(ERROR_MESSAGE,
								"read_bdf_or_edf_file.  failed to read edf data");
							destroy_Rig(&rig);
							return_code=0;
							
						}
					}	
					else
					{
						display_message(ERROR_MESSAGE,
							"read_bdf_or_edf_file. failed to allocate bdf_or_edf_data"); 
						return_code=0;
					}
				}				
				else
				{
					display_message(ERROR_MESSAGE,
						"read_bdf_or_edf_file. read_configuration_file failed");	
					return_code=0;	
				}	
			}/*read_edf_header_file_record*/
			fclose(bdf_or_edf_file);				
			destroy_EDF_to_rig_map(&edf_to_rig_map);	
			destroy_EDF_header_record(&edf_header_record);			
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_bdf_or_edf_file.  Could not open file: %s",file_name);	
			return_code=0;		
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_bdf_or_edf_file.  Missing argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* read_bdf_or_edf_file */



