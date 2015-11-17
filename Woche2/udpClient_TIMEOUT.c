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
#define RANDSIZE 30
/**
* PRIMITIVES CONCEARNING NETWORK COMMUNICATION
*/

//Global structures
int sockfd;
struct sockaddr_in their_addr; // connector's address information
struct hostent *he;
int server_port;
struct addrinfo* res;
struct addrinfo hints;
int a = 0;
int b = 0;
struct timeval tv;
fd_set readfds;

//Build socket
int buildSocket(char *argv[], int argc) {
    
    //Check if arguments of sufficient size
    if (argc != 3) {
        fprintf(stderr,"Usage: udpClient serverName serverPort\n");
        exit(EXIT_FAILURE);
    }

    //Read out arguments
    server_port = atoi(argv[2]);

    //Resolv hostname to IP Address
    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

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

    //bind socket
    bind(sockfd, res->ai_addr, res->ai_addrlen);
 
 	//Set timeout   
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    
    //Build select structure
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
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

//Sending primitive
int sendMessage(unsigned char* buffer){

	//Try to send
	if((sendto(sockfd, buffer, sizeof(char)*BUFSIZE, 0, (struct sockaddr*) &their_addr, sizeof			(their_addr))) < 0) {
		perror("Error in Sendto: ");
	}
}

//Receiving primitive
int receiveMessage(unsigned char* buffer){
	
	//Attempt to receive an awnser
	int rv = 0;
	while(1) {
	
	    //Rebuild select structure
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		
		//Select
		rv = select(sockfd+1, &readfds, NULL, NULL, &tv);
		printf("RV: %d\n",rv);
		
		//If rv < 0 error
		if(rv < 0) {
			perror("Error in select");
		}else if(rv == 0) {
			printf("Timeout. Attempting again");
			sendMessage(buffer);
		}else if(FD_ISSET(sockfd, &readfds)){
			bzero(buffer, BUFSIZE);
			int len = sizeof(their_addr);
			if((recvfrom(sockfd, buffer, sizeof(char)*BUFSIZE, 0,
					(struct sockaddr *) &their_addr, &len))<0){
				perror("Error in Recvfrom: ");
			}
		}
		
		
	}


}

/**
* PRIMITIVES CONCEARNING THE HASHTABLE
*/

//Add element to Hashtable
int set(int key, int value){
    //Build buffer and initialize it 
    unsigned char buffer[BUFSIZE];
    packData(buffer,"SET",key,value);

    //Send that data to server
    sendMessage(buffer);

    //Receive awnser
	receiveMessage(buffer);

    //Return if methof ran successful
    return (buffer[0] =='O');
}

//Get element from Hashtable
int get(int key, int* error){
    //Build buffer and initialize it 
    unsigned char buffer[BUFSIZE];
    packData(buffer,"GET",key,0);

    //Send that data to server
    sendMessage(buffer);

    //Receive awnser
    receiveMessage(buffer);

    //Unpack data
    char command[4];
    int a,b;
    unpackData(buffer,command,&a,&b);
    
    //Check if problem rann correctly
    *error = buffer[0] == 'V';    

    //Return if methof ran successful
    return b;
}

//Remove element from Hashtable
int removeElement(int key){
    //Build buffer and initialize it 
    unsigned char buffer[BUFSIZE];
    packData(buffer,"DEL",key,0);

    //Send that data to server
    sendMessage(buffer);

    //Receive awnser
    receiveMessage(buffer);

    //Return if methof ran successful
    return (buffer[0] =='O');
}


int main(int argc, char *argv[])
{
    printf("Datagram hash client example\n\n");

    //build sockets
    buildSocket(argv,argc);
    
    //Build random number generator
    srand(time(NULL));
    
    //Build arrays with random keys and values
    int keys[RANDSIZE];
    int values[RANDSIZE];
    int i;
    for(i = 0; i < RANDSIZE; i++) {
        keys[i] = (rand()%65536);
        values[i] = (rand()%65536);
    }
   
    for(i = 0; i < RANDSIZE; i++) {
        if(set(keys[i],values[i])){
            printf("Successfully set <%d,%d>\n",keys[i],values[i]);
        }
    }
    /*
    int error;
    int value;
    for(i = 0; i < RANDSIZE; i++) {
        value = get(keys[i],&error);
        if(error == 1) {
            printf("Successfully got element with value:%u\n",value);
        }else{
            printf("No element with key %u\n",keys[i]);
        }
    }
    
    
   for(i = 0; i < RANDSIZE; i++) {
        if(removeElement(keys[i])){
            printf("Successfully removed key value pait with key:%d\n",keys[i]);
        }
    }
    
    for(i = 0; i < RANDSIZE; i++) {
        value = get(keys[i],&error);
        if(error == 1) {
            printf("Successfully got element with value:%u\n",value);
        }else{
            printf("No element with key %u\n",keys[i]);
        }
    }
    */

    //Close socket
	close(sockfd); 

    return EXIT_SUCCESS;
}

