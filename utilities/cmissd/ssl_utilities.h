#if defined (MEMORY_CHECKING)
#include "general/debug.h"
#else /* defined (MEMORY_CHECKING) */
#define ALLOCATE( result , type , number ) \
( result = ( type *) malloc( ( number ) * sizeof( type ) ) )

#define DEALLOCATE( ptr ) \
{ free((char *) ptr ); ( ptr )=NULL;}

#define ENTER( function_name )

#define LEAVE

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) realloc( (void *)( initial ) , \
	( number ) * sizeof( type ) ) )
#endif /* defined (MEMORY_CHECKING) */

#define FULL_NAMES
#include "general/object.h"
#include "user_interface/message.h"

struct Socket_buffer;

struct Socket_buffer *CREATE(Socket_buffer)(SSL *ssl);
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Creates a Socket_buffer object.
==============================================================================*/

int DESTROY(Socket_buffer)(struct Socket_buffer **socket_buffer_address);
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Destroys a Socket_buffer object
x==============================================================================*/

char *Socket_buffer_get_input(struct Socket_buffer *buffer, char terminator);
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
This function is a port of the similar function in general/socket.c
==============================================================================*/

int SSL_unix_socket_tunnel(SSL *ssl, int ssl_socket, int local_socket);
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Operates a tunnel which forwards anything from the plain socket to the 
SSL connection and vice versa.
==============================================================================*/

int terminal_to_SSL(SSL *ssl, int sock, int id);
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Operates a tunnel which forwards anything from the plain socket to the 
SSL connection and vice versa.
==============================================================================*/
