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
#include "message.h"
#define COMMAND_LEN 100
#define BUFFFER_SIZE 1200
bool logged_in;
bool in_session;

// socket
int sockfd;
int num_bytes;
socklen_t servaddr_len;
struct sockaddr_in servaddr;

// buffer
char buffer[BUFFFER_SIZE];

bool send_buffer()
{

	if ((num_bytes = send(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		return true;
	}
	else
	{
		printf("Could not send.\n");
		return false;
	}
}

void login(char *client_id, char *password, char *server_ip, char *server_port)
{
	// check for possible errors
	if (client_id == NULL || password == NULL || server_ip == NULL || server_port == NULL)
	{
		printf("incorrect usage of login");
		return;
	}

	struct sockaddr_in servaddr, cliaddr;

	int port = atoi(server_port);

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr(server_ip);
	servaddr.sin_port = htons(port);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&servaddr,
			 sizeof(servaddr)) < 0)
	{
		printf("bind failed");
		return;
	}

	Message login_mes;
	login_mes.type = LOGIN;
	strcpy(login_mes.source, client_id);
	strcpy(login_mes.data, password);
	login_mes.size = strlen(password);

	strcpy(buffer, serialize(login_mes));
	if(send_buffer()==true){
		printf("client sent");
	}else{
		printf("could not send login info\n");
		return; 
	}

	if ((num_bytes = recv(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		printf("failed recieve");
		close(sockfd);
		return;
	}
	

	Message *response = deserialize(buffer);
	if (response->type == LO_ACK)
	{
		printf("logged in\n");
		logged_in = true; 
		return;
	}
	else if (response->type == LO_NAK)
	{
		printf("wrong login information\n");
		close(sockfd);
		return;
	}else{
		printf("very big wrong ahhh!");

	}
}

void logout()
{
	Message logout_mes;
	logout_mes.type = EXIT;
	logout_mes.size = MAX_NAME;

	char *mes_string = serialize(logout_mes);
	printf("%s", mes_string);

	strcpy(mes_string, buffer);

	// Check if something was received
	if (num_bytes == -1)
	{
		printf("Recvfrom failed!");
		exit(1);
	}
	else
	{
		close(sockfd);
		logged_in = false;
	}
}

void createsession(char *session_id)
{

	printf("creating session: %s \n", session_id);

	Message create_mes;
	create_mes.type = NEW_SESS;
	strcpy(create_mes.data, session_id);
	create_mes.size = strlen(session_id);

	char *create_string = serialize(create_mes);
	strcpy(create_string, buffer);

	send_buffer();
	return;
}

void joinsession(char *session_id)
{
	if (in_session)
	{
		printf("You are in a session.\n");
		return;
	}

	printf("joining: %s\n", session_id);

	Message join_mes;
	join_mes.type = JOIN;
	strcpy(join_mes.data, session_id);
	join_mes.size = strlen(session_id);

	char *join_string = serialize(join_mes);
	int num_bytes;
	strcpy(join_string, buffer);

	if (send_buffer() == true)
	{
		in_session = true;
		return;
	}
	return;
}

void leavesession()
{
	if (!in_session)
	{
		printf("You are not in a session.\n");
		return;
	}

	printf("leaving session\n");

	Message leave_mes;
	leave_mes.type = LEAVE_SESS;
	leave_mes.size = 0;

	char *leave_string = serialize(leave_mes);
	int num_bytes;
	strcpy(leave_string, buffer);

	if (send_buffer() == true)
	{
		in_session = false;
		return;
	}

	return;
}

void list()
{
	if (!in_session)
	{
		printf("You are not in a session.\n");
		return;
	}

	printf("printing list:\n");

	Message list_mes;
	list_mes.type = QUERY;
	list_mes.size = 0;

	char *list_string = serialize(list_mes);
	int num_bytes;
	strcpy(list_string, buffer);

	send_buffer();
	return;
}

void send_text(char *text)
{
	int numbytes;
	Message text_mes;
	text_mes.type = MESSAGE;

	strcpy(text_mes.data, text);
	text_mes.size = strlen(text);
	char *text_string = serialize(text_mes);

	strcpy(text_string, buffer);

	send_buffer();
	return;
}

int main()
{
	// input strings
	char cmd[COMMAND_LEN];
	char client_id[COMMAND_LEN];
	char session_id[COMMAND_LEN];
	char password[COMMAND_LEN];
	char server_ip[COMMAND_LEN];
	char server_port[COMMAND_LEN];

	// socket
	int sockfd;
	int num_bytes;
	socklen_t servaddr_len;
	struct sockaddr_in servaddr;

	// status
	bool logged_in = false;

	// login and make connection
	while (!logged_in)
	{
		scanf("%s", cmd);
		if (strcmp(cmd, "/login") == 0)
		{
			scanf(" %s %s %s %s", client_id, password, server_ip, server_port);

			login(client_id, password, server_ip, server_port);
		}
		else if (strcmp(cmd, "/quit") == 0)
		{
			return 0;
		}
		else
		{
			printf("please login first\n");
		}
	}

	// full access to menu
	while (logged_in)
	{
		while (!in_session)
		{
			scanf("%s", cmd);

			if (strcmp(cmd, "/login") == 0)
			{
				printf("already logged in");
			}
			else if (strcmp(cmd, "/logout") == 0)
			{
				logout();
			}
			else if (strcmp(cmd, "/joinsession") == 0)
			{
				scanf(" %s", session_id);
				joinsession(session_id);
			}
			else if (strcmp(cmd, "/leavesession") == 0)
			{
				printf("invalid at this time\n");
			}
			else if (strcmp(cmd, "/createsession") == 0)
			{
				scanf(" %s", session_id);
				createsession(session_id);
			}
			else if (strcmp(cmd, "/list") == 0)
			{
				list();
			}
			else if (strcmp(cmd, "/quit") == 0)
			{
				return 0;
			}
			else
			{
				printf("invalid command while not in a session\n");
			}
		}

		while (in_session)
		{
			scanf("%s", cmd);

			if (strcmp(cmd, "/login") == 0)
			{
				printf("already logged in\n");
			}
			else if (strcmp(cmd, "/logout") == 0)
			{
				logout();
			}
			else if (strcmp(cmd, "/joinsession") == 0)
			{
				printf("already in a session\n");
			}
			else if (strcmp(cmd, "/leavesession") == 0)
			{
				leavesession();
			}
			else if (strcmp(cmd, "/createsession") == 0)
			{
				scanf(" %s", session_id);
				createsession(session_id);
			}
			else if (strcmp(cmd, "/list") == 0)
			{
				printf("list");
				list();
			}
			else if (strcmp(cmd, "/quit") == 0)
			{
				return 0;
			}
			else
			{
				char total_text[BUFFFER_SIZE];
				char partial_text[800];
				scanf("%[^\n]s", partial_text);

				strcat(total_text, cmd);
				strcat(total_text, partial_text);

				send_text(total_text);
			}
		}
	}

	return 0;
}
