#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h> //socket close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>//
#include <arpa/inet.h> //
#include <pthread.h> //
#include "openssl/ssl.h"
#include "openssl/err.h"
#include <errno.h>
#define len 1024 //max buffer size
char username[20]; //username
struct parameter {
	int server;
	int port;
};

//SSL functions
//Init server instance and context
SSL_CTX* InitCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();		/* Load cryptos, et.al. */
    SSL_load_error_strings();			/* Bring in and register error messages */
    method = SSLv3_client_method();		/* Create new client-method instance */
    ctx = SSL_CTX_new(method);			/* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

SSL_CTX* InitServerCTX(void)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  //load & register all cryptos, etc.
    SSL_load_error_strings();   // load all error messages */
    method = SSLv3_server_method();  // create new server-method instance
    ctx = SSL_CTX_new(method);   // create new context from method
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

//Load the certificate
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
    //set the local certificate from CertFile
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    //set the private key from KeyFile
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    //verify private key
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); // get the server's certificate
    if ( cert != NULL )
    {
        printf("---------------------------------------------\n");
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       // free the malloc'ed string
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);\
        free(line);       // free the malloc'ed string
        X509_free(cert);     // free the malloc'ed certificate copy */
        printf("---------------------------------------------\n");
    }
    else
        printf("No certificates.\n");
}

int OpenConnection(const char *hostname, int port)
{
	int sd;
	struct hostent *host;
	struct sockaddr_in addr;

	if((host = gethostbyname(hostname))==NULL)
	{
		perror(hostname);
		abort();
	}

	sd = socket(PF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = *(long*)(host->h_addr);
	if(connect(sd,(struct sockaddr*)&addr, sizeof(addr))!=0)
	{
		close(sd);
		perror(hostname);
		abort();
	}

	return sd;
}

int OpenListener(int port)
{
    int sd;
    struct sockaddr_in addr;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 ) //bind the IP and port number
    {
        perror("can't bind port");
        abort();
    }
    if ( listen(sd, 10) != 0 )
    {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}

void GetLocal(int sd)
{
	struct sockaddr_in local_sockaddr;
	socklen_t leng;
	leng=sizeof(struct sockaddr_in);
	if(getsockname(sd,(struct sockaddr*) &local_sockaddr, &leng)==-1) {
		 perror("getsockname");
	}
	printf("Connection from: %s:%d\n",inet_ntoa(local_sockaddr.sin_addr), ntohs(local_sockaddr.sin_port));
}

void GetRemote(int sd)
{
	struct sockaddr_in remote_sockaddr;
	socklen_t leng;
	if(getpeername(sd,(struct sockaddr*) &remote_sockaddr, &leng)==-1) {
		perror("getpeername");
	 }
	printf("Connection to: %s:%d\n",inet_ntoa(remote_sockaddr.sin_addr), ntohs(remote_sockaddr.sin_port));

}

int Register(int server)
{
	char acc[20];
	char msg[len];
	memset(msg, '\0', len); //init
	printf("Please enter User account(within 9 character):\n");
	scanf("%s",acc);
	if(strlen(acc)>9)
	{
		printf("Error, register account overflow\n");
		return -1;
	}
	strcat(msg,"REGISTER#");
	strcat(msg,acc);
	//printf("//////////////DEBUG//////////: %s\n", msg);
	send(server, msg, strlen(msg),0);
	return 0;
}

int Login(int server)
{
	char acc[20];
	char port[20];
	char msg[len];
	memset(msg, '\0', len); //init
	printf("Please enter user account:\n");
	scanf("%s",acc);
	if(strlen(acc)>9)
	{
		printf("Error,login account overflow\n");
		return -1;
	}
	strcpy(username,acc); //record username;
	strcat(msg,acc);
	printf("Please enter user port(1024-65535):\n");
	scanf("%s",port);
	strcat(msg,"#");
	strcat(msg,port);
	if(atoi(port)<1024 || atoi(port) > 65535)
	{
		printf("Error, invalid port\n");
		return -1;
	}

	//puts(msg);
	send(server, msg, strlen(msg),0);
	return atoi(port);
}

void List(int server)
{
	char msg[]="List";
	send(server, msg, strlen(msg),0);
}

void Exit(int server)
{
	char msg[]="Exit";
	send(server, msg, strlen(msg),0);
}


int ReceiveMsg(int server)
{
	char buff[len];
	int numBytes;
	memset(buff, '\0', len); //init
	if ( (numBytes=recv(server,buff,len,0)) == -1 )
	{
		perror("recv");
		exit(1);
    	return -1;
	}
	else{
		//buff[numBytes] = '\0';
		printf("Server:\n%s",buff);
	}
	if((strncmp(buff,"your are not",12))==0)
		return -1;
	if((strncmp(buff,"repeated",8))==0)
		return -1;
	if((strncmp(buff,"220",3)) ==0 )
		return -1;

	return 1;
}

int Transfer (int server)  // to correct
{
	//SSL var
	SSL_CTX *ctx;
	SSL *ssl;
	SSL_library_init();
	ctx = InitCTX();
	ssl = SSL_new(ctx);
	//
	int client;
	char buff[len];
	char msg[len];
	char payAmout[10];
	char payee[20];
	int numBytes;
	memset(buff, '\0', len); //init
	List(server); //send list request
	if ( (numBytes=recv(server,buff,len,0)) == -1 )
	{
		perror("recv");
		exit(1);
    	return -1;
	}
	else{
		//printf("Server:\n%s",buff);
		memset(msg, '\0', len);
		printf("Please enter payee account(within 9 char):\n");
		scanf("%s", payee);
		printf("Please enter payAmout:\n");
		scanf("%s", payAmout);
		strcat(msg,username);
		strcat(msg,"#");
		strcat(msg,payAmout);
		strcat(msg,"#");
		strcat(msg,payee);
		char *h;
		char *p;
		//find user ip and port from buffer of list recv from server
		char *par;
		par = strtok(buff, "\n");
		par = strtok(NULL, "\n");
		par = strtok(NULL, "\n");
		int found = 0;
		while(par != NULL)
		{
			int pos = strlen(par) - strlen(strchr(par,'#'));
			char parAcc [10];
			memset(parAcc,0,10);
			strncpy(parAcc, par, pos);
			//printf("%s\n", par);
			if(strcmp(payee,parAcc)==0)
			{
				strtok(par,"#");
				h = strtok(NULL,"#");
				p = strtok(NULL,"\n");
				//printf("Found User:%s\n", par);
				found = 1;
				break;
			}
			//printf("%s\n",par);
			par = strtok(NULL, "\n");
		}
		if(found == 0)
		{
			printf("Transfer failure: user offline or does not exist\n");
			return -1;
		}
		client = OpenConnection(h, atoi(p));
		SSL_set_fd(ssl, client);
		if ( SSL_connect(ssl) == -1 )   // perform the connection
		{
			ERR_print_errors_fp(stderr);
		}
		else
		{
			printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
			ShowCerts(ssl);
			//SSL_get_cipher(ssl);
			SSL_write(ssl, msg, strlen(msg));
			SSL_free(ssl);
		}
		//GetLocal(client);
		//GetRemote(client);
		//int count = send(client, msg, strlen(msg),0);
		//printf("%s:%d\n",msg,count);

	}
	SSL_CTX_free(ctx);
	close(client);
	return client;


}

void * ClientListner(void * param)
{
	//SSL var
	SSL_CTX *ctx;
	SSL_library_init(); //init SSL library
	ctx = InitServerCTX();  //initialize SSL
    LoadCertificates(ctx, "mycert.pem", "mykey.pem"); // load certs and key
	SSL *ssl;
	//listening interaction msg from another clients
	int port = ((struct parameter*)param)->port;
	int server = ((struct parameter*)param)->server;
	int peer = OpenListener(port); //set listener for clients
	int my_server;
	struct sockaddr_in addr;
	socklen_t leng = sizeof(addr);
	char buff[len];
	while (1)
	{
		//GetLocal(my_server);
		//GetRemote(my_server);
		//printf("Connection from: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		my_server=accept(peer, (struct sockaddr*)&addr, &leng);
		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, my_server);
		if ( SSL_accept(ssl) == -1 ) //do SSL-protocol accept
		{
			ERR_print_errors_fp(stderr);
		}
		else
		{
			int bytes;
			while((bytes = SSL_read(ssl,buff,len)))
			{
				if(bytes > 0)
				{
					printf("\n%s\n", buff);
					send(server, buff, strlen(buff), 0);
					ReceiveMsg(server);
					memset(buff,0,len);
				}
			}
		}

	}
	SSL_CTX_free(ctx);
	close(my_server);
	close(peer);
	pthread_exit(0);
	return NULL;
}


