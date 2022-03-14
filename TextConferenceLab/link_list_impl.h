#ifndef LINK_LIST_IMPL_H
#define LINK_LIST_IMPL_H

void insert_cli(char* ID, char* session_ID, struct client_node *head_c);
struct session_node* insert_sess(char* ID, struct session_node* head_sess);
struct client_node* find_cli(char *ID, struct client_node *head_c);
struct session_node* find_sess(char *ID, struct session_node* head_sess);
void delete_sess(char *ID, struct session_node* head_sess);
void delete_cli(char *ID, struct client_node *head_c);

#endif