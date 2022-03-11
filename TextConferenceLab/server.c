#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h> 
   
#define MAXLINE 1250

// Linked list implementation



char ID_arr[5][50] = {
    "pete",
    "julia",
    "luigi",
    "mr.cow",
    "piss"
};
char pw_arr[5][20] = {
    "1234ttyu",
    "3456gv",
    "password3",
    "mr.password",
    "peeword"
};

struct client {
    char ID[50];
    char pw[20];
    char session_ID[50];
    struct sockaddr_in

}

struct message {  
    unsigned int type;  
    unsigned int size; 
    unsigned char source[50]; 
    unsigned char data[1000];  
}; 

struct Session_Info{
    struct client logged_in_list[5];
    char conf_session_id[5][50];
    struct client conf_sessions[5][5];
}


enum TYPES{
    LOGIN,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    NEW_SESS,
    NS_ACK,
    MESSAGE,
    QUERY,
    QU_ACK
};

//parse the packet string
struct packet parsemsg(char * msg){
    struct message pkt;
    int num_colons =0;
    char type[12];
    char size[60];
    char source[50];
    char data[1000];
    int start_of_data = 0;
    int start = 0;
    
    // Parse through the entire buffer
    for(int i =0; i < 1250; i++){

        // Change the starting offset each time a colon is reached
        // Append a null character as well to create a C string 
        if(msg[i] == ':') {
            if(num_colons == 0) {
                type[i] = '\0';
            }else if(num_colons == 1) {
                size[i-start] = '\0';
            }else if(num_colons == 2) {
                source[i-start] = '\0';
            }
            start = i+1;
            num_colons++;
            continue;
        
        }

        // Depending on how many colons have been passed, copy the byte data into the respective buffer
        if(num_colons == 0) {
            type[i] = msg[i];

        }else if(num_colons == 1) {
            size[i-start] = msg[i];

        }else if(num_colons == 2) {
            source[i-start] = msg[i];
            
        }else if(num_colons == 3) {
            // Break out of the for loop and store where the msg data starts in the buffer
            start_of_data = i;
            break;
            
        }
    }
   
    pkt.type = atoi(type); 
    pkt.size = atoi(size); 
    pkt.source = (char *)malloc(strlen(source) + 1);
    strcpy(pkt.source, source);

    // Copy the msg data 
    int i = 0;
    while(i < pkt.size){
        pkt.data[i] = msg[i + start_of_data];
        i++;
    }
    return pkt; 
}

//clear the buffer 
void clearBuf(char* b)
{
    int i;
    for (i = 0; i < strlen(b); i++)
        b[i] = '\0';
}

// Creates and binds a socket, then waits for the first packet to open a file and start writing data to it. 
// On last packet, write the data and close the file descriptor
int main(int argc, char *argv[]) {


    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    if (argc != 2) {
        fprintf(stderr,"usage: server <TCP listen port>\n");
        exit(1);
    }
    int port = atoi(argv[1]);

    //TODO: Check if port number is available
    FILE *fp;
    char path[2];

    char cmd_buffer[30];
    snprintf(cmd_buffer, sizeof(cmd_buffer), "/bin/nc -z 127.0.0.1 %d; echo $?", port);

    /* Open the command for reading. */
    fp = popen(cmd_buffer, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output a line at a time - output it. */
    fgets(path, sizeof(path), fp) != NULL);
    printf("%s", path);
    path[1] = '\n';
    int port_open = atoi(path);

    /* close */
    pclose(fp);

    if (port_open) {
        printf("Port not available. Please run \"netstat -lnt\" and check those ports.\n" );
        exit(1);
    }
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set to non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    

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
    char* spacketnum = malloc(10); 
    char ACKbuffer[120];  

    listen(sockfd, 100);
    // Receive first packet
    num_bytes = recvfrom(sockfd, (char *)buffer, MAXLINE, 
            MSG_WAITALL, ( struct sockaddr *) &cliaddr,
            &len);

    // Check if something was received
    if(num_bytes == -1){
        printf("Recvfrom failed!");

        // Send no ack
        char *NACK = "NACK";
        sendto(sockfd, (const char *)NACK, strlen("NACK"), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
            len);
           
        // Notify and clear buffer
        printf("NACK\n"); 
        clearBuf(buffer);
        exit(1);

    }else{    
            // Process the first packet
            pkt = parsemsg(buffer); 

            //TODO: switch case or some shit based on the pkt.type
            // send back the appropriate message
               
            // Create ACK 
            char *ACK = "ACK";
            if (snprintf(spacketnum, 10, "%d", pkt.frag_no) >= 10) {
                // truncation occured; Lost data because buffer was too small
                perror("snprintf");
            } else {

                // Concatenate the packet number with string "ACK"
                strcat(strcpy(ACKbuffer, ACK), spacketnum);
                free(spacketnum);
            }
            
            fprintf(stderr, "\nSending ACK %d", pkt.frag_no);

            // Send ACK
            sendto(sockfd, (const char *)ACKbuffer, strlen(ACKbuffer),
                   MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                   len);

            // Open filename now that first packet has been received
            fp = fopen(pkt.filename, "w"); 
            if (!fp){
                fprintf(stderr,"Failed to create file");
                exit(1);
            }
            
            // Write to the file 
            fwrite(pkt.filedata, 1, pkt.size, fp); 
            clearBuf(buffer); 
            fprintf(stderr, "\npacket 1 delivered, %d packets remaining", (pkt.total_frag-1));
        }

    // Process all remaining packets
    while(pkt.frag_no < pkt.total_frag){

        // Receive the packet
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

        }else if(rand()%100 == 67){
            //Drop the packet and do nothing
            fprintf(stderr, "\nPacket dropped!");
        }else{            
            // Process the packet
            pkt = parsemsg(buffer); 
           
           
            // Create ACK  
            char *ACK = "ACK";
            char *spacketnum = malloc(10);

            if (snprintf(spacketnum, 10, "%d", pkt.frag_no) >= 10){
                // truncation occured
                perror("snprintf");
            }else{
                strcat(strcpy(ACKbuffer, ACK), spacketnum);
                free(spacketnum);
            }

            // Send ACK
            sendto(sockfd, (const char *)ACKbuffer, strlen(ACKbuffer), 
                MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);  

            fprintf(stderr, "\nSending ACK %d", pkt.frag_no);
            clearBuf(ACKbuffer);
           
            // Write to file
            fwrite(pkt.filedata, 1, pkt.size, fp); 
            clearBuf(buffer); 
            
            fprintf(stderr,"\npacket %d delivered, %d packets remaining", pkt.frag_no, pkt.total_frag-pkt.frag_no);
        }
      
    }
    fclose(fp);
    fprintf(stderr,"\nClosing file\n");
    
    //close the socket
    close(sockfd);
    return 0;
}
