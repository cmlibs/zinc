#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include <openssl/ssl.h>

#include <ssl_utilities.h>

#define FILENAME_BUFFERSIZE (PATH_MAX) /* From limits.h */
#define BUFFSIZE (1024)
#define FILENAME_SIZE (1024)

static int s_server_session_id_context = 1;

BIO *bio_err=0;
static char *pass;
static int password_cb(char *buf,int num,int rwflag,void *userdata);
static void sigpipe_handle(int x);

typedef int (Server_function)(SSL *ssl, int ssl_socket, char *user_shell);

enum Server_mode
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
Controls the operation of the server.
==============================================================================*/
{
	SERVER_ONCE_ON_STDIN_STDOUT,
	SERVER_SINGLETHREADED,
	SERVER_MULTITHREADED
};

/* A simple error and exit routine*/
int err_exit(char *string)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
Routine to check that the filename supplied is sufficiently "secure" that we
can trust it.
Currently I am not doing much at all, just verifying that the filename is real,
owned by root or the current user and not writable by the group or world.
==============================================================================*/
{
	fprintf(stderr,"%s\n",string);
	exit(0);
}

/* Print SSL errors and exit*/
int berr_exit(char *string)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
	BIO_printf(bio_err,"%s\n",string);
	ERR_print_errors(bio_err);
	exit(0);
}

static int tcp_unix_file_connect(char *filename)
 {
    struct hostent *hp;
    struct sockaddr_un addr;
    int sock;
    
    memset(&addr,0,sizeof(addr));
    strcpy(addr.sun_path, filename);
    addr.sun_family = AF_UNIX;

    if((sock=socket(PF_UNIX, SOCK_STREAM, 0))<0)
	 {
		 close (sock);
		 return 0;
	 }
    if(connect(sock,(struct sockaddr *)&addr,sizeof(addr))<0)
		 return 0;
    
    return sock;
  }

