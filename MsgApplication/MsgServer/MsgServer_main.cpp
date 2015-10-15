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
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>		// Windows equiv. to UNIX libraries
#include <windows.h>
#include <map>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include "../common/buffer.h"

#pragma comment (lib, "Ws2_32.lib")

#define PORT 5150
#define SERVPORT "5150"
#define	ADDR "127.0.0.1"
#define DATA_BUFSIZE 2048

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

typedef struct _CLIENT_INFORMATION {
	SOCKET Socket;
	std::string name;
} _CLIENT_INFORMATION;

// Map of rooms
std::map< std::string, std::vector< _CLIENT_INFORMATION > > chatRooms;

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

			// - COMMAND READING [oooooo~] -
			std::string dsrlzdBuf(buf);
			if (!dsrlzdBuf.compare(0, 5, "!join")) {
				// Extract the room name
				_CLIENT_INFORMATION cli;
				cli.Socket = i;
				std::string roomName;
				size_t pos = dsrlzdBuf.find(" ");
				if (pos != std::string::npos) {
					size_t nextPos = dsrlzdBuf.find(" ", pos + 1);
					if (nextPos != std::string::npos) {
						roomName = dsrlzdBuf.substr(pos + 1, nextPos - (pos + 1));
						cli.name = dsrlzdBuf.substr(nextPos + 1, std::string::npos);
					}
					chatRooms[roomName].push_back(cli);
					std::string msg;
					msg.append(cli.name);
					msg.append(" has joined the room [");
					msg.append(roomName);
					msg.append("].");
					strncpy_s(buf, msg.c_str(), DATA_BUFSIZE);
					for(j = 0; j <= fdmax; j++) {
						// Send to everyone...
						if (FD_ISSET(j, &master)) {
							for (int k = 0; k < chatRooms[roomName].size(); k++) 
							{
								// ...in the room
								if (chatRooms[roomName][k].Socket == j
									&& j != i)
								{
									send(j, buf, nbytes, 0);
								}
							}
						}
					}
				}
			} else if (!dsrlzdBuf.compare(0, 6, "!leave"))
			{
				std::string roomName;
				size_t pos = dsrlzdBuf.find(" ");
				if (pos != std::string::npos) {
					roomName = dsrlzdBuf.substr(pos + 1, std::string::npos);
					std::string msg;
					int ij = 0;
					for (;ij < chatRooms[roomName].size(); ij++) {
						msg.append(chatRooms[roomName][ij].name);
						break;
					}
					chatRooms[roomName].erase(chatRooms[roomName]
					.begin() + ij);
					msg.append(" has left the room [");
					msg.append(roomName);
					msg.append("].");
					strncpy_s(buf, msg.c_str(), DATA_BUFSIZE);
					for(j = 0; j <= fdmax; j++) {
						// Send to everyone...
						if (FD_ISSET(j, &master)) {
							for (int k = 0; k < chatRooms[roomName].size(); k++) 
							{
								// ...in the room
								if (chatRooms[roomName][k].Socket == j
									&& j != i)
								{
									send(j, buf, nbytes, 0);
								}
							}
						}
					}
				}
			} else if (!dsrlzdBuf.compare(0, 4, "!msg"))
			{
				std::string roomName, IOMsg;
				size_t pos = dsrlzdBuf.find(" ");
				if (pos != std::string::npos) {
					size_t nextPos = dsrlzdBuf.find(" ", pos + 1);
					if (nextPos != std::string::npos) {
						roomName = dsrlzdBuf.substr(pos + 1, nextPos - (pos + 1));
						IOMsg = dsrlzdBuf.substr(nextPos + 1, std::string::npos);
						strncpy_s(buf, IOMsg.c_str(), DATA_BUFSIZE);
						for(j = 0; j <= fdmax; j++) {
							// Send to everyone...
							if (FD_ISSET(j, &master)) {
								for (int k = 0; k < chatRooms[roomName].size(); k++) 
								{
									// ...in the room
									if (chatRooms[roomName][k].Socket == j
										&& j != i)
									{
										send(j, buf, nbytes, 0);
									}
								}
							}
						}
					}
				}
			}

			// We go some data from a client
			//for(j = 0; j <= fdmax; j++) {
			//	// Send to everyone...
			//	if (FD_ISSET(j, &master)) {
			//		// ...literally everyone!
			//		send(j, buf, nbytes, 0);
			//		// ...except the listener
			//		/*if (j != listener) {
			//			send(j, buf, nbytes, 0);
			//		}*/
			//	}
			//}
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