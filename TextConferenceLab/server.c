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
#define NUMTOTALCLIENTS 5

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

struct session_info{
    struct client logged_in_list[5];
    char conf_session_id[5][50];
    struct client conf_sessions[5][5];
}


typedef enum TYPES{
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
struct message parsemsg(char * msg){
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
    
    struct message msg;
    FILE *fp; 
    char* spacketnum = malloc(10); 
    char ACKbuffer[120];  

    // Constantly listen on this socket
    listen(sockfd, 100);

    while(1){

        // Get the new connection and new fd!
        new_fd = accept(sockfd, ( struct sockaddr *) &cliaddr,
            &len);
        if (new_fd == -1){
            printf("accept failed");
            exit(1);
        }

        // Process the packet
        msg = parsemsg(buffer); 

        switch(msg.type) {

            case LOGIN: 
                on_login(msg, new_fd);
                break;

        }
    }

    
    // Close the socket
    close(sockfd);
    return 0;
}

void on_login(struct message msg, int fd){
    // Check if ID exists
                int ID_exist = 0;
                for (int i =0; i < NUMTOTALCLIENTS; i++){
                    if (strcmp(ID_arr[i], msg.source) == 0){
                        ID_exist = i+1;

                        break;
                    }
                }
                if(ID_exist == 0){

                    // Send NACK
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s:", 
                        LO_NAK, 
                        25,
                        msg.source, 
                        "Client is not registered"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    continue;
                }

                // Check if matches pw
                if (strcmp(pw_arr[ID_exist-1], msg.data) != 0){
                    // Send NACK    
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s:", 
                        LO_NAK, 
                        25,
                        msg.source, 
                        "Client is not registered"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    continue;
                }

                // Check if already logged in
                int logged_in = 0;
                int arrLen = sizeof(session_info.logged_in_list)) / sizeof(session_info.logged_in_list[0]);
                for (int i =0; i < arrLen; i++){
                    if (strcmp(session_info.logged_in_list[i].ID, msg.source) == 0){
                        logged_in = i+1;
                        break;
                    }
                }

                if(logged_in == 0){
                    // Send NACK
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s:", 
                        LO_NAK, 
                        28,
                        msg.source, 
                        "Client is already logged in"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    continue;
                }

                // create the client struct
                struct client c;
                c.ID = msg.source;
                c.pw = msg.data;
                c.session_ID = NULL;

                //TODO: Add to connected clients
                // Send back Lo_ACK
                char pre_pkt_string[200];
                sprintf(pre_pkt_string, "%d:%d:%s:%s:", 
                    LO_ACK, 
                    0, 
                    msg.source, 
                    ""
                );
                send(fd, NULL, 0, 0);
}