int echo(SSL *ssl, int ssl_socket, char *user_shell)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
An echo server that I used to test all the SSL stuff.
==============================================================================*/
{
	char buf[BUFFSIZE];
	int r,len,offset;
	int return_code, done;
   
	done = 0;
	return_code = 1;
	while(!done)
	{
		/* First read data */
		r=SSL_read(ssl,buf,BUFFSIZE);
		switch(SSL_get_error(ssl,r))
		{
			case SSL_ERROR_NONE:
			{
				len=r;
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

		if (!done)
		{
			/* Now keep writing until we've written everything*/
			offset=0;
			 
			while(len)
			{
				r=SSL_write(ssl,buf+offset,len);
				switch(SSL_get_error(ssl,r))
				{
					case SSL_ERROR_NONE:
					{
						len-=r;
						offset+=r;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE, "SSL write error");
						return_code = 0;
						done = 1;
					} break;
				}
			}
		}
	}
}      

int cmissd(SSL *ssl, int ssl_socket, char *user_shell)
/*******************************************************************************
LAST MODIFIED : 7 May 2002

DESCRIPTION :
The cmissd server either connects to an existing cmgui on a named unix socket
or starts one and connects to that.
==============================================================================*/
{
	char *argv[4];
	char *line;
	char *command, *new_command, *new_path, *path;
	char acknowledge[100], local_socket_name[100];
	int return_code, done, local_socket, select_code;
	struct Socket_buffer *buffer;
	int length, pid, tries_remaining;
	fd_set readfds;
      
	done = 0;
	return_code = 1;

	/* Use a buffered connection like this until we have a local connection */

	command = (char *)NULL;
	path = (char *)NULL;
	buffer = CREATE(Socket_buffer)(ssl);
	while(!done)
	{
		printf("In cmissd routine\n");
		FD_ZERO(&readfds);
		FD_SET(ssl_socket, &readfds);
		/* Block until there is something to read */
		if (0 < (select_code = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)))
		{
			if (line = Socket_buffer_get_input(buffer, '\n'))
			{
				display_message(INFORMATION_MESSAGE, "Received line'%s'\n", line);
				if (strstr(line, "START"))
				{
					length = strlen(line + 5) + 15;
					if (path)
					{
						length += strlen(path);
					}
					if (REALLOCATE(new_command, command, char, length))
					{
						command = new_command;
						if (path)
						{
							sprintf(command, "cd %s;exec %s", path, line + 5);
						}
						else
						{
							sprintf(command, "exec %s", line + 5);
						}
						display_message(INFORMATION_MESSAGE,
							"About to execute command: %s.\n", command);
						
						/* Execute an arbitrary command,
							we have authenticated and setuid to be a normal user. */
						if (!(pid = fork()))
						{
							/* Child comes here */
							argv[0] = user_shell;
							argv[1] = "-c";
							argv[2] = command;
							argv[3] = 0;
							execv(user_shell, argv);
							display_message(ERROR_MESSAGE,"cmissd.  "
								"Error in execve, child returned.");
							/* This is the child, we want it to die here with an error code */
							exit (127);
						}
						/* We now expect to connect to this process on /tmp/.cmiss/cmiss#####
							where #### is the pid */
						sprintf(local_socket_name, "/tmp/.cmiss%d", pid);
						tries_remaining = 100;
						while (tries_remaining && 
							!(local_socket = tcp_unix_file_connect(local_socket_name)))
						{
							usleep (50000);
							tries_remaining--;
						}
						done = 1;
						if (tries_remaining)
						{
							/* Then we assume we are successful */
							sprintf (acknowledge, "RESULT SUCCESS\n");
							SSL_write(ssl, acknowledge, strlen(acknowledge));
						}
						else
						{
							/* Then we assume we failed */
							sprintf (acknowledge, "RESULT FAILURE\n");
							SSL_write(ssl, acknowledge, strlen(acknowledge));
							display_message(ERROR_MESSAGE,"cmissd.  "
								"Unable to connect to local cmiss process after starting it.");
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"cmissd.  "
							"Unable to allocate memory for command string.");
					}
				}
				else if (strstr(line, "PATH"))
				{
					if (REALLOCATE(new_path, path, char, strlen(line + 4) + 1))
					{
						path = new_path;
						strcpy(path, line + 4);
					}
					else
					{
						display_message(ERROR_MESSAGE,"cmissd.  "
							"Unable to allocate memory for path string.");
					}
				}
				else if (strstr(line, "CONNECT"))
				{
				}
				else
				{
					display_message(ERROR_MESSAGE,"cmissd.  "
						"Unknown command line on ssl socket connection.");
					return_code = 0;
				}
			}
			else
			{
				/* Then we must be done and yet we have no connection so quit */
				display_message(ERROR_MESSAGE,"cmissd.  "
					"No connection started, exiting.");
				done = 1;
				return_code = 0;
			}
		}
		else
		{
			done = 1;
			if (select_code == -1)
			{
				display_message(ERROR_MESSAGE,"cmissd.  "
					"Error on socket");
				return_code = 0;
			}  
		}
	}
	DESTROY(Socket_buffer)(&buffer);
	if (path)
	{
		DEALLOCATE(path);
	}
	if (command)
	{
		DEALLOCATE(command);
	}

	if (return_code)
	{
		/* Now just act as a tunnel */
		if (ssl_socket && local_socket)
		{
			return_code = SSL_unix_socket_tunnel(ssl, ssl_socket, local_socket);
		}
	}
}      

int check_filename(char *filename, struct passwd *passwd)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
Routine to check that the filename supplied is sufficiently "secure" that we
can trust it.
Currently I am not doing much at all, just verifying that the filename is real,
owned by root or the current user and not writable by the group or world.
==============================================================================*/
{
	int return_code;
	char full_filename[FILENAME_BUFFERSIZE];
	struct stat stat_buf;

	if (realpath(filename, full_filename))
	{
      if (0 == stat(filename, &stat_buf))
		{
			if ((stat_buf.st_uid == 0 || stat_buf.st_uid == passwd->pw_uid) &&
            (stat_buf.st_mode & 022) == 0)
			{
				return_code = 1;
			}
			else
			{
				/* Bad modes */
				display_message(ERROR_MESSAGE,
					"Bad modes on file");
				return_code = 0;
			}
		}
		else
		{
			/* Could not stat */
			display_message(ERROR_MESSAGE,
				"Could not stat file");
			return_code = 0;
		}
	}
	else
	{
		/* Could not get realpath */
		display_message(ERROR_MESSAGE,
			"Could not get realpath on file %s, %s", filename, strerror(errno));
		return_code = 0;
	}

	return(return_code);
}
      
static void load_dh_params(SSL_CTX *ctx, char *file)
{
	DH *ret=0;
	BIO *bio;

	if ((bio=BIO_new_file(file,"r")) == NULL)
      berr_exit("Couldn't open DH file");

	ret=PEM_read_bio_DHparams(bio,NULL,NULL,NULL);
	BIO_free(bio);
	if(SSL_CTX_set_tmp_dh(ctx,ret)<0)
      berr_exit("Couldn't set DH parameters");
}

