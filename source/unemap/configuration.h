/*******************************************************************************
FILE : configuration.h

LAST MODIFIED : 30 October 1992

DESCRIPTION :
Structures and function prototypes for creating and modifying rig
configurations.
==============================================================================*/
#include "rig.h"

struct Rig *create_standard_Rig(char *name,enum Rig_type rig_type,
	enum Monitoring_status monitoring,enum Experiment_status experiment,
	int number_of_rows,int number_of_columns,int number_of_regions,
	int number_of_auxiliary_inputs,float sock_focus);
/*******************************************************************************
LAST MODIFIED : 30 October 1992

DESCRIPTION :
This function is a specialized version of create_Rig (in rig.c).  It creates a
rig with <number_of_regions> regions with identical electrode layouts.  In each
region the electrodes are equally spaced in <number_of_rows> rows and
<number_of_columns> columns.  There are <number_of_auxiliary_inputs> auxiliary
inputs.
???Move to rig.c ?
==============================================================================*/
