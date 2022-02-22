#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <poll.h>
   
#define MAXLINE 1250
struct packet {  
    unsigned int total_frag;  
    unsigned int frag_no; 
    unsigned int size; 
    char* filename; 
    char filedata[1000];  
};

// For each packet, create it and store the bytes, and put the packet in an array 
void create_packet_array(int num_packets, long len, char * file_stream, char * filename, struct packet * p_array){
    for (int i=0; i < num_packets; i++){
        struct packet pkt;
        pkt.total_frag = num_packets;
        pkt.frag_no = i+1;
        pkt.filename = filename;

        // The size is 1000 bytes unless it is the last packet which will be the remaining amount
        if (i == num_packets - 1){
            // This is the last packet
            pkt.size = len % 1000;
        }else{
            pkt.size = 1000; 
        }
        for (int j = 0; j < pkt.size; j++){
            pkt.filedata[j] = file_stream[i*1000 + j];
        }
        p_array[i] = pkt;
    }

}

//clear the buffer 
void clearBuf(char* b)
{
    int i;
    for (i = 0; i < strlen(b); i++)
        b[i] = '\0';
}

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
    char *spacketnum;
    char ACKbuffer[100];

    // Check file existence and send message if exists / readable
    if( access( filename, R_OK ) == 0 ) {
        FILE *fileptr;
        char *buffer;
        long filelen;

        // Open the file in binary mode and jump to EOF
        // Get the byte offset which is now the length of whole file
        // Go back to beginning of file
        fileptr = fopen(filename, "rb");  
        fseek(fileptr, 0, SEEK_END);          
        filelen = ftell(fileptr);             
        rewind(fileptr);                      

        // Create a big enough buffer and read in all the bytes
        // Then close file
        buffer = (char *)malloc(filelen * sizeof(char));
        fread(buffer, sizeof(char), filelen, fileptr);
        fclose(fileptr);
        printf("File read\n");

        // Get the number of packets needed and create the packet array 
        int num_packets = filelen / 1000 + (filelen % 1000 == 0 ? 0 : 1);
        struct packet* packet_array = malloc(num_packets*sizeof(struct packet));
        create_packet_array(num_packets, filelen, buffer, filename, packet_array);
        
        char ACKbuffer[100];
        char* spacketnum = malloc(10);

        // RTT related variables
        double Est_rtt = 0.500;
        double Dev_rtt = 0.100;
        double timeout_interval = Est_rtt + 4 * Dev_rtt;
        double rtt;
        struct pollfd pfds[1];
        pdfs[0].fd = sockfd;
        pdfs[0].events = POLLIN;
        struct timeval t_start;
        struct timeval t_end;

        for(int i=0; i<num_packets; i++){
            int retransmitted = 0;
            // Create the packet string without the file data
            char pre_pkt_string[200];
            sprintf(pre_pkt_string, "%u:%u:%u:%s:", 
                packet_array[i].total_frag, 
                packet_array[i].frag_no, 
                packet_array[i].size, 
                packet_array[i].filename
                );
            
            // Allocate a string with enough space for all the data and its metadata
            int packet_len = strlen(pre_pkt_string) + packet_array[i].size;            
            char pkt_string[2000];
            strcpy(pkt_string, pre_pkt_string);

            // Copy the file data into the packet string
            for(int j =0; j< packet_array[i].size; j++){
                pkt_string[strlen(pre_pkt_string) + j] = packet_array[i].filedata[j];
            }

            // Always reset the timed_out value to false
            int timed_out = 0;
                        
            // Send a packet and retransmit if receiving the ACK timed_out
            do{
                // If it has timed out, print the retransmitting message and change the retransmitted var so that rtt values for this segment won't be used
                if (timed_out) {
                    printf("Retransmitting!"); 
                    retransmitted = 1;
                }
                
                // Send the packet and record the start time
                gettimeofday(&t_start, NULL);
                num_bytes = sendto(sockfd, (const char *)pkt_string, packet_len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

                // Poll to watch for data ready to be received on our socket file descriptor 
                int num_events = poll(pdfs, 1, timeout_interval);
                if(num_events == 0){ 
                    // Nothing happened; Never received ACK
                    timed_out = true;

                }else{
                    // Received ACK
                    timed_out = false;
                    num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                                     &servaddr_len);

                    // Record end time and calculate rtt
                    gettimeofday(&t_end, NULL);
                    rtt = (t_end.tv_sec - t_start.tv_sec)*1000 + (t_end.tv_usec - t_start.tv_usec)/1000.0;

                    // Check if something was received
                    if(num_bytes == -1){
                        printf("Recvfrom failed!");
                        exit(1);
                    }  
                }

            }while(timed_out == 0);
            // Recalculate the ACK_timer using double t
            if (!retransmitted){
                Est_rtt = 0.875*Est_rtt + 0.125*rtt;
                Dev_rtt = 0.75*Dev_rtt +0.25*abs(rtt-Est_rtt);
                timeout_interval = Est_rtt + 4*Dev_rtt;
            }

            

            // gettimeofday(&t_end, NULL);
            // double rtt = (t_end.tv_sec - t_start.tv_sec)*1000 + (t_end.tv_usec - t_start.tv_usec)/1000.0;
            fprintf(stderr, "RTT: %f\n", t);

            
            buffer[num_bytes] = '\0';
            
            //check if correct ACK recieved
            char* ACK = "ACK";
            if (snprintf(spacketnum, 10, "%d", packet_array[i].frag_no) >= 10) {
                // truncation occured in snprintf
                perror("snprintf");
            } else {
                strcat(strcpy(ACKbuffer, ACK), spacketnum);
            }
           
            if (strcmp(buffer, ACKbuffer) == 0){
                printf("\npacket %d successfully sent", packet_array[i].frag_no); 
            }else if (strcmp(buffer, "NACK") == 0){
                printf("packet %d was not delivered, exiting\n", packet_array[i].frag_no);
                exit(1);
            }else{
                fprintf(stderr, "yikes server error, did not send right message");
                exit(1); 
            }
            
            clearBuf(ACKbuffer);
            
        }

        
        
    } else {
        printf("The file does not exist or the pathname is incorrect");
        exit(1);
    }
    
    // Close the file descriptor
    close(sockfd);
    return 0;
}