static int verify_callback(int preverify_ok, X509_STORE_CTX *x509_ctx)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
Callback called by SSL to ensure validity of a certificate.
==============================================================================*/
{
	int return_code;

	printf("In verify -> preverify %d\n", preverify_ok);
	return_code = preverify_ok;

	/* If the certificate is from the general path then we need to ensure
		that this user is allowed to login with the presented certificate */
	/* check_cert_chain(ssl, "Shane Blackett"); */
	
	return (return_code);
}

static int password_cb(char *buf,int num,int rwflag,void *userdata)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
	char *newline;
	struct termios tio; 
	struct termios saved_tio; 

	tcgetattr(fileno(stdin), &tio); 
	saved_tio = tio; 
	tio.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL); 
	tcsetattr(fileno(stdin), TCSANOW, &tio); 

	printf("Server key requires a password, enter it now.\n");

	fgets(buf, num, stdin); 
	/* Trim a newline */
	if (newline = strchr(buf, '\n'))
	{
		*newline = 0;
	}

	tcsetattr(fileno(stdin), TCSANOW, &saved_tio); 
	printf("Password read in.\n");

	return(strlen(buf));
}

static void sigpipe_handle(int x)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
}

static SSL_CTX *initialize_ctx(char *certificate, char *keyfile)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
    SSL_METHOD *meth;
    SSL_CTX *ctx;
    
    if(!bio_err)
	 {
      /* Global system initialization*/
      SSL_library_init();
      SSL_load_error_strings();
      
      /* An error write context */
      bio_err=BIO_new_fp(stderr,BIO_NOCLOSE);
    }

    /* Set up a SIGPIPE handler */
    signal(SIGPIPE,sigpipe_handle);
    
    /* Create our context*/
    meth=SSLv3_method();
    ctx=SSL_CTX_new(meth);

    /* Load the server identity certificate - this common name should
	    be the host name so that a client can validate the hostname */
    if(!(SSL_CTX_use_certificate_file(ctx,certificate,SSL_FILETYPE_PEM)))
      berr_exit("Couldn't read certificate file");

	 /* Load the corresponding private key */
    SSL_CTX_set_default_passwd_cb(ctx,password_cb);
    if(!(SSL_CTX_use_PrivateKey_file(ctx,keyfile,SSL_FILETYPE_PEM)))
      berr_exit("Couldn't read key file");

    /* Load randomness */
    if(!(RAND_load_file("random.pem",1024*1024)))
      berr_exit("Couldn't load randomness");
       
    return ctx;
}
      
void destroy_ctx(  SSL_CTX *ctx)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
	SSL_CTX_free(ctx);
}

/* Check that the common name matches the host name*/
void check_cert_chain(SSL *ssl, char *check_name)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
    X509 *peer;
    char peer_CN[256];
    
	 /* Check the authenticity of our clients certificate against
		 the locations we just allowed */
    if(SSL_get_verify_result(ssl)!=X509_V_OK)
      berr_exit("Certificate doesn't verify");

    /*Check the cert chain. The chain length
      is automatically checked by OpenSSL when we
      set the verify depth in the ctx */

    /*Check the common name*/
    if (peer=SSL_get_peer_certificate(ssl))
	 {
		 X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
			 NID_commonName, peer_CN, 256);

		 /* Check the client is who they say they are */
		 if(strcasecmp(peer_CN, check_name))
		 {
			 display_message(ERROR_MESSAGE,
				 "Certificate common name '%s' doesn't match required name '%s' for user",
				 peer_CN, check_name);
		 }
	 }
	 else
	 {
		 /* No client certificate supplied */
		 printf ("No client certificate supplied\n");
	 }
  }
         
