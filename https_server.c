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
#include <openssl/err.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
	if (
		bind(
			sockfd,
			(struct sockaddr*) &addr,
			sizeof(addr)
		) == -1
	) {
		perror("Bind error");
		printf("Couldn't bind socket");
		return 1;
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

	SSL_library_init(); //initlize the ssl library

	//initilizing SSL context
	SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());

	if (ctx == NULL) {
    	perror("Error creating SSL context");
		close(sockfd);
	    return 1;
	}

	SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION); // Adjust the version based on your requirements
	SSL_CTX_set_cipher_list(ctx, "TLS_AES_128_GCM_SHA256"); // Adjust the cipher suite based on your requirements

	//creating an SSL struct pointer using the ssl context
	SSL* ssl = SSL_new(ctx);
	if (ssl == NULL) {
    	perror("Error creating SSL structure");
    	SSL_CTX_free(ctx);
		close(sockfd);
    	return 1;
	}

	//setting communication with the client using ssl
	SSL_set_fd(ssl, clientfd);
	
	//getting SSL certificate
    if (SSL_use_certificate_chain_file(ssl, "ssl/cert.pem") == -1) {
		perror("Error loading certificate");
		SSL_shutdown(ssl);
		SSL_CTX_free(ctx);
		close(sockfd);
		return 1;
	}
	
	//getting SSL Private key
	if (SSL_use_PrivateKey_file(ssl, "ssl/key.pem", SSL_FILETYPE_PEM) == -1) {
		perror("Error loading private key");
		SSL_shutdown(ssl);
		SSL_CTX_free(ctx);
		close(sockfd);
		return 1;
	}
	
	//verifying the SSL connection and wait for tcp handshake
    SSL_accept(ssl);
	int ssl_error = SSL_get_error(ssl, SSL_accept(ssl));
	if (ssl_error != SSL_ERROR_NONE) {
		fprintf(stderr, "SSL_accept error: %s (SSL_get_error: %d)\n", ERR_error_string(ssl_error, NULL), ssl_error);
		ERR_print_errors_fp(stderr);
		SSL_shutdown(ssl);
		SSL_CTX_free(ctx);
		close(sockfd);
		return 1;
	}

	fprintf(stderr, "SSL State: %s\n", SSL_state_string_long(ssl));

	char *buffer = malloc(8192);

	//enable the option to read bytes from the SSL connection
	int read_result = SSL_read(ssl, buffer, sizeof(buffer) - 1);
	
	//excpetion handling for read_result
	if (read_result <= 0) {
		int ssl_error = SSL_get_error(ssl, read_result);
		fprintf(stderr, "SSL_read error: %s (SSL_get_error: %d)\n", ERR_error_string(ssl_error, NULL), ssl_error);

		if (read_result == 0) {
			fprintf(stderr, "SSL_read: Connection closed by peer\n");
		} else {
			fprintf(stderr, "SSL_read: Error reading data\n");
		}

		printf("Couldn't read SSL\n");
	} else {
		buffer[read_result] = '\0';  // Null-terminate the received data
		printf("Received data from the client:\n%s\n", buffer);
	}

	// "GET /" 5 characters long
	char* get_file_request = buffer + 5; //get request from the client

	// "POST /" 6 characters long
	char* post_file_request = buffer + 6; //post request from the client

	char response[1024] = {0}; //also keeping a null terminator

	//defining the reponse metadata
	char* metadata = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

	//copying metadata to reponse string
	if(memcpy(response, metadata, strlen(metadata)) == NULL) {
		perror("Coudln't copy metadata to response");
		SSL_CTX_free(ctx);
		SSL_shutdown(ssl);
		close(sockfd);
		return 1;
	}

	char content_length[100];
	sprintf(content_length, "Content-Length: %zu\r\n", strlen(response) - strlen(metadata));
	strcat(metadata, content_length);

	//if the get request is for the root path
	if (strcmp(get_file_request, root) == 0) {
		FILE* f = fopen(root, "r");
		if (f == NULL) {
			perror("Error opening file");
			SSL_CTX_free(ctx);
			SSL_shutdown(ssl);
			close(sockfd);
    		return 1;
		}

		//reading root file
		fread(response + strlen(metadata), 1024 - strlen(metadata) - 1, 1, f);
		
		//closing fs session
		fclose(f);
	} else {
		perror("Coudln't find path\n");
		char* error = "No page found";
		if(memcpy(response + strlen(metadata), error, strlen(error)) == NULL) {
			perror("Coudn't copy error page response");
			SSL_CTX_free(ctx);
			SSL_shutdown(ssl);
			close(sockfd);
		}
	}

	//replying with a response with SSL connection
	SSL_write(ssl, response, strlen(response));

	//shutting down ssl
	SSL_shutdown(ssl);

	SSL_CTX_free(ctx);

	//free the ssl buffer
	free(buffer);

	//close the tcp socket
	close(sockfd);

	return 0;
}