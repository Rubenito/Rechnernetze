/*
	Onion HTTP server library
	Copyright (C) 2011 David Moreno Montero
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.
	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	*/

#include <signal.h>
#include <onion/log.h>
#include <onion/onion.h>
#include <onion/shortcuts.h>

/**
 * @short This handler just answers the content of the POST parameter "text"
 * 
 * It checks if its a HEAD in which case it writes the headers, and if not just printfs the
 * user data.
 */
onion_connection_status post_data(void *_, onion_request *req, onion_response *res){
	if (onion_request_get_flags(req)&OR_HEAD){
		onion_response_write_headers(res);
		return OCS_PROCESSED;
	}
	const char *user_cmd=onion_request_get_post(req,"cmd");
	const char *user_key=onion_request_get_post(req,"key");
	const char *user_val=onion_request_get_post(req,"value");
	
	onion_response_printf(res, "The user wrote: CMD: %s Key: %s Value: %s\n", user_cmd, user_key, user_val);
	return OCS_PROCESSED;
}

onion *o=NULL;

void onexit(int _){
	ONION_INFO("Exit");
	if (o)
		onion_listen_stop(o);
}

/**
 * This example creates a onion server and adds two urls: the base one is a static content with a form, and the
 * "data" URL is the post_data handler.
 */
int main(int argc, char **argv){
	o=onion_new(O_ONE_LOOP);
	onion_url *urls=onion_root_url(o);
	onion_set_hostname(o,"0.0.0.0");
	onion_set_port(o, "4711");
	

	onion_url_add(urls, "data", post_data);
	
	
	/*
		
	SOCKET hashServer;
	
	while(1) {
	SOCKET client annehmen;
	if(client.cmd = GET/SET/DEL) ..
		hashServer.GET/SET/DEL ..
			message = hashServer...
				message an client schicken	
	}		
		
	*/
		
		
	

	signal(SIGTERM, onexit);	
	signal(SIGINT, onexit);	
	onion_listen(o);

	onion_free(o);
	return 0;
}