int start_ssl_server(SSL_CTX *ctx, BIO *sbio, BIO *sbio2, 
	int ssl_socket, Server_function *server_function)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
	char *line_end, *user_authorised_certificates, *user_name;
	char user_certificate_suffix[] = ".cmissd/authorised_certificates";
	char *all_user_certificate_path = NULL;
	char read_buffer[BUFFSIZE];
	SSL *ssl;
	int index, r;
	struct passwd *passwd_entry;

	user_name = (char *)NULL;

	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

	ssl=SSL_new(ctx);
	SSL_set_bio(ssl,sbio,sbio);

	if((r=SSL_accept(ssl)<=0))
		berr_exit("SSL accept error");
		
	/* Use this connection to find out who the client wants to be */
	index = 0;
	*read_buffer = 0;
	while (!(line_end = strchr(read_buffer, '\n')))
	{
		r=SSL_read(ssl,read_buffer + index, BUFFSIZE - index);
		index += r;
		read_buffer[index] = 0;
	}
	*line_end = 0;
		
	printf ("Client wants to connect as %s\n", read_buffer);

	if (ALLOCATE(user_name, char, strlen(read_buffer)))
	{
		strcpy(user_name, read_buffer);
	}
	else
	{
		berr_exit("Unable to allocate string");
	}

	/* Shut this all down */
	/* The first shutdown announces that we want to close,
		then we issue the next one which blocks until the 
		connection is really closing. */
	if(SSL_shutdown(ssl)<0)
		berr_exit("Start SSL shutdown error");
		
	/* Finally close for real */
	if((r=SSL_shutdown(ssl)<=0))
		berr_exit("SSL shutdown error");

	SSL_free(ssl);

	/* Start a new connection that is authenticated for the
		client as well.  Valid certificates will be either ones
		local for this user only, in their 
		user_authorised_certificate file or ones in the common 
		all_user_certificate_path directory */

	/* This allows local certificate copies and one level of 
		certificate authority signing */
	SSL_CTX_set_verify_depth(ctx,1);

	/* Construct the users path */
	if (passwd_entry = getpwnam(user_name))
	{
		if (ALLOCATE(user_authorised_certificates, char, 
				 strlen(passwd_entry->pw_dir) + strlen(user_certificate_suffix) + 2))
		{
			sprintf(user_authorised_certificates, "%s/%s", passwd_entry->pw_dir,
				user_certificate_suffix);
		}
		/* Check the permissions on the file as we don't want other people
			to be able to mess with it */
		if (check_filename(user_authorised_certificates, passwd_entry))
		{
			if (!(SSL_CTX_load_verify_locations(ctx, user_authorised_certificates,
						all_user_certificate_path)))
			{
				berr_exit("Couldn't read CA list");
			}
				
			SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
				verify_callback);
				
			ssl=SSL_new(ctx);
			SSL_set_bio(ssl,sbio2,sbio2);
				
			if((r=SSL_accept(ssl)>0))
			{
				/* Change permanently to be the requested user, this is
					only reasonable in the multiuser or inetd versions as there
					is no way for this process to regain root now. */
				setuid(passwd_entry->pw_uid);
				/* Start in their home directory */
				chdir(passwd_entry->pw_dir);

				server_function(ssl, ssl_socket, passwd_entry->pw_shell);
					
				SSL_shutdown(ssl);
				SSL_free(ssl);
			}
			else
			{
				display_message(ERROR_MESSAGE, "SSL accept error");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"User certificate file %s does not exist or has insecure permissions",
				user_authorised_certificates);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Unknown user %s", user_name);
	}

	if (user_name)
	{
		DEALLOCATE(user_name);
	}
}

int ssl_server_listen(enum Server_mode server_mode, int port,
	char *public_certificate_filename, char *private_key_filename)
