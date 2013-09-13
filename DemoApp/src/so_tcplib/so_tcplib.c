
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>

#include <stdio.h>

int tcpConnectSocket(char *host, int porto ) {
	
  int								sockfd;
  struct sockaddr_in	serv_addr;
  struct hostent 			*phe;
  in_addr_t					addr;
  
  /* Abrir um socket TCP (an Internet Stream socket) */
  if ( (sockfd = socket( PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 )) < 0 ) return -1; 
  
  /* Preencher a estrutura serv_addr com o endereco do servidor que pretendemos contactar */
  memset( (char*)&serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sin_family      = AF_INET;
  
  if ( (phe = gethostbyname( host ) ) != NULL)  bcopy(phe->h_addr, (char*)(&addr), phe->h_length);
  else if ( (addr = inet_addr(host)) == -1) { close (sockfd); return -2;}
  
  serv_addr.sin_addr.s_addr = addr;
  serv_addr.sin_port             = htons(porto);
  
  /* Ligar-se ao servidor */
  if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0) { close (sockfd); return -3;}
  return(sockfd);
 
 }
 
int tcpCreateServerSocket(char *IPinterface, int porto ) {
	
  in_addr_t					IPintrf =  htonl( INADDR_ANY);   /*Fazer o bindig para todas as interfaces do host */
  int								sockfd;
  struct sockaddr_in	serv_addr;
  
  /* Criar sockect TCP */
  if ( (sockfd = socket( PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 )) < 0 ) return -1;
  
  if( IPinterface ) { /* Cont�m o IP da interface a usar end decimal doted notaion(ex: 127.0.0.1)*/
  	if ( ((IPintrf = inet_addr(IPinterface)) == -1)) { close (sockfd); return -2;} /*converter de string para para bin�rio */
  }
  
  /* Registar endere�o local de modo a que os clientes nos possam contactar */
  memset( (char*)&serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sin_family          = AF_INET;
  serv_addr.sin_addr.s_addr = IPintrf;
  serv_addr.sin_port             = htons( porto );

  if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ) { close (sockfd); return -2;}

 /* Activar socket com fila de espera de dimens�o 5 */
  if ( listen( sockfd, 5 ) < 0 ) { close (sockfd); return -3;}
  return sockfd;
  
}

int tcpAccpetSocket(int srv_socket) {
	
	int							newsockfd;
	struct sockaddr_in	client_addr;
	unsigned int			dim_client;

	dim_client = sizeof( client_addr );
	newsockfd = accept( srv_socket, (struct sockaddr *)(&client_addr), &dim_client );
	if ( newsockfd < 0 ) return -1;
	return newsockfd;
}
   
