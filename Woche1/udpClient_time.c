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

#define MAX_BUFFER_LENGTH 100
#define BILLION  1000000000L


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
    int server_port;
    int a = 0;
    int b = 0;

	struct addrinfo* res;
	struct addrinfo hints;

    printf("Datagram socket client example\n\n");

    if (argc != 5) {
        fprintf(stderr,"Usage: udpClient serverName serverPort int1 int2\n");
        exit(EXIT_FAILURE);
    }

    server_port = atoi(argv[2]);
    a = atoi(argv[3]);
    b = atoi(argv[4]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    /* ******************************************************************
    TO BE DONE: Create socket
    ******************************************************************* */


	//Build hints
	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // TCP stream sockets

	//Get res
	getaddrinfo(argv[1],argv[2],&hints,&res);

	//Build socket out of queried data
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons((uint16_t)server_port);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    /* ******************************************************************
    TO BE DONE:  Binding
    ******************************************************************* */
bind(sockfd, res->ai_addr, res->ai_addrlen);

    unsigned char buffer[4];

    packData(buffer, a, b);

    /* ******************************************************************
    TO BE DONE:  Send data
    ******************************************************************* */
	//Build necessary structs
	struct timespec requestStart, requestEnd;

	//Stop time before sending
	clock_gettime(CLOCK_REALTIME, &requestStart);

	sendto(sockfd, buffer, sizeof(char)*4, 0, (struct sockaddr*) &their_addr, sizeof(their_addr));

	//Stop time after
	clock_gettime(CLOCK_REALTIME, &requestEnd);

	double time_before = requestStart.tv_sec+(double)requestStart.tv_nsec/(double)BILLION;
	double time_after = requestEnd.tv_sec+(double)requestEnd.tv_nsec/(double)BILLION;

	printf("Time before send:%lf \tTime after send:%lf\n",time_before,time_after);

    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */
	close(sockfd); 


    return EXIT_SUCCESS;
}

