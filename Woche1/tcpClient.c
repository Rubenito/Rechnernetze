/*
############################################################################
#                                                                          #
# Copyright TU-Berlin, 2011-2015                                           #
# Die Weitergabe, Veröffentlichung etc. auch in Teilen ist nicht gestattet #
#                                                                          #
############################################################################
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAX_BUFFER_LENGTH 100


int packData(unsigned char *buffer, unsigned int a, unsigned int b) {
    /* ******************************************************************
    TO BE DONE:  pack data
    ******************************************************************* */

	//Convert number a
	buffer[0] = a>>8;
	buffer[1] = a % 256;

	//Convert number
	buffer[2] = b>>8;
	buffer[3] = b % 256;	

    return 0;
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int a = 0;
    int b = 0;
	char* host = "www.tu-berlin.de";
	char* server_port = "80";
	struct addrinfo* res;
	struct addrinfo hints;

    printf("Streaming socket client example\n\n");


    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */

	//Resolv hostname to IP Address
    if ((he=gethostbyname(host)) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

	//Build hints
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	//Get res
	int status;
	if((status = getaddrinfo(host,server_port,&hints,&res)) != 0 ) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    	exit(1);
	}

	//Build socket out of queried data
	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		perror("Building socket failed: ");
	}

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons((uint16_t)atoi(server_port));
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

	//Connecting
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("Connecting failed: ");
	}
	
	//Build structures for sending of data
	char* input_buffer = malloc(20000);
	char* httpget = "Get / HTTP/1.1\r\nHost:www.tu-berlin.de\r\n\r\n";

    /* ******************************************************************
    TO BE DONE:  Send data
    ******************************************************************* */
	send(sockfd,httpget,strlen(httpget),0);
	
	while(recv(sockfd,input_buffer,19999,0) > 0){
		printf("%s\n",input_buffer);
	}

    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */
	close(sockfd); 


    return EXIT_SUCCESS;
}

