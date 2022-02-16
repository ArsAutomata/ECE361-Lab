#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
   
#define MAXLINE 1250

struct packet {  
    unsigned int total_frag;  
    unsigned int frag_no; 
    unsigned int size; 
    char* filename; 
    char filedata[1000];  
}; 

//parse the packet string
struct packet parsepacket(char * filebuffer){
    struct packet pkt;
    int num_colons =0;
    char total_frag[50];
    char frag_no[50];
    char size[50];
    char filename[50];
    int start_of_data = 0;
    int start = 0;
    
    for(int i =0; i < 1250; i++){
        if(filebuffer[i] == ':') {
            if(num_colons == 0) {
            total_frag[i] = '\0';
            }else if(num_colons == 1) {
                frag_no[i-start] = '\0';
            }else if(num_colons == 2) {
                size[i-start] = '\0';
            }else if(num_colons == 3) {
                filename[i-start] = '\0';
            }
            start = i+1;
            num_colons++;
            continue;
        
        }
        if(num_colons == 0) {
            total_frag[i] = filebuffer[i];
            
        }else if(num_colons == 1) {
            frag_no[i-start] = filebuffer[i];
        }else if(num_colons == 2) {
            size[i-start] = filebuffer[i];
            
        }else if(num_colons == 3) {
            filename[i-start] = filebuffer[i];
        }else if(num_colons == 4) {
            start_of_data = i;
            break;
            
        }
    }
   
    pkt.total_frag = atoi(total_frag); 
    pkt.frag_no = atoi(frag_no); 
    sscanf(size, "%d", &(pkt.size));
    pkt.filename = filename;
    int i = 0;
    while(i < pkt.size){
        pkt.filedata[i] = filebuffer[i + start_of_data];
        i++;
    }
    return pkt; 
}

//write into given file
void writepacket(char * filebuffer, char * filename){
    FILE *fp; 
   
    if(fp == NULL)
    {
      printf("Error in file name");   
      exit(1);
    }
    fp = fopen(filename, "w"); 
    if(fp){
        fwrite(filebuffer, sizeof(filebuffer), 1, fp); 
        fclose(fp);
    }else{
        printf("Could not open file"); 
        exit(1); 
        fclose(fp);
    }
}

//clear the buffer 
void clearBuf(char* b)
{
    int i;
    for (i = 0; i < strlen(b); i++)
        b[i] = '\0';
}

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
    servaddr.sin_family = AF_INET; // IPv4
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
    
    struct packet pkt;

    FILE *fp; 
    
    // Receive first packet
        num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        // Check if something was received
        if(num_bytes == -1){
            printf("Recvfrom failed!");
            //send no ack
            char *NACK = "NACK";
            sendto(sockfd, (const char *)NACK, strlen("NACK"), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);
           
            //notify and clear buffer
            printf("NACK\n"); 
            clearBuf(buffer);
            exit(1);
        }else{
            //send ack
            char *ACK = "ACK";
            sendto(sockfd, (const char *)ACK, strlen(ACK), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);
            
            
            //process the packet
            pkt = parsepacket(buffer); 
            
            
            fp = fopen(pkt.filename, "w"); 
            if (!fp){
                fprintf(stderr,"Failed to create file");
                exit(1);
            }
            
            
            fwrite(pkt.filedata, 1, pkt.size, fp); 
            fprintf(stderr,"\n");
            clearBuf(buffer); 
            printf("\npacket 1 delivered, %d packets remaining", (pkt.total_frag-1));
        }

    //process packets
    for(int k = 1; k< pkt.total_frag; k++){
        if (k >= 244){
            fprintf(stderr, "%d %d", k, pkt.total_frag);
        }
        num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                &len);
        // Check if something was received
        if(num_bytes == -1){
            printf("Recvfrom failed!");
            //send no ack
            char *NACK = "NACK";
            sendto(sockfd, (const char *)NACK, strlen("NACK"), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);
           
            //notify and clear buffer
            printf("NACK\n"); 
            clearBuf(buffer);
            exit(1);
        }else{
            //send ack
            char *ACK = "ACK";
            sendto(sockfd, (const char *)ACK, strlen(ACK), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);
            fprintf(stderr, "\nSending ACK");
            
            
            //process the packet
            pkt = parsepacket(buffer); 
            fwrite(pkt.filedata, 1, pkt.size, fp); 
            clearBuf(buffer); 
            
            printf("\npacket %d delivered, %d packets remaining", pkt.frag_no, pkt.total_frag-pkt.frag_no);
        }
      
    }
    fclose(fp);
    fprintf(stderr,"Closing file\n");
    
    //close the socket
    close(sockfd);
    return 0;
}
