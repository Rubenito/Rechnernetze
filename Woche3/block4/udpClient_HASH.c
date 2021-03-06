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
#include <onion/onion.h>
#include <onion/log.h>
#include <signal.h>

#include "favicon.h"

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
onion *o=NULL;

/// Format query string a little bit to understand the query itself
void format_query(const char *key, const char *value, char *temp){
	strcat(temp," ");
	strcat(temp,key);
	strcat(temp,"=");
	strcat(temp,value);
}

static void shutdown_server(int _){
	if (o)
		onion_listen_stop(o);
}

//HTTP handler to forward http requests
int getDel(void *p, onion_request *req, onion_response *res){

	//Query elements
	const char* element = onion_request_get_query(req,"key");
	
	//Get header argument one
	if((onion_request_get_flags(req)&OR_METHODS) == OR_GET) {
		
		//Attempt to get data
		
		
		//Sent back true
		onion_response_set_code(res, 200);
	}

	//Set header and type
	onion_response_set_header(res, "HTTP-Translator", "Forward that data");
	onion_response_set_header(res, "Content-Type", "text/plain");

	//Set example message
	onion_response_write0(res, "First try\n Nice one :)\n");
	onion_response_printf(res,"Path: %s\n",element);

	//Tell if everything worked properly
	return OCS_PROCESSED;
}

//HTTP handler to forward http requests
int put(void *p, onion_request *req, onion_response *res){

	//Get header argument one
	if((onion_request_get_flags(req)&OR_METHODS) == OR_GET) {
		
		//Attempt to get data
		
		//Sent back true
		onion_response_set_code(res, 200);
	}

	//Set header and type
	onion_response_set_header(res, "HTTP-Translator", "Forward that data");
	onion_response_set_header(res, "Content-Type", "text/plain");

	//Set example message
	onion_response_write0(res, "First try\n Nice one :)\n");

	//Tell if everything worked properly
	return OCS_PROCESSED;
}

//Initialize favicon
int initializeFavicon(char *argv[]) {
	//What does this do?
	signal(SIGINT,shutdown_server);
	signal(SIGTERM,shutdown_server);

	//Initialize address and port 
	o=onion_new(O_POOL);
	onion_set_timeout(o, 5000);
	onion_set_hostname(o,"0.0.0.0");
	onion_set_port(o, argv[3]);
	onion_url *urls=onion_root_url(o);
	
	//Add handlers
	
	onion_url_add(urls, "^key=[0-9]+", getDel);
	onion_url_add(urls, "^key=[0-9]+&value=[0-9]+", put);
}

//Build socket
int buildSocket(char *argv[]) {

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
	if((sendto(sockfd, buffer, sizeof(char)*BUFSIZE, 0, (struct sockaddr*) &their_addr, sizeof(their_addr)))<0) {
		perror("Error in Sendto: ");
	}
}

//Receiving primitive
int receiveMessage(unsigned char* buffer){
	bzero(buffer, BUFSIZE);
	int len = sizeof(their_addr);
	if((recvfrom(sockfd, buffer, sizeof(char)*BUFSIZE, 0,
		(struct sockaddr *) &their_addr, &len))<0){
		perror("Error in Recvfrom: ");
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

	//Check if arguments of sufficient size
    if (argc != 4) {
        fprintf(stderr,"Usage: udpClient serverName serverPort ownPort\n");
        exit(EXIT_FAILURE);
    }

    //build sockets
    buildSocket(argv);
    
    //Init favicon
    initializeFavicon(argv);
    

	//TODO: MAIN LOOP	
	onion_listen(o);
	onion_free(o);

    //Close socket
	close(sockfd); 

    return EXIT_SUCCESS;
}

