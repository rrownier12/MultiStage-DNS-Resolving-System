#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <sys/socket.h> 
#include <cstring>
#define BUFFER_SIZE 512
#define DATABASE_NAME "database.txt"
using namespace std;

int PORT;

vector<pair<string,string>> ip_domain_mapping;

int printError(string err_msg){
	printf("%s \n",err_msg.c_str());
    exit(-1);
}

void find_domain_name_mapping(char* ip_address, char* domain_name){
    string ip_addr(ip_address);
    domain_name[0] = '3';
    domain_name[1] = '#';
    
    // searching for the entry in the database
    for(auto entry:ip_domain_mapping){
        if(entry.first == ip_addr){
			printf("Found matching entry in DNS server database: ");
            strcpy(domain_name + 2, entry.second.c_str());
            return;
        }
    }

    domain_name[0] = '4';
    string msg = "No entry found in DNS server database!";
    strcpy(domain_name+2, msg.c_str());
}

void find_ip_address_mapping(char* domain_name, char* ip_address){
    string domain(domain_name);
    ip_address[0] = '3';
    ip_address[1] = '#';

    // searching for the entry in the database
    for(auto entry:ip_domain_mapping){
        if(entry.second == domain){
        	printf("Found matching entry in DNS server database: ");
            strcpy(ip_address+2, entry.first.c_str());
            return;
        }
    }

    ip_address[0] = '4';
    string msg = "No entry found in DNS server database!";
    strcpy(ip_address+2, msg.c_str());
}

// reading IP address to domain name mapping from database
void readDatabase(){
    
    char ip[50], domain[50];
    FILE* fp = fopen(DATABASE_NAME, "r");
    while(!feof(fp))
    {
    	fscanf(fp,"%s %s", ip, domain);
    	ip_domain_mapping.push_back({string(ip), string(domain)});
    }

}

int main(int argc, char* argv[]){
    if(argc != 2) 
    	printError("Invalid number of arguments!");
    
    // Sotring Database entries in local map 
	readDatabase();
    
    struct sockaddr_in socket_address;
    int socket_address_len;
    int socket_server;
    
    PORT = atoi(argv[1]);
    
    // creating new socket for communication
    
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_server != -1) 
    	printf("[+] Socket created \n");
    else
	    printError("[-] Could not create socket!");


    // configuring socket pair attributes
   	socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(PORT);
	socket_address_len = sizeof(socket_address);
    // binding IP address and port number 

    if(bind(socket_server, (struct sockaddr *)&socket_address,  sizeof(socket_address)) == -1) 
        printError("[-] Error in binding IP address & port!");
	else
		printf("[+] IP address and port number bind sucessfully to the socket \n");
    
    // waiting for connection requests
    if(listen(socket_server, 3) != -1) 
    	printf("[+] Listerning...\n");
    else
    	printError("[-] Error while listerning!");
    	
    while(true){
		printf("---------------------------------------------------\n");
		// establishing connection with the client
		int new_socket = accept(socket_server, (struct sockaddr *)&socket_address,  (socklen_t *)&socket_address_len);
			
		if(new_socket == -1)
			printError("[-] Error occured while accepting request");
		else 
			printf("[+] Successfully accepted client request. \n");

		
		// retrieving DNS query
		char buffer_recv[BUFFER_SIZE] = {0};
		char buffer_send[BUFFER_SIZE] = {0};
			
		while(read(new_socket, buffer_recv, BUFFER_SIZE) == 0);

		// Searching IP address mapping in database
		if(buffer_recv[0] == '2') 
			find_domain_name_mapping(buffer_recv+2, buffer_send);
		else 
			find_ip_address_mapping(buffer_recv+2, buffer_send);
		
		printf("%s \n", buffer_send);
		
		write(new_socket, buffer_send, BUFFER_SIZE);
		
		close(new_socket);
		printf("[+] closes the connection \n");
		
	}
	printf("[+] Connection closed \n");
    close(socket_server);
	return 0;
}
  
