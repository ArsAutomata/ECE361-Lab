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
#include <errno.h>
#include <poll.h>

#define MAXLINE 1250
#define NUMTOTALCLIENTS 5

#include "link_list_impl.h"
#include "message.h"

struct client_node* head_cli = NULL;
struct session_node* head_sess = NULL;

struct m_data{
    unsigned int type;
	unsigned int size;
	char* client_id;
	char* client_data;
};


void on_join(struct m_data msg, int fd);
void on_new_sess(struct m_data msg, int fd);
void on_login(struct m_data msg, int fd, struct sockaddr* cli_addr);
void on_message(struct m_data msg, int fd);
void on_query(char *ID, int fd);
void on_leave_sess(char *ID);
void on_logout(char *ID);

//parse the packet string
Message parsemsg(char * msg){
    Message pkt;
    int num_colons =0;
    char type[12];
    char size[60];
    char source[MAX_NAME];
    char data[MAX_DATA];
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

    if (argc != 1) {
        fprintf(stderr,"No arguments needed\n");
        exit(1);
    }

    FILE *fp;
    char path[1000];
    int port;

    /* Open the command for reading. */
    fp = popen("/bin/netstat -lnt", "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }

    /* Read the output and get the first line that starts with tcp for IPv4 */
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(!strncmp(path, "tcp ", 4)){
            char *localhost = "127.0.0.1:";
            char *start = strstr(path, "127.0.0.1:");
            if(start == NULL){
                continue;
            }
            
            char *end = strstr(start," ");
            
            int port_numlen = end - start-strlen(localhost);
            char *port_str = malloc(port_numlen);
            strncpy(port_str, start+strlen(localhost), port_numlen);
            port = atoi(port_str);
            break;
        }
    }

    /* close */
    pclose(fp);

    printf("Using port number %d", port);
       
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
       
    int len = sizeof(cliaddr);

    // Constantly listen on this socket
    listen(sockfd, 100);
    Message msg;
    struct m_data m_msg;
    int new_fd;
    struct pollfd pfds[1];
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    
    while(1){

        // Get the new connection and new fd!
        new_fd = accept(sockfd, ( struct sockaddr *) &cliaddr,
            &len);
        if (new_fd < 0){
            perror("accept failed, waiting for a connection");
            if((errno == ENETDOWN || errno == EPROTO || errno == ENOPROTOOPT || errno == EHOSTDOWN ||
            errno == ENONET || errno == EHOSTUNREACH || errno == EOPNOTSUPP || errno == ENETUNREACH || errno == EAGAIN)) {
            poll(pfds,1,-1);
            continue;
         }
            exit(1);
        }

        recv(new_fd, (char *)buffer, MAXLINE,0);

        // Process the packet
        msg = parsemsg(buffer); 
        m_msg.type = msg.type;
        m_msg.size = msg.size;
        // Turn the char arrays into char pointers to easily compare to NULL and such 
        strcpy(m_msg.client_data, msg.data);
        strcpy(m_msg.client_id, msg.source);

        switch(msg.type) {

            case LOGIN: 
                on_login(m_msg, new_fd, (struct sockaddr *) &cliaddr);
                break;

            case JOIN:
                on_join(m_msg, new_fd);

            case EXIT:
                on_logout(m_msg.client_id);
                break;

            case LEAVE_SESS:
                on_leave_sess(m_msg.client_id);
                break;
            
            case NEW_SESS:
                on_new_sess(m_msg, new_fd);
                break;

            case MESSAGE:
                on_message(m_msg, new_fd);
                break;
            
            case QUERY:
                on_query(m_msg.client_id, new_fd);
                break;

            default:
                printf("Shouldnt be here");
                exit(1);
        }
    }
    
    // Close the socket
    close(sockfd);
    return 0;
}

void on_login(struct m_data msg, int fd, struct sockaddr* cli_addr){
    // Check if ID exists
                int ID_exist = 0;
                for (int i =0; i < NUMTOTALCLIENTS; i++){
                    if (strcmp(ID_arr[i], msg.client_id) == 0){
                        ID_exist = i+1;

                        break;
                    }
                }
                if(ID_exist == 0){

                    // Send NACK
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
                        LO_NAK, 
                        25,
                        msg.client_id, 
                        "Client is not registered"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    return;
                }

                // Check if matches pw
                if (strcmp(pw_arr[ID_exist-1], msg.client_data) != 0){
                    // Send NACK    
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
                        LO_NAK, 
                        25,
                        msg.client_id, 
                        "Client is not registered"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    return;
                }

                // Check if already logged in
                struct client_node* client = find_cli(msg.client_id, head_cli);

                if(client != NULL){
                    // Send NACK
                    char pre_pkt_string[200];
                    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
                        LO_NAK, 
                        28,
                        msg.client_id, 
                        "Client is already logged in"
                    );
                    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
                    close(fd);
                    return;
                }

                // Add to connected clients
                insert_cli(msg.client_id, NULL, cli_addr, head_cli);

                // Send back Lo_ACK
                char pre_pkt_string[200];
                sprintf(pre_pkt_string, "%d:%d:%s:%s", 
                    LO_ACK, 
                    0, 
                    msg.client_id, 
                    ""
                );
                send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
}

