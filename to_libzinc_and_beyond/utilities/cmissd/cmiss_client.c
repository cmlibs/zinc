#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <string.h>

#include <ssl_utilities.h>

#define HOST "130.216.208.133"
#define KEYFILE "client_key.pem"
#define CERTFILE "client_cert.pem"
#define PASSWORD ""

#define USER "blackett\n"
#define BUFFSIZE 1024

BIO *bio_err=0;
static char *pass;
static int password_cb(char *buf,int num,int rwflag,void *userdata);
static void sigpipe_handle(int x);

typedef int (Client_function)(SSL *ssl, int ssl_socket, int id);

enum Client_mode
/*******************************************************************************
LAST MODIFIED : 8 May 2002

DESCRIPTION :
Controls the operation of the server.
==============================================================================*/
{
	CLIENT_UNIX_FILE_TUNNEL,
	CLIENT_TERMINAL
};

/* A simple error and exit routine*/
int err_exit(string)
  char *string;
  {
    fprintf(stderr,"%s\n",string);
    exit(0);
  }

/* Print SSL errors and exit*/
int berr_exit(string)
  char *string;
  {
    BIO_printf(bio_err,"%s\n",string);
    ERR_print_errors(bio_err);
    exit(0);
  }

/*The password code is not thread safe*/
static int password_cb(char *buf,int num,int rwflag,void *userdata)
  {
	  printf ("Requesting Password %d\n", num);

    if(num<strlen(pass)+1)
      return(0);

    strcpy(buf,pass);
    return(strlen(pass));
  }

static void sigpipe_handle(int x){
}

static SSL_CTX *initialize_ctx(char *keyfile,char *certificate,char *password)
  {
    SSL_METHOD *meth;
    SSL_CTX *ctx;
    
    if(!bio_err){
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

    if(!(SSL_CTX_use_certificate_file(ctx,certificate,SSL_FILETYPE_PEM)))
      berr_exit("Couldn't read certificate file");

    pass=password;
    SSL_CTX_set_default_passwd_cb(ctx,password_cb);
    if(!(SSL_CTX_use_PrivateKey_file(ctx,keyfile,SSL_FILETYPE_PEM)))
      berr_exit("Couldn't read key file");

    /* Load the CAs we trust*/
    /* if(!(SSL_CTX_load_verify_locations(ctx, "root.pem", 0)))
		 berr_exit("Couldn't read CA list");*/
    SSL_CTX_set_verify_depth(ctx,1);

    /* Load randomness */
    if(!(RAND_load_file("random.pem",1024*1024)))
      berr_exit("Couldn't load randomness");
       
    return ctx;
  }
      
void destroy_ctx(ctx)
  SSL_CTX *ctx;
  {
    SSL_CTX_free(ctx);
  }

int tcp_connect(int port)
 {
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;
    
    if(!(hp=gethostbyname(HOST)))
      berr_exit("Couldn't resolve host");
    memset(&addr,0,sizeof(addr));
    addr.sin_addr=*(struct in_addr*)hp->h_addr_list[0];
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);

    if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
      err_exit("Couldn't create socket");
    if(connect(sock,(struct sockaddr *)&addr,sizeof(addr))<0)
      err_exit("Couldn't connect socket");
    
    return sock;
  }

/* Check that the common name matches the host name*/
void check_cert_chain(ssl,host)
  SSL *ssl;
  char *host;
  {
    X509 *peer;
    char peer_CN[256];
    
	 /* Check the authenticity of our server certificate
		 if we care to ensure that we are connecting to the 
		 correct place */
#if defined (ONLY_TEMPORARY_SAB)
    if(SSL_get_verify_result(ssl)!=X509_V_OK)
      berr_exit("Certificate doesn't verify");
#endif /* defined (ONLY_TEMPORARY_SAB) */

    /*Check the cert chain. The chain length
      is automatically checked by OpenSSL when we
      set the verify depth in the ctx */

	 /* There should be a server certificate as I believe our
		 SSLv3 method requires one (we aren't using an anonoymous
		 cipher) */
    /*Check the common name*/
    if (peer=SSL_get_peer_certificate(ssl))
	 {
		 X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
			 NID_commonName, peer_CN, 256);

		 printf("name %s\n", peer_CN);

	 /* Check the server is who they say they are */
#if defined (ONLY_TEMPORARY_SAB)
		 if(strcasecmp(peer_CN,host))
			 err_exit("Common name doesn't match host name");
#endif /* defined (ONLY_TEMPORARY_SAB) */
	 }
	 else
	 {
		 /* No server certificate supplied */
		 printf ("No server certificate supplied\n");
	 }
  }

