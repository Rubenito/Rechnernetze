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
#include <time.h>

#define BUFSIZE 4


//Unpack le data
void unpackData(unsigned char *buffer, unsigned int *a, unsigned int *b) {

	//Le a
	*a = (buffer[0] << 8 )|buffer[1]; 
	
	//Der b
	*b = (buffer[2] << 8 )|buffer[3];
}

//Pack that shit back
int packData(unsigned char *buffer, unsigned int a, unsigned int b) {

	//Convert number a
	buffer[0] = a>>8;
	buffer[1] = a % 256;

	//Convert number
	buffer[2] = b>>8;
	buffer[3] = b % 256;	

    return 0;
}

//Wow, such ggt, many dividing
int getGCD(int a, int b) {
	int rest;
	if (a < 0) a = -a;
	if (b < 0) b = -b;
	while (rest != 0) {
		rest = a % b;
		a = b;
		b = rest;	
	}

	return a;
}


int main(int argc, char *argv[])
{
    int sockfd;
	struct sockaddr_storage their_addr; // connector's address information
    struct hostent *he;
    int server_port;
    int a = 0;
    int b = 0;

	struct addrinfo* res;
	struct addrinfo hints;

    printf("Streaming socket client example\n\n");

    if (argc != 2) {
        fprintf(stderr,"Usage: tcpServer serverPort\n");
        exit(EXIT_FAILURE);
    }


    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */


	//Build hints
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // UDP stream sockets
	hints.ai_flags = AI_PASSIVE; 	 // use my IP

	//Get res
	getaddrinfo(NULL,argv[1],&hints,&res);

	//Build socket out of queried data
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


    /* ******************************************************************
    TO BE DONE:  Binding
    ******************************************************************* */
	bind(sockfd, res->ai_addr, res->ai_addrlen);

    unsigned char buf[BUFSIZE]; /* message buf */

    //Structure to memorize adress of calling client
    struct sockaddr_in clientaddr; /* client addr */
    int clientlen = sizeof(clientaddr); /*size of address struct*/
    char *hostaddrp; /* dotted decimal host addr string */

	while(1) {

        /*
        * recvfrom: receive a UDP datagram from a client
        */
        bzero(buf, BUFSIZE);
        if(recvfrom(sockfd, buf, BUFSIZE, 0,
		    (struct sockaddr *) &clientaddr, &clientlen)<0){
            perror("Error in Recvfrom: ");
        }

        /* 
        * gethostbyaddr: determine who sent the datagram
        */
        struct hostent *hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
		                        sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        
        if (hostp == NULL) {
            perror("ERROR on gethostbyaddr");
        }

        /*
        * Convert host adress to dotted string
        */
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) {
            perror("ERROR on inet_ntoa\n");
        }

        /*
        * Give out sender
        */
        printf("server received datagram from %s (%s)\n", 
	        hostp->h_name, hostaddrp);

        /*
        * unpack data and print out result  
        */
		unpackData(buf, &a,&b);
		a = getGCD(a,b);
        packData(buf,a,0);

		/*
		* Send Information back
		*/
        if(sendto(sockfd, buf, sizeof(char)*BUFSIZE, 0, (struct sockaddr*) &clientaddr, sizeof(clientaddr)) < 0) {
			perror("Error Sending GGT back\n");
		}        
		
	}

    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */
	close(sockfd); 

    return EXIT_SUCCESS;
}

