#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#define MAXLINE 1024

// Creates and binds a socket, then waits for message
int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAXLINE];
    
    struct sockaddr_in servaddr, cliaddr;

    if (argc != 2) {
        fprintf(stderr,"usage: server <UDP listen port>\n");
        exit(1);
    }
    int port = atoi(argv[1]);
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // get IP address of local machine
    char hostbuffer[256];
    gethostname(hostbuffer, sizeof(hostbuffer));
    char *IPbuffer;
    struct hostent *host_entry;
    host_entry = gethostbyname(hostbuffer);
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    if (IPbuffer == NULL){ 
        printf("Couldn't get IP of local machine"); 
        exit(1);
    }
    
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr(IPbuffer);
    servaddr.sin_port = htons(port);
       
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    int len, num_bytes;
    len = sizeof(cliaddr);  //len is value/result
   
    num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);

    // Check if something was received
    if(num_bytes == -1){
        printf("Recvfrom failed!");
        exit(1);
    }
    buffer[num_bytes] = '\0';
    printf("Client : %s\n", buffer);
    
    if (strcmp(buffer, "ftp") == 0){
        char *yes = "yes";
        sendto(sockfd, (const char *)yes, strlen(yes), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
        printf("yes message sent.\n"); 
    }else{
        char *no = "no";
        sendto(sockfd, (const char *)no, strlen("no"), 
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
        printf("no message sent.\n"); 
    }
    
    return 0;
}