int tcp_unix_file_listen(char *filename)
{
	int sock;
	struct sockaddr_un sun;
    
	if((sock=socket(PF_UNIX,SOCK_STREAM,0))<0)
      err_exit("Couldn't make socket");
    
	memset(&sun,0,sizeof(sun));
	strcpy(sun.sun_path, filename);
	sun.sun_family=AF_UNIX;
    
	if(bind(sock,(struct sockaddr *)&sun,sizeof(sun))<0)
      berr_exit("Couldn't bind");
	listen(sock,2);  

	return(sock);
}

int tunnel(SSL *ssl, int ssl_socket, int id)
{
	char local_socket_name[100];
	int local_socket, return_code, local_connection;
	
	if (!id)
	{
		id = getpid();
	}
	sprintf(local_socket_name, "/tmp/.cmiss%d", id);
	if (local_socket = tcp_unix_file_listen(local_socket_name))
	{

		if((local_connection=accept(local_socket,0,0)) > 0)
		{
			
			/* Now just act as a tunnel */
			if (ssl_socket && local_connection)
			{
				SSL_unix_socket_tunnel(ssl, ssl_socket, local_connection);
			}
			
			close(local_connection);
		}
		else
		{
			display_message(ERROR_MESSAGE,"tunnel.  "
				"Error accepting connection.");
			return_code = 0;
		}
		
		close(local_socket);
		unlink(local_socket_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"tunnel.  "
			"Unable to make local socket.");
		return_code = 0;
	}

	return(return_code);
}

int start_client(int port, enum Client_mode client_mode)
{
	Client_function *client_function;
	int return_code;
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *sbio;
	int r, sock;
	char read_buffer[BUFFSIZE];

	return_code = 1;

	switch (client_mode)
	{
		case CLIENT_UNIX_FILE_TUNNEL:
		{
			client_function = tunnel;
		} break;
		case CLIENT_TERMINAL:
		{
			client_function = terminal_to_SSL;
		} break;
		default:
		{
		} break;
	}

	/* Build our SSL context*/
	ctx=initialize_ctx(KEYFILE,CERTFILE,PASSWORD);

	/* Connect the TCP socket*/
	sock=tcp_connect(port);

	/* Connect the SSL socket */
	ssl=SSL_new(ctx);
	sbio=BIO_new_socket(sock,BIO_NOCLOSE);
	SSL_set_bio(ssl,sbio,sbio);
	if(SSL_connect(ssl)<=0)
      berr_exit("SSL connect error");
	check_cert_chain(ssl,HOST);

	/* Send the user we want to be */
	r=SSL_write(ssl, USER, strlen(USER));
	 
	/* read  */
	while(r=SSL_read(ssl,read_buffer, BUFFSIZE))
	{
		 
	}

	/* Reconnect */
	if((r=SSL_shutdown(ssl)<=0))
		berr_exit("SSL shutdown error");
	/* Connect the SSL socket */
	ssl=SSL_new(ctx);
	sbio=BIO_new_socket(sock,BIO_NOCLOSE);
	SSL_set_bio(ssl,sbio,sbio);
	if(SSL_connect(ssl)<=0)
      berr_exit("SSL connect error");
	check_cert_chain(ssl,HOST);

	(*client_function)(ssl, sock, /*id*/0);

	while((r=SSL_shutdown(ssl))==0)
	{
	}

	close(sock);

	destroy_ctx(ctx);

	return(return_code);
}

int main(int argc, char **argv)
{
	int return_code;
	

	return_code = start_client(/*port*/4433, CLIENT_TERMINAL);

	return(return_code);
}
