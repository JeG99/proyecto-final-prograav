/*
	Live Server on port 8888
*/
#include<io.h>
#include<windows.h>
#include<winsock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

void write_file(int s){
	int n;
	char buffer[11514196];

		
		n = recv(s,buffer, 11514196,0);


		
		
		if(n>0){
		for(int i=0;i<100;i++){
		printf("%c",buffer[i]);
		
		}

		}
}

int main(int argc , char *argv[])
{
	WSADATA wsa;
	SOCKET s , new_socket;
	struct sockaddr_in server , client;
	int c;
	char *message;

    // Initializes the library
	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}
	
	printf("Library initialized\n");
	
	//Create a socket
    // Type stream tcp
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
	{
		printf("Error creating socket : %d" , WSAGetLastError());
	}

	printf("Socket created.\n");
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6666 );
	
	//Bind
	if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind error : %d" , WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	
	puts("Bound");

	listen(s , 3);
	
	puts("Waiting for connections...");
	
	c = sizeof(struct sockaddr_in);
	
	while( (new_socket = accept(s , (struct sockaddr *)&client, &c)) != INVALID_SOCKET )
	{
		puts("Connection accepted");
        //recv(new_socket, message, strlen(message), 0);
        //puts(message);
		//Reply to the client
		write_file(new_socket);
		//message = "Hello socket\n";
		//send(new_socket , message , strlen(message) , 0);
	}

	
	
	if (new_socket == INVALID_SOCKET)
	{
		printf("accept failed with error code : %d" , WSAGetLastError());
		return 1;
	}

	

	closesocket(s);
	WSACleanup();
	
	return 0;
}