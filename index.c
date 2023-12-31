#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define root "/ "
#define root_location "index.html"
#define api "/api"

#define BUFFER_SIZE 8192
#define RESPONSE_SIZE 8192

void *handle_client(void *arg) {
    int clientfd = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE] = {0}; //keep a null terminator
    ssize_t bytesRead;
    int headerEnd = 0;

    while ((bytesRead = read(clientfd, buffer + headerEnd, sizeof(buffer) - 1 - headerEnd)) > 0) {
        buffer[headerEnd + bytesRead] = '\0';

        char *headerEndPtr = strstr(buffer, "\r\n\r\n");
        if (headerEndPtr != NULL) {
            //calculate the position of the end of headers
            headerEnd = headerEndPtr - buffer + 4;
            break;
        }
    }

    // "GET /" 4 characters long (before the slash)
    char *file_request = buffer + 4;
    char response[RESPONSE_SIZE] = {0};
    char *metadata = "HTTP/1.3 200 OK\r\nContent-Type: text/html\r\n\r\n";

    memcpy(response, metadata, strlen(metadata));

    if (strncmp(file_request, root, strlen(root)) == 0) {
        FILE *f = fopen(root_location, "r");
        if (f != NULL) {
            fread(response + strlen(metadata), sizeof(response) - strlen(metadata) - 1, 1, f);
            fclose(f);
        } else {
            char *error = "Error opening file";
            memcpy(response + strlen(metadata), error, strlen(error));
        }
	} else if (strncmp(file_request, api, strlen(api)) == 0) {
        FILE *f = popen("./ml.o", "r");
        if (f != NULL) {
            fread(response + strlen(metadata), sizeof(response) - strlen(metadata) - 1, 1, f);
            pclose(f);
        } else {
            char *error = "Error executing program";
            memcpy(response + strlen(metadata), error, strlen(error));
        }
    } else {
        char *error = "No page found";
        memcpy(response + strlen(metadata), error, strlen(error));
    }

    write(clientfd, response, strlen(response));
    printf("Replied to %s\n", file_request);

    // free(file_request);
    // free(metadata); not allocated to there is no point in freeing it

    close(clientfd);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(8080),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(sockfd, 10);

    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len);

    printf("Server is running on %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

    while (1) {
        int *clientfd = malloc(sizeof(int));
        *clientfd = accept(sockfd, NULL, NULL);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)clientfd);

        pthread_detach(thread);
    }

    close(sockfd);

    return 0;
}