/*******************************************************************************
FILE : computed_field_dijkstra_path.h

LAST MODIFIED : 21 June 2005

DESCRIPTION :
Implements image processing operations on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_DIJKSTRA_PATH_H)
#define COMPUTED_FIELD_DIJKSTRA_PATH_H

typedef struct _ELIST {
     int      row, col;
     struct _ELIST * next;
} ELIST;

typedef struct _PLIST {
     int     wt;
     ELIST * elements;
     struct _PLIST * next;
} PLIST;

typedef struct {
     int    count;
     PLIST * priorities;
} PQUEUE;

PQUEUE * pqueue_new(void);
int pqueue_insert(PQUEUE * pq, int row, int col, int wt);
int pqueue_peekmin(PQUEUE * pq, int * row, int * col);
int pqueue_popmin(PQUEUE * pq, int * row, int * col);
FE_value h_2d(int r1, int c1, int r2, int c2);

int Computed_field_register_types_dijkstra_path(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 21 June 2005

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_DIJKSTRA_PATH_H) */
