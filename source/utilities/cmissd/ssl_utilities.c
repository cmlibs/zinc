#include <openssl/ssl.h>

#include <ssl_utilities.h>
#include <fcntl.h>


#define BLOCKSIZE (1024)

struct Socket_buffer
{
	char *buffer;
	int buffer_size;
	int buffer_index;
	int buffer_acknowledge_index;

	SSL *ssl;
};

struct Socket_buffer *CREATE(Socket_buffer)(SSL *ssl)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Creates a Socket_buffer object.
==============================================================================*/
{
	struct Socket_buffer *socket_buffer;

	if (ALLOCATE(socket_buffer, struct Socket_buffer, 1))
	{
		socket_buffer->ssl = ssl;
		socket_buffer->buffer = (char *)NULL;
		socket_buffer->buffer_index = 0;
		socket_buffer->buffer_size = 0;
		socket_buffer->buffer_acknowledge_index = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Socket_buffer).  "
			"Unable to allocate Socket_buffer.");
		socket_buffer = (struct Socket_buffer *)NULL;
	}
}

int DESTROY(Socket_buffer)(struct Socket_buffer **socket_buffer_address)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Destroys a Socket_buffer object
x==============================================================================*/
{
	int return_code;
	struct Socket_buffer *socket_buffer;

	if (socket_buffer_address && 
		(socket_buffer = (struct Socket_buffer *)*socket_buffer_address))
	{
		if (socket_buffer->buffer)
		{
			DEALLOCATE(socket_buffer->buffer);
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

char *Socket_buffer_get_input(struct Socket_buffer *buffer, char terminator)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
This function is a port of the similar function in general/socket.c
==============================================================================*/
{
	char *new_string, *return_string, *terminator_index;
	fd_set readfds;
	int finish, return_length, select_code, total_read;
	ssize_t number_read;
	struct timeval timeout_struct;
 
	ENTER(Socket_get_line_from_stdout);
	if (buffer)
	{
		return_string = (char *)NULL;
		total_read = 0;
		finish = 0;
		if (buffer->buffer_index + BLOCKSIZE >= buffer->buffer_size)
		{
			if (REALLOCATE(new_string, buffer->buffer, char,
				buffer->buffer_size + 2 * BLOCKSIZE))
			{
				buffer->buffer = new_string;
				buffer->buffer_size += 2 * BLOCKSIZE;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Socket_get_line."
					"  Unable to reallocate string");
				finish = 1;
			}
		}
		if (!finish)
		{
			number_read = SSL_read(buffer->ssl,
				(void *)(buffer->buffer + buffer->buffer_index), BLOCKSIZE);
			if (number_read < 1)
			{
				/* Socket has reached EOF or had an error and so must close */
				finish = 1;
			}
			else
			{
				buffer->buffer_index += number_read;
				total_read += number_read;
			}
		}

		/* End the buffer string but don't increment the buffer index
			so the next data overwrites the NULL character. */
		if (buffer->buffer)
		{
			buffer->buffer[buffer->buffer_index] = 0;
		}

		if (buffer->ssl && (total_read > 0))
		{
			char acknowledge[100];
			int acknowledge_number;

			/* Acknowledge any new commmands that have been completed
				even if they aren't being returned yet */
			while (terminator_index = strchr(buffer->buffer + buffer->buffer_acknowledge_index, 
				terminator))
			{
				acknowledge_number = terminator_index - buffer->buffer 
					- buffer->buffer_acknowledge_index;
				sprintf(acknowledge, "RECEIVED %d bytes\n", acknowledge_number);
				SSL_write(buffer->ssl, acknowledge, strlen(acknowledge));
				buffer->buffer_acknowledge_index = terminator_index - buffer->buffer + 1;
			}
		}

		/* If there is a terminated string ready in the buffer return it
			and remove those characters from the buffer */
		if (terminator_index = strchr(buffer->buffer, terminator))
		{
			return_length = terminator_index - buffer->buffer;
			if (ALLOCATE(return_string, char, return_length + 1))
			{
				memcpy(return_string, buffer->buffer, return_length);
				return_string[return_length]  = 0;

				memmove(buffer->buffer, terminator_index + 1, strlen(terminator_index));
				buffer->buffer_index -= return_length + 1;
				buffer->buffer_acknowledge_index -= return_length + 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Socket_get_line."
					"  Unable to allocate string");
				return_string = (char *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Socket_get_lines."
			"  Missing socket");
		return_string = (char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Socket_get_line */


int SSL_unix_socket_tunnel(SSL *ssl, int ssl_socket, int local_socket)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Operates a tunnel which forwards anything from the plain socket to the 
SSL connection and vice versa.
==============================================================================*/
{
	char *new_string;
	int done, len, return_code, select_code;
	int waiting_to_write_local, write_ssl_buffer_size, write_ssl_buffer_index;
	int waiting_to_write_ssl, write_local_buffer_size, write_local_buffer_index;
	char *write_local_buffer, *write_ssl_buffer;
	fd_set readfds, writefds;

	done = 0;
	if (ALLOCATE(write_local_buffer, char, 2 * BLOCKSIZE)
		&& ALLOCATE(write_ssl_buffer, char, 2 * BLOCKSIZE))
	{
		waiting_to_write_local = 0;
		waiting_to_write_ssl = 0;
		write_local_buffer_size = 2 * BLOCKSIZE;
		write_ssl_buffer_size = 2 * BLOCKSIZE;
		write_local_buffer_index = 0;
		write_ssl_buffer_index = 0;
		while (!done)
		{
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			if (waiting_to_write_ssl)
			{
				FD_SET(ssl_socket, &writefds);
			}
			FD_SET(ssl_socket, &readfds);
			if (waiting_to_write_local)
			{
				FD_SET(local_socket, &writefds);
			}
			FD_SET(local_socket, &readfds);
			/* Block until there is something to do */
			if (0 < (select_code = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL)))
			{
				/* Send things in the buffers if we can */
				if (FD_ISSET(local_socket, &writefds))
				{
					len = write(local_socket, write_local_buffer, write_local_buffer_index);
					memmove(write_local_buffer, write_local_buffer + len,
						write_local_buffer_index - len);
					write_local_buffer_index -= len;
					if (0 == write_local_buffer_index)
					{
						waiting_to_write_local = 0;
					}
				}
				/* Send things in the buffers if we can */
				if (FD_ISSET(ssl_socket, &writefds))
				{
					len = SSL_write(ssl, write_ssl_buffer, write_ssl_buffer_index);
					switch(SSL_get_error(ssl, len))
					{
						case SSL_ERROR_NONE:
						{
							memmove(write_ssl_buffer, write_ssl_buffer + len,
								write_ssl_buffer_index - len);
							write_ssl_buffer_index -= len;
							if (0 == write_ssl_buffer_index)
							{
								waiting_to_write_ssl = 0;
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "SSL write error");
							return_code = 0;
							done = 1;
						} break;
					}
				}
				/* Read anything new */
				if (FD_ISSET(ssl_socket, &readfds))
				{
					if (write_local_buffer_index + BLOCKSIZE >= write_local_buffer_size)
					{
						if (REALLOCATE(new_string, write_local_buffer, char,
								 write_local_buffer_size + 2 * BLOCKSIZE))
						{
							write_local_buffer = new_string;
							write_local_buffer_size += 2 * BLOCKSIZE;
						}
						else
						{
							display_message(ERROR_MESSAGE,"cmissd.  "
								"Unable to enlarge buffer.");
							done = 1;
							return_code = 0;
						}
					}
					len = SSL_read(ssl, write_local_buffer + write_local_buffer_index,
						BLOCKSIZE);
					switch(SSL_get_error(ssl, len))
					{
						case SSL_ERROR_NONE:
						{
							write_local_buffer_index += len;
							waiting_to_write_local = 1;
						} break;
						case SSL_ERROR_ZERO_RETURN:
						{
							/* This is a clean shutdown */
							done = 1;
						} break;
						default:
						{
							display_message(ERROR_MESSAGE, "SSL read error");
							return_code = 0;
							done = 1;
						} break;
					}
				}
				if (FD_ISSET(local_socket, &readfds))
				{
					if (write_ssl_buffer_index + BLOCKSIZE >= write_ssl_buffer_size)
					{
						if (REALLOCATE(new_string, write_ssl_buffer, char,
								 write_ssl_buffer_size + 2 * BLOCKSIZE))
						{
							write_ssl_buffer = new_string;
							write_ssl_buffer_size += 2 * BLOCKSIZE;
						}
						else
						{
							display_message(ERROR_MESSAGE,"cmissd.  "
								"Unable to enlarge buffer.");
							done = 1;
							return_code = 0;
						}
					}
					len = read(local_socket, write_ssl_buffer + write_ssl_buffer_index,
						BLOCKSIZE);
					if (len > 0)
					{
						write_ssl_buffer_index += len;
						waiting_to_write_ssl = 1;
					}
					else
					{
						/* The select indicated ready but there was no data */
						done = 1;
					}
				}
			}
		}
	}
	return (return_code);
}

int terminal_to_SSL(SSL *ssl, int sock, int id)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
Operates a tunnel which forwards anything from the plain socket to the 
SSL connection and vice versa.
==============================================================================*/
{
#define BUFFSIZE (1024)

	int done, width;
	int r,c2sl=0,c2s_offset=0;
	fd_set readfds,writefds;
	int shutdown_wait=0;
	char c2s[BUFFSIZE],s2c[BUFFSIZE];
	int ofcmode;
    
	/*First we make the socket nonblocking*/
	ofcmode=fcntl(sock,F_GETFL,0);
	ofcmode|=O_NDELAY;
	if(fcntl(sock,F_SETFL,ofcmode))
      err_exit("Couldn't make socket nonblocking");
    

	width=sock+1;
    
	done = 0;
	while(!done)
	{
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);

      FD_SET(sock,&readfds);

      /*If we've still got data to write then don't try to read*/
      if(c2sl)
			FD_SET(sock,&writefds);
      else
			FD_SET(fileno(stdin),&readfds);

      r=select(width,&readfds,&writefds,0,0);
      if(r==0)
			continue;

      /* Now check if there's data to read */
      if(FD_ISSET(sock,&readfds))
		{
			do {
				r=SSL_read(ssl,s2c,BUFFSIZE);
          
				switch(SSL_get_error(ssl,r)){
					case SSL_ERROR_NONE:
						fwrite(s2c,1,r,stdout);
						break;
					case SSL_ERROR_ZERO_RETURN:
						/* End of data */
						if(!shutdown_wait)
							SSL_shutdown(ssl);
						done = 1;
						break;
					case SSL_ERROR_WANT_READ:
						break;
					default:
						berr_exit("SSL read problem");
				}
			} while (SSL_pending(ssl));
      }
      
		if (!done)
		{
			/* Check for input on the console*/
			if(FD_ISSET(fileno(stdin),&readfds))
			{
				c2sl=read(fileno(stdin),c2s,BUFFSIZE);
				if(c2sl==0){
					shutdown_wait=1;
					if(SSL_shutdown(ssl))
						done = 1;
				}
				c2s_offset=0;
			}

			/* If we've got data to write then try to write it*/
			if (!done)
			{
				if(c2sl && FD_ISSET(sock,&writefds))
				{
					r=SSL_write(ssl,c2s+c2s_offset,c2sl);

					switch(SSL_get_error(ssl,r)){
						/* We wrote something*/
						case SSL_ERROR_NONE:
							c2sl-=r;
							c2s_offset+=r;
							break;
            
							/* We would have blocked */
						case SSL_ERROR_WANT_WRITE:
							break;

							/* Some other error */
						default:	      
							berr_exit("SSL write problem");
					}
				}
			}
		}
      
	}

	return;
}
