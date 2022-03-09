#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAX_DATA 1000
#define MAX_NAME 100

#define COMMAND_LEN 100

typedef struct message
{
    unsigned int type;
    unsigned int size; 
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
    
}message;




int main() {
    
    //input strings
    char cmd [COMMAND_LEN];
    char client_id [COMMAND_LEN];
    char session_id [COMMAND_LEN];
    char password [COMMAND_LEN];
    char server_ip [COMMAND_LEN];
    char server_port [COMMAND_LEN];
    
    
    //socket 
    int sockfd;
    int num_bytes;
    socklen_t servaddr_len;
    struct sockaddr_in servaddr;
    
    //status 
	bool logged_in = false; 
	
	
    
    //login and make connection
PRELOG: 
	while(!logged_in){
		scanf("%s", cmd);
		if (strcmp(cmd, "/login") == 0){
            scanf(" %s %s %s %s", client_id, password, server_ip, server_port);
            
            //create socket and conenct to server
            char *server_address = server_ip;
            int port = atoi(server_port);
            
            // socket file descriptor for IPv4
            if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
                perror("socket creation failed");
                break; 
            }
            
            memset(&servaddr, 0, sizeof(servaddr));
            
            // Filling server information
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(port);
            servaddr.sin_addr.s_addr = inet_addr(server_address);
            
            
            
            //prepare the message
            message login_mes;
            login_mes.type = 1; 
            strcpy(login_mes.source, client_id);
            strcpy(login_mes.data, password);
            login_mes.size = strlen(login_mes.data);
            
            char mes_string[400];
            sprintf(mes_string, "%u:%u:%s:%s:", 
                login_mes.type, 
                login_mes.size, 
                login_mes.source, 
                login_mes.data
                );
				
			int mes_len = strlen(mes_string);
			
            //send the message
			num_bytes = sendto(sockfd, (const char *)mes_string, mes_len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            
			/ Check if something was received
            if(num_bytes == -1){
                printf("login failed!");
                exit(1);
            }else{
                logged_in = true; 
            }
		}else if (strcmp(cmd, "/quit") == 0) {
            return 0; 
		}else{
			printf("please login first\n");
		}
	}
	
	//full access to menu
    while(logged_in){
        scanf("%s", cmd); 
        
        //logout command
		if (strcmp(cmd, "/logout") == 0) {
		    message logout_mes;
            logout_mes.type = 4; 
            strcpy(logout_mes.source, client_id);
            logout_mes.size = strlen(logout_mes.data);
            
            char mes_string[200];
            sprintf(mes_string, "%u:%u:%s:%s:", 
                logout_mes.type, 
                logout_mes.size, 
                logout_mes.source, 
                logout_mes.data
                );
            
            int mes_len = strlen(mes_string);
            
            int num_bytes;
            num_bytes = sendto(sockfd, (const char *)mes_string, mes_len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
            
            // Check if something was received
            if(num_bytes == -1){
                printf("Recvfrom failed!");
                exit(1);
            }else{
                close(sockfd);
                logged_in = false; 
                goto PRELOG;
            }
		    
		} 
		//join session command
		else if (strcmp(cmd, "/joinsession") == 0) {
            scanf(" %s", session_id);
        } else if (strcmp(cmd, "/leavesession") == 0) {    
            printf("leavesession");
        } else if (strcmp(cmd, "/createsession") == 0) {
            scanf(" %s", session_id);
        } else if (strcmp(cmd, "/list") == 0) {
            printf("list");
        } else if (strcmp(cmd, "/quit") == 0) {
            return 0; 
        } else {
            printf("invalid command\n");
        }
        

    }
    
    
    return 0; 
}
