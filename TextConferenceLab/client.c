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
#include <sys/fcntl.h> 
#include <errno.h>
#include <poll.h>
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

// client source
char client_id[COMMAND_LEN];

void clear_buffer()
{
	// clear the buffer for next message
	for (int i = 0; i < strlen(buffer); i++)
	{
		buffer[i] = '\0';
	}
}

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

	clear_buffer();
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

	// Set to non-blocking
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = inet_addr(server_ip);
	servaddr.sin_port = htons(port);

	// connect on client side
	if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		logged_in = false;
		perror("could not connect to server");
		return;
	}

	Message login_mes;
	login_mes.type = LOGIN;
	strcpy(login_mes.source, client_id);
	strcpy(login_mes.data, password);
	login_mes.size = strlen(password);

	strcpy(buffer, serialize(login_mes));
	if (send_buffer() == true)
	{
		printf("client sent");
	}
	else
	{
		printf("could not send login info\n");
		return;
	}

	int num_bytes;
	if ((num_bytes = recv(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		printf("failed recieve");
		close(sockfd);
		return;
	}

	clear_buffer();
	Message *response = deserialize(buffer);

	if (response->type == LO_ACK)
	{
		printf("logged in\n");
		logged_in = true;
		return;
	}
	else if (response->type == LO_NAK)
	{
		printf("%s", response->data);
		close(sockfd);
		return;
	}
	else
	{
		printf("very big wrong ahhh!");
	}
}

void logout()
{
	Message logout_mes;
	logout_mes.type = EXIT;
	logout_mes.size = MAX_NAME;
	strcpy(logout_mes.size, client_id);
	strcpy(logout_mes.data, "");

	char *mes_string = serialize(logout_mes);
	printf("%s", mes_string);

	strcpy(buffer, mes_string);

	close(sockfd);
	logged_in = false;
}

void joinsession(char *session_id)
{
	printf("joining: %s\n", session_id);

	Message join_mes;
	join_mes.type = JOIN;
	strcpy(join_mes.data, session_id);
	join_mes.size = strlen(session_id);
	strcpy(join_mes.source, client_id);

	char *join_string = serialize(join_mes);
	int num_bytes;
	strcpy(join_string, buffer);

	send_buffer();

	// check the type of ACK (JN_ACK or JN_NAK) for join and handle appropriately
	if ((num_bytes = recv(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		printf("failed recieve");
		close(sockfd);
		return;
	}

	clear_buffer();
	Message *response = deserialize(buffer);

	if (response->type == JN_ACK)
	{
		printf("joined session: %s\n", session_id);
		logged_in = true;
		return;
	}
	else if (response->type == JN_NAK)
	{
		printf("%s", response->data);
		close(sockfd);
		return;
	}
}

void createsession(char *session_id)
{

	printf("creating session: %s \n", session_id);

	Message create_mes;
	create_mes.type = NEW_SESS;
	strcpy(create_mes.data, session_id);
	create_mes.size = strlen(session_id);
	strcpy(create_mes.source, client_id);

	char *create_string = serialize(create_mes);
	strcpy(buffer, create_string);

	send_buffer();

	// recieve data, print sucess on NS_ACK, print error data on NS_NAK
	int num_bytes;
	if ((num_bytes = recv(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		printf("failed recieve");
		close(sockfd);
		return;
	}

	Message *response = deserialize(buffer);
	if (response->type == NS_ACK)
	{
		printf("session %s created\n", session_id);
		in_session = true;
		return;
	}
	else if (response->type == NS_NAK)
	{
		printf("%s", response->data);
		in_session = false;
		return;
	}
	else
	{
		printf("very big wrong ahhh!");
	}

	// join the created session
	joinsession(session_id);

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
	strcpy(leave_mes.source, client_id);
	strcpy(leave_mes.data, " ");

	char *leave_string = serialize(leave_mes);
	int num_bytes;
	strcpy(buffer, leave_string);

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
	strcpy(list_mes.source, client_id);

	char *list_string = serialize(list_mes);
	strcpy(buffer, list_string);

	send_buffer();

	// TODO: call recv to get the QU_ACK
	//  print out the user list and their sessions
	if ((num_bytes = recv(sockfd, buffer, BUFFFER_SIZE - 1, 0)) == -1)
	{
		printf("failed recieve");
		close(sockfd);
		return;
	}

	clear_buffer();
	Message *response = deserialize(buffer);

	if (response->type == QU_ACK)
	{
		printf("list recieved\n");
		return;
	}
	else
	{
		printf("error in recieveing list");
		return;
	}
}

void send_text(char *text)
{
	int numbytes;
	Message text_mes;
	text_mes.type = MESSAGE;
	strcpy(text_mes.source, client_id);

	strcpy(text_mes.data, text);
	text_mes.size = strlen(text);
	char *text_string = serialize(text_mes);

	strcpy(buffer, text_string);

	send_buffer();
	return;
}

int main()
{
	// input strings
	char cmd[COMMAND_LEN];
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
			// TODO:
			//  Should be listening for packets on a non-blocking port as well somewhere here as long as you are in the session (it already is in connect)
			//  probably should open a thread for getting user input while listening for packets on this thread (checkout beej's on listen(), and accept(), adn then you can call recv() once these two are done)
			//  can use my code for reference on how to listen for stuff

			// Constantly listen on this socket
			listen(sockfd, 10);
			int new_sockfd = accept(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

			recv(new_sockfd, buffer, BUFFFER_SIZE - 1, 0);

			// process the buffer to see if it is text
			clear_buffer();
			Message *response = deserialize(buffer);
			if (response->type == MESSAGE)
			{
				printf("%s", response->data);
			}

			// the code abovve should theoretically work, but it dosn't, idk why

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
