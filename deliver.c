#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#define MAXLINE 1024

// https://stackoverflow.com/questions/15707933/how-to-serialize-a-struct-in-c
// https://stackoverflow.com/questions/1577161/passing-a-structure-through-sockets-in-c

// TODO: Define the packet struct here

// Creates and binds a socket, then sends message and waits for response
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
   
    // socket file descriptor for IPv4
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(1);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_address);
       
    int num_bytes;
    socklen_t servaddr_len;
    
    printf("Please input the following command with the file you would like to transfer:\n(ftp <file name>)\n");
    char filename[50];

    // Call twice to get both words
    scanf("%s", filename);
    if (strcmp(filename, "ftp") != 0){
        printf("Usage: ftp <file name>");
        exit(1);
    }

    // Get filename
    scanf("%s", filename);

    // Check file existence and send message if exists / readable
    if( access( filename, R_OK ) == 0 ) {
        // TODO: 
        // read filestream into byte[]
        // https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array
        // 


        // declare packet_array[] of const void *;
        // packet_array = serialize_file(fileStream, filename) 
        // This method should take the stream and break it up into the required number of packets 
        // Should return an array of packets
        
        // send_packets(packet_array);
        // This method should {for each packet in array: sendto(); wait for ACK; deserialize_ACK() and ensure its good}

        char *ftp = "ftp";
        num_bytes = sendto(sockfd, (const char *)ftp, strlen(ftp),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));

        // Ensure if something was sent
        if(num_bytes == -1){
            printf("Sendto failed!");
            exit(1);
        }
    } else {
        printf("The file does not exist or the pathname is incorrect");
        exit(1);
    }
    
    // Get a message back
    num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &servaddr_len);
    
    // Check if something was received
    if(num_bytes == -1){
        printf("Recvfrom failed!");
        exit(1);
    }
    buffer[num_bytes] = '\0';
    if (strcmp(buffer, "yes") == 0){
        printf("A file transfer can start.\n"); 
    }else{
        exit(1);
    }
    
    // Close the file descriptor
    close(sockfd);
    return 0;
}