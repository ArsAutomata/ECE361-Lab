#ifndef LINK_LIST_IMPL_H
#define LINK_LIST_IMPL_H



// Nodes for logged_in list
struct client_node {
    char* ID;
    char* session_ID;
    struct sockaddr* cli_addr;
    struct client_node *next;
    int fd;
    int active; 
};

// Nodes for conference session list
struct session_node {
    char* ID;
    char* admin; 
    struct client_node *head_c;
    struct session_node *next;
};

void insert_cli(char* ID, char* session_ID, struct sockaddr* cli_addr, struct client_node** head_c, int fd);
struct session_node* insert_sess(char* ID, struct session_node** head_sess);
struct client_node* find_cli(char *ID, struct client_node** head_c);
struct session_node* find_sess(char *ID, struct session_node** head_sess);
void delete_sess(char *ID, struct session_node** head_sess);
struct client_node* delete_cli(char *ID, struct client_node** head_c);

#endif
