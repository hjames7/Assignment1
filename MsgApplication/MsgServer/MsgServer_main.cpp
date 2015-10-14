// Description:
//
//    This sample illustrates how to develop a simple echo server Winsock
//    application using the select() API I/O model. This sample is
//    implemented as a console-style application and simply prints
//    messages when connections are established and removed from the server.
//    The application listens for TCP connections on port 5150 and accepts
//    them as they arrive. When this application receives data from a client,
//    it simply echos (this is why we call it an echo server) the data back in
//    it's original form until the client closes the connection.
//
//    Note: There are no command line options for this sample.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>		// Windows equiv. to UNIX libraries
#include <windows.h>
#include <map>
#include <vector>
#include <ws2ipdef.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

#define PORT 5150
#define SERVPORT "5150"
#define	ADDR "127.0.0.1"
#define DATA_BUFSIZE 8192

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

typedef struct _SOCKET_INFORMATION {
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	OVERLAPPED Overlapped;
	DWORD BytesSEND;
	DWORD BytesRECV;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

// Map of rooms (should be 8 rooms of max capacity of 8)
std::map< std::string, std::vector< SOCKET > > chatRooms;

// Prototypes
BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);

// Global var
DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[FD_SETSIZE]; // Maximum size of connections on entire server

int main(int argc, char **argv)
{
	// To do WSAStartup()
	WSADATA wsaData;
    INT iResult;

	FD_SET master;		// master file descriptor list
	FD_SET read_fds;	// temp file descriptor list for select()

	SOCKET fdmax;

	SOCKET listener;		// listening socket descriptor
	SOCKET newfd;			// newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr;	// client address
	socklen_t addrlen;

	CHAR buf[DATA_BUFSIZE];		// buffer for client data
	INT nbytes;

	CHAR remoteIP[INET6_ADDRSTRLEN];

	//INT yes=1;			// for setsockopt() SO_REUSEADDR, below

	SOCKET i, j, rv;

	struct addrinfo hints, *ai, *p;
	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
    }

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	if ((rv = getaddrinfo(NULL, SERVPORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
	}

	for(p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, NULL, sizeof(int));

		if (bind (listener, p->ai_addr, p->ai_addrlen) < 0) {
			closesocket(listener);
			continue;
		}

		break;
	}

	// if we go here, it means we didn't get bound
	if (p == NULL ) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai);

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener;	// so far, it's this one
	printf("fdmax %d\n ", fdmax);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;	// 500 ms
	
	// main loop
	for(;;) {
		read_fds = master; // copy it
		// - Timeouts if no immediate activity -
		if (select(fdmax + 1, &read_fds, NULL, NULL, &tv) == -1) {
			perror("select");
			exit(4);
		}
		//printf("tick\n");

		// run through the existing connections looking for data to read
		for(i = 0; i <= fdmax; i++) {
			if (! FD_ISSET(i, &read_fds)) {
				continue;
			}

			if (i == listener) {
				// handle new connections
				addrlen = sizeof remoteaddr;
				newfd = accept(listener,
							(struct sockaddr *)&remoteaddr,
							&addrlen);

				printf ("newfd %d \n", newfd);
				if (newfd == -1) {
					perror("accept");
					continue;
				}

				FD_SET(newfd, &master);	// add to master set
				if (newfd > fdmax) { // keep track of the max
					fdmax = newfd;
				}

				printf("selectserver: new connection from %s on "
					"socket %d\n", 
					inet_ntop(remoteaddr.ss_family,
							  get_in_addr((struct sockaddr*)&remoteaddr),
							  remoteIP, INET6_ADDRSTRLEN), newfd);
				
				continue;
			}

			printf("try to recv\n");
			// handle data from a client
			if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
				printf("fail\n");
				// got error or connection closed by client
				if (nbytes == 0) {
					// connection closed
					printf("selectserver: socket %d hung up\n", i);
				} else {
					perror("recv");
				}
				closesocket(i);	// bye!
				FD_CLR(i, &master);	// remove from master set

				continue;
			}
			printf("get some data \n");

			// - INSERT COMMAND READING [oooooo~] -
			//if (!buf.compare(0, 5, "!join")) {
			//	// Extract the room name
			//	if (isInARoom) {
			//		printf("You can only be in one room at a time [currently].\n");
			//	} else {
			//		size_t pos = inputStr.find(" ");
			//		if (pos != std::string::npos) {
			//			size_t nextPos = inputStr.find(" ", pos + 1);
			//			if (nextPos != std::string::npos) {
			//				connectedRoom = inputStr.substr(pos, nextPos);
			//			}
			//		}
			//		if (connectedRoom != "") {
			//			sendbuf = "";
			//			strcpy(sendbuf, inputStr.c_str());
			//			iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
			//			if (iResult == SOCKET_ERROR) {
			//				printf("Send failed with error: %d\n", WSAGetLastError());
			//				WSACleanup();
			//				return 1;
			//			}
			//			printf("You have joined the room named [%d].\n", connectedRoom);
			//		}
			//	}
			//} else if (!inputStr.compare(0, inputStr.size(), "!leave")) {
			//	// Disconnect from the room if you're connected to one
			//	if (isInARoom) {
			//		sendbuf = "";
			//		strcpy(sendbuf, inputStr.c_str());
			//		iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
			//		if (iResult == SOCKET_ERROR) {
			//			printf("Send failed with error: %d\n", WSAGetLastError());
			//			WSACleanup();
			//			return 1;
			//		}
			//		printf("You have left the room named [%d].\n", connectedRoom);
			//		connectedRoom = "";
			//		isInARoom = false;
			//	} else { // Otherwise, tell the client they're silly
			//		printf("Silly; you're not in a room!");
			//	}

			//} else if (!inputStr.compare(0, 4, "!msg")) {
			//	if (isInARoom) {
			//		std::string messageStr;
			//		messageStr.append("!msg ");
			//		messageStr.append(username);
			//		messageStr.append(": ");
			//		size_t pos = inputStr.find(" ");
			//		if (pos == std::string::npos) {
			//			printf("Message error: Please try again!\n");
			//		} else {
			//			messageStr.append(inputStr.substr(pos, std::string::npos));
			//			sendbuf = "";
			//			strcpy(sendbuf, messageStr.c_str());
			//			iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
			//			if (iResult == SOCKET_ERROR) {
			//				printf("Send failed with error: %d\n", WSAGetLastError());
			//				WSACleanup();
			//				return 1;
			//			}
			//		}
			//	} else {
			//		printf("Please join a room before trying this...\n");
			//	}
			//} else if (!inputStr.compare(0, inputStr.size(), "!quit")) {
			//	isGoingToQuit = true;
			//	// Disconnect from the room if you're connected to one
			//	if (isInARoom) {
			//		sendbuf = "";
			//		strcpy(sendbuf, "!leave");
			//		iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
			//		if (iResult == SOCKET_ERROR) {
			//			printf("Send failed with error: %d\n", WSAGetLastError());
			//			WSACleanup();
			//			return 1;
			//		}
			//		printf("You have left the room named [%d].\n", connectedRoom);
			//		isInARoom = false;
			//	}
			//} else {
			//	printf("Command error! Reminder: Please enter "
			//		"'!help' to receive a list of commands.\n");
			//}

			// We go some data from a client
			for(j = 0; j <= fdmax; j++) {
				// Send to everyone...
				if (FD_ISSET(j, &master)) {
					// ...literally everyone!
					send(j, buf, nbytes, 0);
					// ...except the listener
					/*if (j != listener) {
						send(j, buf, nbytes, 0);
					}*/
				}
			}
		}
	}

	// How to get buffer from Buffer class
	/*
	std::vector<uint8_t>& Buffer::getData() {
		return _buffer;
	}

	recv(socket, &client->buffer.getData()[writeIndex]..)
	send(socket, &buffer.getData()[0]..)
	*/
}

BOOL CreateSocketInformation(SOCKET s)
{
	LPSOCKET_INFORMATION SI;

	printf("Accepted socket number %d\n", s);

	if ((SI = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}
	else
		printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");

	// Prepare SocketInfo structure for use
	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;

	SocketArray[TotalSockets] = SI;
	TotalSockets++;
	return(TRUE);
}

void FreeSocketInformation(DWORD Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];
	DWORD i;

	closesocket(SI->Socket);
	printf("Closing socket number %d\n", SI->Socket);
	GlobalFree(SI);

	// Squash the socket array
	for (i = Index; i < TotalSockets; i++)
	{
		SocketArray[i] = SocketArray[i + 1];
	}

	TotalSockets--;
}