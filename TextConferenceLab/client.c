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
    
    bool connected = false; 
    
    while(1){
        scanf("%s", cmd); 
    
        if (strcmp(cmd, "/login") == 0){
            printf("login");
            scanf(" %s %s %s %s", client_id, password, server_ip, server_port);

        } else if (strcmp(cmd, "/logout") == 0) {
        } else if (strcmp(cmd, "/joinsession") == 0) {
            scanf(" %s", session_id);
        } else if (strcmp(cmd, "/leavesession") == 0) {    
            printf("leavesession");
        } else if (strcmp(cmd, "/createsession") == 0) {
            scanf(" %s", session_id);
        } else if (strcmp(cmd, "/list") == 0) {
        
        } else if (strcmp(cmd, "/quit") == 0) {
        
        } else {
            
        }

    }
    
    
    return 0; 
}
