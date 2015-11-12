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

#define BUFSIZE 8

//Give out buffer
int printBuffer(unsigned char* buffer) {
	printf("Command: %c%c%c%c\tKey: %d\tValue: %d\n",buffer[0],buffer[1],buffer[2],buffer[3],((buffer[4] << 8 )|buffer[5]),((buffer[6] << 8 )|buffer[7]));
} 

//Pack data to avoid LE vs. BE conflict
int packData(unsigned char *buffer, char* command,unsigned int a, unsigned int b) {

	//Convert number a
	buffer[0] = command[0];
	buffer[1] = command[1];
    buffer[2] = command[2];
    buffer[3] = '\0';

	//Convert key
	buffer[4] = a>>8;
	buffer[5] = a % 256;	

    //Convert value
	buffer[6] = b>>8;
	buffer[7] = b % 256;

    return 0;
}

//Unpack le data
void unpackData(unsigned char *buffer,char* command,unsigned int *a, unsigned int *b) {

    //Read out command
    command[0] = buffer[0];
    command[1] = buffer[1]; 
    command[2] = buffer[2];
    command[3] = buffer[3];

	//Le key
	*a = (buffer[4] << 8 )|buffer[5]; 
	
	//Der value
	*b = (buffer[6] << 8 )|buffer[7];
}

//Parse input
int parse(unsigned char* buffer){
	int a,b;
	char command[4];
	unpackData(buffer,command,&a,&b);

	switch (command[0]) {
		case 'S': set(a,b);break;
		case 'G': get(a);break;
		case 'D': del(a);break;
		default: sendError();break;   
	}
}

//

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

	//Create socket

	//Build hints
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // UDP stream sockets
	hints.ai_flags = AI_PASSIVE; 	 // use my IP

	//Get res
	getaddrinfo(NULL,argv[1],&hints,&res);

	//Build socket out of queried data
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	//Bind server
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


		//Show buffer
		printBuffer(buf);


		/*
		* Send Information back
		*/
        if(sendto(sockfd, buf, sizeof(char)*BUFSIZE, 0, (struct sockaddr*) &clientaddr, sizeof(clientaddr)) < 0) {
			perror("Error Sending GGT back\n");
		}        
		
	}

    //Close socket
	close(sockfd); 

    return EXIT_SUCCESS;
}

