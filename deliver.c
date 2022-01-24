// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#define MAXLINE 1024
   
// Driver code
int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stderr,"usage: deliver <server address> <server port number>\n");
        exit(1);
    }
    char *server_address = argv[1];
    int port = atoi(argv[2]);
   
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_address);
       
    int n, len;
    printf("Please input the following command with the file you would like to transfer:\n(ftp <file name>)\n");
    char filename[50];
    scanf("%s", filename);
    if( access( filename, F_OK ) == 0 ) {
        char *ftp = "ftp";
        sendto(sockfd, (const char *)ftp, strlen(ftp),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
    } else {
        printf("huh %d\n", access( filename, F_OK ));
        exit(1);
    }
    
           
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';
    if (strcmp(buffer, "yes") == 0){
        printf("A file transfer can start.\n"); 
    }else{
        exit(1);
    }
   
    close(sockfd);
    return 0;
}