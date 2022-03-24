// Linked list implementation
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "link_list_impl.h"


//insert link at the first location of logged in list
void insert_cli(char* ID, char* session_ID, struct sockaddr* cli_addr, struct client_node** head_c, int fd) {
   //create a link
   struct client_node *link = (struct client_node*) malloc(sizeof(struct client_node));

   
	link->ID = (char *) malloc(100);
   link->session_ID = (char *) malloc(100);
   strcpy(link->ID ,ID);
   if(session_ID != NULL){
      strcpy(link->session_ID ,session_ID);
   }
   else{ link->session_ID = NULL;}
   
   link->fd = fd;
   if(cli_addr != NULL){
      link->cli_addr = cli_addr;
   }

	
   //point it to old first node
   link->next = *head_c;
	
   //point first to new first node
   *head_c = link;

}

//insert link at the first location of conf session list
struct session_node* insert_sess(char* ID, struct session_node** head_sess) {
   //create a link
   struct session_node *link = (struct session_node*) malloc(sizeof(struct session_node));
	link->ID = (char *) malloc(100);
   strcpy(link->ID ,ID);
   link->head_c = NULL;
	
   //point it to old first node
   link->next = *head_sess;
	
   //point first to new first node
   *head_sess = link;
   return link;
}

//find a link with given key
struct client_node* find_cli(char *ID, struct client_node** head_c) {

   //start from the first link
   struct client_node* current = *head_c;

   //if list is empty
   if(*head_c == NULL) {
      return NULL;
   }
   //navigate through list
   while(strcmp(current->ID,ID) != 0) {
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}

//find a link with given key
struct session_node* find_sess(char *ID, struct session_node** head_sess) {

   //start from the first link
   struct session_node* current = *head_sess;

   //if list is empty
   if(*head_sess == NULL) {
      return NULL;
   }

   //navigate through list
   while(strcmp(current->ID,ID) != 0) {
	
      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //go to next link
         current = current->next;
      }
   }      
	
   //if data found, return the current Link
   return current;
}

//delete a link with given key
void delete_sess(char *ID, struct session_node** head_sess) {

   //start from the first link
   struct session_node* current = *head_sess;
   struct session_node* previous = NULL;
	
   // if list is empty
   if(current == NULL) {
      return;
   }

   //navigate through list
   while(strcmp(current->ID,ID) != 0) {

      //if it is last node
      if(current->next == NULL) {
         // return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == *head_sess) {
      //change first to point to next link
      *head_sess = (*head_sess)->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
}

//delete a link with given key
struct client_node* delete_cli(char *ID, struct client_node** head_c) {

   //start from the first link
   struct client_node* current = *head_c;
   struct client_node* previous = NULL;
	
   //if list is empty
   if(*head_c == NULL) {
      return NULL;
   }

   //navigate through list
   while(strcmp(current->ID,ID) != 0) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == *head_c) {
      //change first to point to next link
      *head_c = (*head_c)->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
   return current;
}
