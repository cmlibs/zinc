/*******************************************************************************
FILE : cell_component.h

LAST MODIFIED : 09 September 1999

DESCRIPTION :
Functions and structures for using the Cell_component structure.
==============================================================================*/
#if !defined (CELL_COMPONENT_H)
#define CELL_COMPONENT_H

#if defined (MOTIF)
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "cell/cell_window.h"
#include "cell/cell_3d.h"

/*
Global types
============
*/
enum Cell_components
/*******************************************************************************
LAST MODIFIED : 26 February 1999

DESCRIPTION :
The different components allowed.
==============================================================================*/
{
  INA,
  ICAL,
  IPCA,
  PASSIVE_ELEMENTS,
  ICAB,
  ICAT,
  EXTRACELLULAR_CA,
  INACA,
  INSCA,
  MEMBRANE,
  JSRCA,
  IREL,
  ITR,
  IUP,
  NSRCA,
  ILEAK,
  MITOCHONDRIA,
  CALMODULIN,
  INTRACELLULAR_CA,
  TROPONIN,
  TROPOMYOSIN,
  STEADY_STATE_MECHANICS,
  CROSSBRIDGES,
  ITO,
  IKP,
  IK1,
  IKS,
  IKR,
  INAK,
  INTRACELLULAR_MG,
  INTRACELLULAR_NA,
  INTRACELLULAR_K,
  EXTRACELLULAR_NA,
  EXTRACELLULAR_K,
  INAB,
  IF,
  PARAMETERS,
  UNKNOWN_COMPONENT
}; /* enum Cell_components */

struct Cell_component
/*******************************************************************************
LAST MODIFIED : 08 Septemebr 1999

DESCRIPTION :
Stores information for a cell component (i.e. channel, pump, etc...)
==============================================================================*/
{
  char *name;
  struct Cell_window *cell;
  Region region;
  /*enum Cell_components component;*/
  char *component;
  int number_of_parameters;
  struct Cell_parameter **parameters;
  struct Parameter_dialog *dialog;
  struct Cell_graphic *graphic;
  FE_value axis1[3],axis2[3],axis3[3],point[3];
  struct Cell_component *next;
}; /* struct Cell_component */

/*
Global functions
================
*/
enum Cell_components get_cell_component_from_string(char *component_string);
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Returns the Cell_components corresponding to the <component>.
==============================================================================*/
void destroy_cell_component_list(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Deallocates the memory associated with the component list.
==============================================================================*/
int create_cell_component_list(struct Cell_window *cell);
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Creates the cell component list from the defined parameters.
==============================================================================*/

#endif /* !defined (CELL_PARAMETER_H) */