int main(int argc, char *argv[])
{
	int server;
	char *hostname, *portnum;
	if(argc !=3)
	{
		printf("usage: %s <hostname> <portnum>\n", argv[0]);
		exit(0);
	}

	hostname = argv[1];
	portnum = argv[2];

	server = OpenConnection(hostname, atoi(portnum));
	//GetLocal(server); //Check the port client listening to
	//GetRemote(server); //Check the port client connected to
	printf("Connected to %s:%d\n", hostname, atoi(portnum));
	ReceiveMsg(server);


	int online = 0;
	int end = 0;
	while(end != 1)
	{
		int port = 0;
		int order = 0;


		while((order ==0 ||order==1 || order==2) && online !=1)
		{
			printf("Please enter command (1)Resgister (2)Login (3)Exit: ");
			scanf("%d", &order);
			if(order == 1)
			{
				Register(server);
				ReceiveMsg(server);
			}
			else if(order == 2)
			{
				while((port=Login(server))<0){};

				if(ReceiveMsg(server) > 0)
					online = 1;
			}
			else
			{
				Exit(server);
				ReceiveMsg(server);
				end = 1;
				break;
			}
		}
		int client = 0;
		struct parameter p;
		p.server = server;
		p.port = port;
		void *ptr = &p;
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid, &attr, ClientListner, ptr);
		//pthread_join(tid, NULL);
		while(online == 1)
		{
			printf("Please enter command (1)List (2)Transfer (3)Exit: ");
			scanf("%d", &order);
			if(order == 1)
			{
				List(server);
				ReceiveMsg(server);
			}
			else if(order ==2)
			{
				 //get user list from server and Transfer
				client = Transfer(server);
				ReceiveMsg(server);
			}
			else
			{
				Exit(server);
				ReceiveMsg(server);
				end =1;
				break;
			}
		}
	}

	close(server);
}
