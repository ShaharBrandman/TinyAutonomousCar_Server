/*
 * A simple minimalist HTTPS Server
 * responseble for getting and sending data to the TinyAutonoumous Car
 * It is compatible with all Unix operating systems
 * and using severable built in libraries like glibc, bind, socket and htons
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>

#include <stdio.h>
#include <string.h>

#define root "index.html"

int main() {
	//creating a tcp socket stream
	int sockfd = socket(
		AF_INET, //for ipv4
		SOCK_STREAM, //for tcp
		0
	);

	//creating an ipv4 address struct to run on port 443
	struct sockaddr_in addr = {
		AF_INET, //family type
		htons(443), //port number, using htons to convert the value between host and network byte order
		INADDR_ANY //for any address
	};

	//binding the socket with the address struct
	if (bind(
		sockfd,
		(struct sockaddr*) &addr,
		sizeof(addr)
	) == -1) {
		perror("Bind error");
		printf("Couldn't bind socket\n");
		return 0;
	}

	//listen on this socket with max 5 pending connections at once
	listen(sockfd, 5);

	struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len);

    // print the IP address and port number
    printf("Server is running on %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

	//defining the accept mechanism for the socket
	int clientfd = accept(sockfd, NULL, NULL);

	//initilizing SSL context
	SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
	
	//creating an SSL struct pointer using the ssl context
	SSL* ssl = SSL_new(ctx);

	//setting communication with the client using ssl
	SSL_set_fd(ssl, clientfd);
	
	//getting SSL certificate
    SSL_use_certificate_chain_file(ssl, "ssl/cert.pem");
	
	//getting SSL Private key
	SSL_use_PrivateKey_file(ssl, "ssl/key.pem", SSL_FILETYPE_PEM);
	
	//verifying the SSL connection and wait for 
    SSL_accept(ssl);

	char buffer[1024] = {0}; //keeping a null terminator
	SSL_read(ssl, buffer, 1023); //enable the option to read bytes from the SSL connection
	
	// GET /index.html
	char* get_file_request = buffer + 5; //get request from the client

	// POST /index.html
	char* post_file_request = buffer + 6; //post request from the client

	char response[1024] = {0}; //also keeping a null terminator

	//defining the reponse metadata
	char* metadata = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";

	//copying metadata to reponse string
	memcpy(response, metadata, strlen(metadata));

	//if the get request is for the root path
	if (strcmp(get_file_request, root) == 0) {
		FILE* f = fopen(root, "r");

		//reading root file
		fread(response + strlen(metadata), 1024 - strlen(metadata) - 1, 1, f);
		
		//closing fs session
		fclose(f);
	} else {
		char* error = "No page found";
		memcpy(response + strlen(metadata), error, strlen(error));
	}

	//replying with a response with SSL connection
	SSL_write(ssl, response, 1024);

	//shutting down ssl
	SSL_shutdown(ssl);

	free(ssl);
	free(ctx);

	//close the tcp socket
	close(sockfd);

	return 1;
}