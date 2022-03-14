// Linked list implementation

// Nodes for logged_in list
struct client_node {
    char ID[50];
    char session_ID[50];
    struct sockaddr* cli_addr;
    struct client_node *next;
};

// Nodes for conference session list
struct session_node {
    char ID[50];
    struct client_node *head_c;
    struct session_node *next;
};


//insert link at the first location of logged in list
void insert_cli(char* ID, char* session_ID, struct sockaddr* cli_addr, struct client_node *head_c) {
   //create a link
   struct client_node *link = (struct client_node*) malloc(sizeof(struct client_node));
	
   link->ID = ID;
   link->session_ID = session_ID;
   if(cli_addr != NULL){
      link->cli_addr = cli_addr;
   }
   
	
   //point it to old first node
   link->next = head_cli;
	
   //point first to new first node
   head_cli = link;
}

//insert link at the first location of conf session list
struct session_node* insert_sess(char* ID, struct session_node* head_sess) {
   //create a link
   struct session_node *link = (struct session_node*) malloc(sizeof(struct session_node));
	
   link->ID = ID;
   link->head_c = NULL;
	
   //point it to old first node
   link->next = head_sess;
	
   //point first to new first node
   head_sess = link;
   return head_sess;
}

//find a link with given key
struct client_node* find_cli(char *ID, struct client_node *head_c) {

   //start from the first link
   struct client_node* current = head_c;

   //if list is empty
   if(head_c == NULL) {
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
struct session_node* find_sess(char *ID, struct session_node* head_sess) {

   //start from the first link
   struct session_node* current = head_sess;

   //if list is empty
   if(head_sess == NULL) {
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
void delete_sess(char *ID, struct session_node* head_sess) {

   //start from the first link
   struct session_node* current = head_sess;
   struct session_node* previous = NULL;
	
   //if list is empty
   if(head_sess == NULL) {
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
   if(current == head_sess) {
      //change first to point to next link
      head_sess = head_sess->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
}

//delete a link with given key
struct client_node* delete_cli(char *ID, struct client_node *head_c) {

   //start from the first link
   struct client_node* current = head_c;
   struct client_node* previous = NULL;
	
   //if list is empty
   if(head_c == NULL) {
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
   if(current == head_c) {
      //change first to point to next link
      head_c = head_c->next;
      current->session_ID = NULL;
   } else {
      //bypass the current link
      previous->next = current->next;
      current->session_ID = NULL;
   }    
   return current;
}