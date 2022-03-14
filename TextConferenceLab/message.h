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
	MESSAGE,
	QUERY,
	QU_ACK
};

typedef struct message
{
	unsigned int type;
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
}
Message;

//reference 1 https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
//reference 2 https://stackoverflow.com/questions/3889992/how-does-strtok-split-the-string-into-tokens-in-c
Message* deserialize(char *string)
{
	Message *new_message = malloc(sizeof(struct message));

	char *typevar;
	char *size;
	char *source;
	char *data;

	//store
	typevar = strtok(string, ":");
	size = strtok(NULL, ":");
	source = strtok(NULL, ":");
	data = strtok(NULL, "");

	//exception handle
	if (typevar == NULL || size == NULL || source == NULL || data == NULL)
	{
		return NULL;
	}

	//store
	new_message->type = atoi(typevar);
	new_message->size = atoi(size);
	strcpy((char*) new_message->source, source);
	strcpy((char*) new_message->data, data);

	return new_message;
}


// reference 3 https://stackoverflow.com/questions/48422573/writing-to-file-if-sprintf-buffer-is-overflow
char *serialize(Message message)
{
	char *string = malloc((MAX_NAME+MAX_DATA+20) *sizeof(char));

	sprintf(string, "%d:%d:%s:%s", message.type, message.size, message.source, message.data);

	return string;
}

void print_message(Message message)
{
	printf("Type: %d\n", message.type);
	printf("Size: %d\n", message.size);
	printf("Source: %s\n", message.source);
	printf("Data: %s\n", message.data);
}
