#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h> 
#define BUFFER_SIZE 512

using namespace std;

int printError(string err_msg){
    cout << err_msg << endl;
    exit(0);
}

int main(int argc, char* argv[]){
    
    if(argc != 3) 
    	printError("Insufficient no of arguments...\n");

    struct sockaddr_in server_address;
    int socket_client;
    int conn_status;
    int type;
    // Creating socket 
    socket_client = socket(AF_INET, SOCK_STREAM, 0);
        
    if(socket_client == -1) 
    	printError("[-] Error while creating socket...\n");
    else
    	cout<<"[+] Socket created."<<endl;
    
    
    // setting up server address structure    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // establishing connection with the proxy server
    conn_status = connect(socket_client, (struct sockaddr *)&server_address, sizeof(server_address));
    
    if(conn_status == -1) 
    	printError("[-] Error connecting to the server!");
    else 
    	cout << "[+] Connected to the server..." << endl;
    

    while(true){
        
        // creating DNS query
        char buffer_request[BUFFER_SIZE] = {0};
    	
    	cout << "------------------------------------------------------" << endl;
        cout << "1) Domain name to IP conversion " << endl;
        cout << "2) Ip to Domain Name Conversion " << endl;
        cout << "5) Exit" << endl;
        cout << "Enter your choice: ";
        cin >> type;
		
        if(type == 5){
            write(socket_client, "5#Close", BUFFER_SIZE);
            break;
        }
        else if(type == 1)
        {
            cout << "Enter Domain Name : ";
            buffer_request[0] = '1';
        }
        else if(type == 2)
        {
        	cout << "Enter IP Address: ";        
        	buffer_request[0] = '2';
        }
        else{
        	cout<<"no option availabe...\n";
        	continue;
        } 

        buffer_request[1] = '#';
        cin >> buffer_request + 2;

        write(socket_client, buffer_request, BUFFER_SIZE);

        char buffer_reply[BUFFER_SIZE] = {0};
        
        read(socket_client, buffer_reply, BUFFER_SIZE);

		// successfull reply from the server
        if(buffer_reply[0] == '3'){
            if(type == 1)
				cout << "The IP address is: ";
            else
				cout << "The domain name is: ";
        }
        cout << buffer_reply+2 << endl;
    }

    // closing connection
    cout<<"[+] Connection closed..."<<endl;
    close(socket_client);
    return 0;
}