void on_join(struct m_data msg, int fd){
    
    // Check if session exists
    struct session_node* client_session = find_sess(msg.client_data, head_sess);
    if(client_session == NULL){
        // Session does not exist
        // Send JN_NAK
        char pre_pkt_string[200];
        char *data = "%s:Session does not exist";
        sprintf(pre_pkt_string, "%d:%d:%s:%s", 
            JN_NAK, 
            strlen(data), 
            msg.client_id, 
            data
        );
        send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
        return;
    }
    
    // Check if have already joined a session

    struct client_node* client = find_cli(msg.client_id, head_cli);
    if(client->session_ID != NULL) {
        // Already joined a session
        // Send JN_NAK
        char pre_pkt_string[200];
        char *data = "%s:You have already joined a session";
        sprintf(pre_pkt_string, "%d:%d:%s:%s", 
            JN_NAK, 
            strlen(data), 
            msg.client_id, 
            data
        );
        send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
        return;
    }

    // Add client to conference session 
    insert_cli(msg.client_id, client_session->ID, NULL,client_session->head_c);
    
    // Send JN_ACK
    char pre_pkt_string[200];
    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
        JN_ACK, 
        sizeof(msg.client_data), 
        msg.client_id, 
        msg.client_data
    );
    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
}

void on_new_sess(struct m_data msg, int fd){

    // Check if already joined a session 
    struct client_node* client = find_cli(msg.client_id, head_cli);
    if(client->session_ID != NULL) {
        // Already joined a session
        // Send NS_NAK
        char pre_pkt_string[200];
        char *data = "%s:Please leave your current session to create a new one";
        sprintf(pre_pkt_string, "%d:%d:%s:%s", 
            NS_NAK, 
            strlen(data), 
            msg.client_id, 
            data
        );
        send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
        return;
    }

    // Check if session already exists
    struct session_node* client_session = find_sess(msg.client_data, head_sess);
    if(client_session != NULL){
        // Session already exists
        // Send NS_NAK
        char pre_pkt_string[200];
        char *data = "%s:The session already exists";
        sprintf(pre_pkt_string, "%d:%d:%s:%s", 
            NS_NAK, 
            strlen(data), 
            msg.client_id, 
            data
        );
        send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
        return;
        
    }


    // Create new session 
    struct session_node* current = insert_sess(msg.client_data, head_sess);

    // Join the session 
    insert_cli(msg.client_id, msg.client_data, NULL, current->head_c);
    
    // Send JN_ACK
    char pre_pkt_string[200];
    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
        JN_ACK, 
        sizeof(msg.client_data), 
        msg.client_id, 
        msg.client_data
    );
    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
}

void on_message(struct m_data msg, int fd){

    // Get the client's session id
    struct client_node* client = find_cli(msg.client_id, head_cli);
    struct session_node* client_session = find_sess(client->session_ID, head_sess);
    
    client = client_session->head_c;

    // For each client in the session
    while(client){

        // Creating socket file descriptor
        int temp_fd;
        if ( (temp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        // Connect the socket
        connect(temp_fd, client->cli_addr,sizeof(client->cli_addr));

        // Send the message packet
        char pre_pkt_string[200];
        sprintf(pre_pkt_string, "%d:%d:%s:%s", 
            MESSAGE, 
            sizeof(msg.client_data), 
            msg.client_id, 
            msg.client_data
        );
        send(temp_fd, pre_pkt_string, sizeof(pre_pkt_string), 0);

        // Close the temporary file descriptor
        close(temp_fd);   

        // Next client
        client = client->next;
    }  
}

void on_query(char *ID, int fd){
    // john and peter are logged in but not connected to a session
    // jack and jill are part of the session called "lab_help"
    // josh is part of a session called  "class_help"
    // Example:
    // "john:peter:session:lab_help:jack:jill:class_help:josh"

    char pre_pkt_string[1000];
    char data[500];

    // Get the names of those not connected to a session first
    struct client_node* head = head_cli;
    while(head){
        if(head->session_ID == NULL){
            strcat(data, ":");
            strcat(data, head->ID);
        }
        head = head->next;
    }

    // Loop through all the clients in each session 
    struct session_node* head_s = head_sess;
    while(head_s){
        strcat(data, ":session:");
        strcat(data, head_s->ID);
        struct client_node* head_c = head_s->head_c;
        while(head_c){
            strcat(data, ":");
            strcat(data, head_c->ID);
            head_c = head_c->next;
        }
        head_s = head_s->next;
    }

    sprintf(pre_pkt_string, "%d:%d:%s:%s", 
        QU_ACK, 
        strlen(data), 
        ID, 
        data
    );
    send(fd, pre_pkt_string, sizeof(pre_pkt_string), 0);
    return;

}

void on_leave_sess(char *ID){
    // If the client is in a session, remove them
    struct client_node* client = find_cli(ID, head_cli);
    struct session_node* client_session = find_sess(client->session_ID, head_sess);
    if(client_session != NULL){
        struct client_node* client= delete_cli(ID, client_session->head_c);
        free(client);
    }

    // Check if conference session empty
    if (client_session == NULL){
        // Remove from the session list
        delete_sess(client_session->ID, head_sess);
        free(client_session);
    }
}

void on_logout(char *ID){

    // Remove the client from the connected list
    struct client_node* client = delete_cli(ID, head_cli);
    struct session_node* client_session = find_sess(client->session_ID, head_sess);
    free(client);

    // If the client is in a session, remove them
    if(client_session != NULL){
        struct client_node* client= delete_cli(ID, client_session->head_c);
        free(client);
    }

    // Check if conference session empty
    if (client_session == NULL){
        // Remove from the session list
        delete_sess(client_session->ID, head_sess);
        free(client_session);
    }
}
