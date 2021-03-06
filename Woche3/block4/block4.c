/** Licensed under AGPL 3.0. (C) 2010 David Moreno Montero. http://coralbits.com */
#include <onion/onion.h>
#include <onion/log.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>

#include "favicon.h"

#define server_port 1040
#define HOST "localhost"
#define TIMEOUT_SEC 2
#define MAXREQUESTS 2

int sockfd;
struct sockaddr_in their_addr; // connector's address information
struct hostent *he;
socklen_t len;

unsigned char buffer[8];
int returnState;
int key;
int value;


int favicon(void *p, onion_request *req, onion_response *res){

	onion_response_set_header(res, "Server", "Super Rechnernetze Server TxxGyy v0.1");
	onion_response_set_header(res, "Content-Type", "image/x-icon");

	if (onion_response_write_headers(res)==OR_SKIP_CONTENT) // Maybe it was HEAD.
		return OCS_PROCESSED;

	onion_response_write(res, (const char *)favicon_ico, favicon_ico_len);

	return OCS_PROCESSED;
}

int hello(void *p, onion_request *req, onion_response *res){
	onion_response_write0(res,"Hello world");
	if (onion_request_get_query(req, "1")){
		onion_response_printf(res, "<p>Path: %s", onion_request_get_query(req, "1"));
	}
	onion_response_printf(res,"<p>Client description: %s",onion_request_get_client_description(req));

	onion_response_set_header(res, "Server", "Super Rechnernetze Server TxxGyy v0.1");

	return OCS_PROCESSED;
}

void packData_hash(){

}

void unpackData_hash() {

    //printBuffer("unpackData");
    returnState = (buffer[0]=='O'||buffer[0]=='V')? 0 : -1;
    key = (buffer[4]<<8)|buffer[5];
    value = (buffer[6]<<8)|buffer[7];
}

void packData_http(){

}

/*
*  initialize socket with constants above
*/
int init_socket(){

    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    
    //Resolv hostname to IP Address
    if ((he=gethostbyname(HOST)) == NULL) {  // get the host info
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    //setup transport address
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons((uint16_t) server_port);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    len = (socklen_t) sizeof(their_addr);

    return sockfd;
}

//close socket
void close_socket(){
    close(sockfd);
}

/*
*   send and recv request then unpackData in global variables 
*   returnState (fail/done)
*   key (returned key from server)
*   value (returned value from server)
*/
int request(){
    
    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    FD_SET(sockfd, &readfds);

    returnState = -1;
    int i = 0;
    while(i<MAXREQUESTS){

        sendto(sockfd, &buffer, sizeof(char)*8, 0, 
            (const struct sockaddr*) &their_addr, len);
        
        select(sockfd+1, &readfds, NULL, NULL, &tv);

        if(FD_ISSET(sockfd, &readfds)){
            returnState = (int) recvfrom(sockfd, buffer, sizeof(char)*8, 0, 
            (struct sockaddr*) &their_addr, &len);
            break;
        }else
            printf("TIMEOUT\n");
        i++;
    }

    //perror("send");

    if(returnState < 0)
        return -1;
    
    unpackData_hash();
    return 0;
}
//--------------------------------------------------------------------------------------------
int get_del(void *p, onion_request *req, onion_response *res){

	printf("Warste hier?");
	
//if request is get
	if((onion_request_get_flags(req) & OR_METHODS) == OR_GET){

		//get key
		char* str;
		const char* path = onion_request_get_path(req);
		strncpy(str, path, strlen(path));

		const char* start = strstr(str, "key/");
		strncpy(str, start+4, (int)strlen(str)-(int)start-4);		

		key = atoi(str);

		onion_response_set_code(res, 200);
		
//if request is del
	}else if((onion_request_get_flags(req) & OR_METHODS) == OR_DELETE){

//if request op is not get/del
	}else{
		onion_response_printf(res, "%d\n", onion_request_get_flags(req) & OR_GET);
		onion_response_set_code(res, 503);
	}

	return OCS_PROCESSED;
}

int get_del_collection(void *p, onion_request *req, onion_response *res){

}

int post(void *p, onion_request *req, onion_response *res){

}

onion *o=NULL;

static void shutdown_server(int _){
	if (o)
		onion_listen_stop(o);
}

int main(int argc, char **argv){
	signal(SIGINT,shutdown_server);
	signal(SIGTERM,shutdown_server);

	o=onion_new(O_POOL);
	onion_set_timeout(o, 5000);
	onion_set_hostname(o,"0.0.0.0");
	onion_set_port(o, "4711");
	onion_url *urls=onion_root_url(o);

	onion_url_add(urls, "favicon.ico", favicon);
	onion_url_add(urls, "", hello);
	onion_url_add(urls, "^key=[0-9]+", get_del);
	onion_url_add(urls, "^key/", get_del_collection);
	onion_url_add(urls, "^key=[0-9]+&value=[0-9]+", post);

	onion_listen(o);
	onion_free(o);
	return 0;
}
