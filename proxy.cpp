#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#include <deque>
#include <cstring>
#define CACHE_FILE_NAME "cache.txt"
#define CACHE_SIZE 3
#define MAX_NO_OF_TCP_CONNECTION 10
#define BUFFER_SIZE 512
using namespace std;


int PROXY_PORT, DNS_PORT;

int printError(string err_msg){
    cout << err_msg << endl;
    exit(-1);
}

void load_cache(deque<pair<string,string>>& cache){
	FILE* fp = fopen(CACHE_FILE_NAME,"r");
	char s1[100], s2[100];

	while(!feof(fp))
	{
		fscanf(fp,"%s %s",s1, s2);
		if(feof(fp) == true)
			break;
		cache.push_back({string(s1),string(s2)});
	}
	fclose(fp);
	return;
}

void store_cache(deque<pair<string,string>>& cache){
	FILE* fp = fopen(CACHE_FILE_NAME,"w");
	
	for(auto it = cache.begin(); it != cache.end(); it++)
	{
		fprintf(fp,"%s %s\n",(it->first).c_str(), (it->second).c_str());
	}
	fclose(fp);
	return;
}

void updateCache(string ip_address, string domain_name){
	deque<pair<string,string>> cache;
	load_cache(cache);
    if(cache.size() == CACHE_SIZE)
        cache.pop_front();

    cache.push_back(make_pair(ip_address, domain_name));

    store_cache(cache);
}

void connect_to_DNS_Server(char* buffer_request, char* buffer_reply, char* DNS_IP_address){
    int socket_client;
    struct sockaddr_in DNS_server_address;
    int conn_status;

    // creating new client socket    
    socket_client = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_client == -1) 
    	printError("[-] Error creating client socket to query a DNS server!");
    else
    	cout<<"[+] Socket created to query a DNS server..."<<endl;
    
	//setting parameter to connect DNS server
    DNS_server_address.sin_family = AF_INET;
    DNS_server_address.sin_addr.s_addr = inet_addr(DNS_IP_address);
    DNS_server_address.sin_port = htons(DNS_PORT);

    // Connecting to DNS server
    conn_status = connect(socket_client, (struct sockaddr *)&DNS_server_address, sizeof(DNS_server_address));
    if(conn_status == -1) 
    	printError("Error connecting to the DNS server!");
    else
    	cout<<"[+] Connected to DNS server..."<<endl;

    // sending DNS query to DNS server
    write(socket_client, buffer_request, BUFFER_SIZE);
    read(socket_client, buffer_reply, BUFFER_SIZE);
    close(socket_client);
}



void findDomainName(char* ip_address, char* domain_name, char* DNS_IP_address){
    string ip_addr(ip_address + 2);
    domain_name[0] = '3';
    domain_name[1] = '#';
    // searching for the entry in the cache
    deque<pair<string,string>> cache;

    load_cache(cache);

    for(auto mapping : cache){
        if(mapping.first == ip_addr){
            cout << "Found matching entry in the cache: ";
            strcpy(domain_name + 2, mapping.second.c_str());
            return;
        }
    }

    // forwarding request to DNS server
    cout << "No entry found in the cache!" << endl;
    cout << "Forwarding request to DNS server..." << endl;
    connect_to_DNS_Server(ip_address, domain_name, DNS_IP_address);    
    
    cout << "Reply from DNS server: ";
    if(domain_name[0] == '3') 
    	updateCache(ip_addr, string(domain_name+2));
}

void findIPAddress(char* domain_name, char* ip_address, char* DNS_IP_address){
    string domain(domain_name+2);
    ip_address[0] = '3';
    ip_address[1] = '#';
	deque<pair<string,string>> cache;
	load_cache(cache);

    // searching for the entry in cache
    for(auto mapping:cache){
        if(mapping.second == domain){
            cout << "Found matching entry in the cache: ";
            strcpy(ip_address+2, mapping.first.c_str());
            return;
        }
    }

    // forwarding request to the DNS server
    cout << "No entry found in the cache!" << endl;
    cout << "Forwarding request to DNS server..." << endl;
    connect_to_DNS_Server(domain_name, ip_address, DNS_IP_address);
    cout << "Reply from DNS server: ";
    if(ip_address[0] == '3') 
    	updateCache(string(ip_address+2), domain);
}

int main(int argc, char* argv[]){
    
	if(argc != 4) 
    	printError("Insufficient no of arguments are provided...");
    
    FILE* fp;
    fp = fopen(CACHE_FILE_NAME, "w");
    fclose(fp);
    // Proxy port number
	PROXY_PORT = atoi(argv[1]);
    
    // DNS port number
	DNS_PORT = atoi(argv[3]);

    int socket_server;
    struct sockaddr_in socket_address;
    int socket_address_len;
    int new_socket;
    
    // creating new socket for communication
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_server == -1) 
    	printError("[-] Not able to create socket!");
   else
   		cout<<"[+] Socket created..."<<endl;


    // setting socket parameters
    socket_address_len = sizeof(socket_address);
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(PROXY_PORT);

    // binding IP address and port number
    if(bind(socket_server, (struct sockaddr *)&socket_address,  sizeof(socket_address)) == -1) 
        printError("Error in binding IP address & port!");
    else
    	cout<<"[+] Successfully bind the socket with port and ip address"<<endl;
    	
    // waiting for connection requests from client
    if(listen(socket_server, MAX_NO_OF_TCP_CONNECTION) == -1) 
    	printError("Error while listerning!");
	else
		cout<<"[+] Listening...\n";
	
	cout << "-------------------------------------------------------" << endl;

	while(true)
    {
		// establishing connection with the client
		new_socket = accept(socket_server, (struct sockaddr *)&socket_address, (socklen_t *)&socket_address_len);
		if(new_socket == -1) 
			printError("Accept failure!");
		else
			cout<<"[+] Accepted a new client connection...\n";
		pid_t pid;
    
		if( (pid = fork()) == 0)
		{
			close(socket_server);
	 	
		 	while(true)
		 	{
				cout << "-------------------------------------------------------" << endl;
				
				// retrieving DNS query
				char buffer_recv[BUFFER_SIZE] = {0};
				char buffer_send[BUFFER_SIZE] = {0};
				
				read(new_socket, buffer_recv, BUFFER_SIZE);
				
				// Client want to close his connection
				if(buffer_recv[0] == '5'){
					break;
				}
				
				// processing DNS query
				if(buffer_recv[0] == '1') 
					findIPAddress(buffer_recv, buffer_send, argv[2]);
				else 
					findDomainName(buffer_recv, buffer_send, argv[2]);
				
				cout << buffer_send << endl;
				
				write(new_socket, buffer_send, BUFFER_SIZE);
			}
			cout << "[+] Connection Closed..." << endl;
    		close(new_socket);
    		return 0;
		}

   }
    return 0;
}