/*******************************************************************************
LAST MODIFIED : 6 May 2002

DESCRIPTION :
==============================================================================*/
{
	char *line_end, *user_authorised_certificates, *user_name;
	char user_certificate_suffix[] = ".cmissd/authorised_certificates";
	char *all_user_certificate_path = NULL;
	char read_buffer[BUFFSIZE];
	int sock,s;
	BIO *sbio, *sbio2;
	SSL_CTX *ctx;
    
	SSL *ssl;
	int index, r;
	struct passwd *passwd_entry;
	int done;
	int pid;
	struct sockaddr_in sin;
	int val=1;
	Server_function *server_function = cmissd;

	/* Build our SSL context*/
	ctx=initialize_ctx(public_certificate_filename, private_key_filename);
	load_dh_params(ctx, "dh1024.pem");
    
	SSL_CTX_set_session_id_context(ctx,(void*)&s_server_session_id_context,
      sizeof s_server_session_id_context);
    
	if (SERVER_ONCE_ON_STDIN_STDOUT == server_mode)
	{
		s = fileno(stdin);
		sbio=BIO_new_fd(s,BIO_NOCLOSE);
		sbio2=BIO_new_fd(s,BIO_NOCLOSE);
		start_ssl_server(ctx, sbio, sbio2, s, server_function);
	}
	else
	{
		if((sock=socket(AF_INET,SOCK_STREAM,0))<0)
			err_exit("Couldn't make socket");
    
		memset(&sin,0,sizeof(sin));
		sin.sin_addr.s_addr=INADDR_ANY;
		sin.sin_family=AF_INET;
		sin.sin_port=htons(port);
		setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
		
		if(bind(sock,(struct sockaddr *)&sin,sizeof(sin))<0)
			berr_exit("Couldn't bind");
		listen(sock,5);

		printf("cmissd server now accepting connections on port %d\n", port);

		done = 0;
		while(!done)
		{
			/* Accept TCPIP socket connection */
			if((s=accept(sock,0,0))<0)
				err_exit("Problem accepting");

			if (SERVER_MULTITHREADED == server_mode)
			{
				if (pid = fork())
				{
					/* This is the server, it no longer needs a handle to the
						socket */
					close(s);
				}
				else
				{
					/* Open the first SSL connection which is authenticated
						on this servers certificate only so that we can find
						out who the client wants to log in as */
					sbio=BIO_new_socket(s,BIO_NOCLOSE);
					sbio2=BIO_new_socket(s,BIO_NOCLOSE);
					
					start_ssl_server(ctx, sbio, sbio2, s, server_function);
					
					close (s);
				}
			}
			else
			{
				/* Open the first SSL connection which is authenticated
					on this servers certificate only so that we can find
					out who the client wants to log in as */
				sbio=BIO_new_socket(s,BIO_NOCLOSE);
				sbio2=BIO_new_socket(s,BIO_NOCLOSE);
				
				start_ssl_server(ctx, sbio, sbio2, s, server_function);
				
				close (s);
			}
		}
	}
	destroy_ctx(ctx);
}

static int exit_with_usage(char *name, char *usage)
{
	fprintf(stderr,usage,name);
	exit(1);
}

int main(int argc, char **argv)
/*******************************************************************************
LAST MODIFIED : 28 May 2002

DESCRIPTION :
==============================================================================*/
{
	char *name, *public_certificate_filename, *private_key_filename;
	char usage[] = "Usage: %s -c public_certificate_file -k private_key_file "
		"[-h] [-p port_number] [-1 (single thread only mode)] "
		"[-i (inetd stdin/stdout mode)]\n";
	enum Server_mode server_mode;
	int i, port, return_code;

	name = argv[0];
	server_mode = SERVER_MULTITHREADED;
	public_certificate_filename = (char *)NULL;
	private_key_filename = (char *)NULL;
	/* Parse the command line */
	for (i=1;i<argc;i++)
	{
		/* If we have a command line switch */		
		if (argv[i][0] == '-')
		{
			if (strncmp(argv[i],"-c",2) == 0)
			{
				if (argc - i < 1) 
				{
					exit_with_usage(name, usage);
				}
				i++;
				public_certificate_filename = argv[i];
			}
			else if (strncmp(argv[i],"-h",2) == 0)
			{
				exit_with_usage(name, usage);
			}
			else if (strncmp(argv[i],"-i",2) == 0)
			{
				server_mode = SERVER_ONCE_ON_STDIN_STDOUT;
			}
			else if (strncmp(argv[i],"-k",2) == 0)
			{
				if (argc - i < 1) 
				{
					exit_with_usage(name, usage);
				}
				i++;
				private_key_filename = argv[i];
			}
			else if (strncmp(argv[i],"-p",2) == 0)
			{
				if (argc - i < 1) 
				{
					exit_with_usage(name, usage);
				}
				i++;
				if (sscanf(argv[i],"%d",&port) != 1)
				{
					exit_with_usage(name, usage);
				}
			}
			else if (strncmp(argv[i],"-1",2) == 0)
			{
				server_mode = SERVER_SINGLETHREADED;
			}     
			else
			{
				exit_with_usage(name, usage);
			}
		}
		else
		{
			exit_with_usage(name, usage);
		}
	}
	return_code = 1;
	if (!private_key_filename)
	{
		display_message(ERROR_MESSAGE, "You must specify a private key filename.");
		return_code = 0;
	}
	if (!public_certificate_filename)
	{
		display_message(ERROR_MESSAGE, "You must specify a public certificate filename.");
		return_code = 0;
	}
	if (return_code)
	{
		ssl_server_listen(server_mode, port, public_certificate_filename, private_key_filename);
	}
	else
	{
		exit_with_usage(name, usage);
	}
}
