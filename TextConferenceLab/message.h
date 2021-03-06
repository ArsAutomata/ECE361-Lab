#ifndef MESSAGE_H
#define MESSAGE_H

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
#define MAX_NAME 100
#define MAX_DATA 1000

enum TYPES
{
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
	NS_NAK,
	MESSAGE,
	QUERY,
	QU_ACK,
	ADM_KICK,
	ADM_TRAN,
	ADM_SHOW, 
	ADM_ACK,
	ADM_NAK,
	SERVER_CLOSED,
	REGISTER,
	RE_ACK,
	RE_NAK
};

typedef struct message
{
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
} Message;

// reference 1 https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
// reference 2 https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
Message *deserialize(char *string)
{
	Message *new_message = malloc(sizeof(Message));

	char *typevar;
	char *size;
	char *source;
	char *data;
	// store
	typevar = strtok(string, ":");
	size = strtok(NULL, ":");
	source = strtok(NULL, ":");
	data = strtok(NULL, ":");
	// exception handle
	if (typevar == NULL) {
		return NULL;}
	else if(size == NULL) {
		return NULL;}
	else if(source == NULL){
		return NULL;}
	else if(data == NULL)
	{
		return NULL;
	}

	// store
	new_message->type = atoi(typevar);
	new_message->size = atoi(size);
	strcpy((char *)new_message->source, source);
	strcpy((char *)new_message->data, data);

	return new_message;
}

// reference 3 https://stackoverflow.com/questions/48422573/writing-to-file-if-sprintf-buffer-is-overflow
char *serialize(Message msg)
{
	char *string = malloc((MAX_NAME + MAX_DATA + 20) * sizeof(char));
	
	sprintf(string, "%d:%d:%s:%s", msg.type, msg.size, msg.source, msg.data);
	
	

	return string;
}

void print_message(Message msg)
{
	printf("Type: %d\n", msg.type);
	printf("Size: %d\n", msg.size);
	printf("Source: %s\n", msg.source);
	printf("Data: %s\n", msg.data);
}

#endif
