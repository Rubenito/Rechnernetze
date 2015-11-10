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

#define BILLION  1000000000L
#define MAX_BUFFER_LENGTH 100


//Unpack le data
void unpackData(unsigned char *buffer, unsigned int *a, unsigned int *b) {

	//Le a
	*a = (buffer[0] << 8 )|buffer[1]; 
	
	//Der b
	*b = (buffer[2] << 8 )|buffer[3];
}

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
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE; 	 // use my IP

	//Get res
	getaddrinfo(NULL,argv[1],&hints,&res);

	//Build socket out of queried data
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);


    /* ******************************************************************
    TO BE DONE:  Binding
    ******************************************************************* */
	bind(sockfd, res->ai_addr, res->ai_addrlen);

	//Listen to connections
	listen(sockfd, 1);

	//Accept connections
    unsigned char buffer[4];
	//Build necessary structs
	struct timespec requestStart, requestEnd;

	while(1) {
		int addr_size = sizeof(their_addr);
		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
		if(new_fd == -1) {
			continue;
		}
		//Stop time before sending
		clock_gettime(CLOCK_REALTIME, &requestStart);
		printf("Read in bytes: %d\n",(int) recv(new_fd, buffer, 32, 0));
		//Stop time after
		clock_gettime(CLOCK_REALTIME, &requestEnd);
		unpackData(buffer, &a,&b);
		printf("a: %d b: %d\n",a,b);
		printf("The GCT is %d\n",getGCD(a,b));

		double time_before = requestStart.tv_sec+(double)requestStart.tv_nsec/(double)BILLION;
		double time_after = requestEnd.tv_sec+(double)requestEnd.tv_nsec/(double)BILLION;

		printf("Time before receive:%lf \tTime after receive:%lf\n",time_before,time_after);
	}

    /* ******************************************************************
    TO BE DONE:  Close socket
    ******************************************************************* */
	close(sockfd); 

    return EXIT_SUCCESS;
}

