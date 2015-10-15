// Description:
//    This sample is the echo client. It connects to the TCP server,
//    sends data, and reads data back from the server.
//
// Command Line Options:
//    client [-p:x] [-s:IP] [-n:x] [-o]
//           -p:x      Remote port to send to
//           -s:IP     Server's IP address or hostname
//           -n:x      Number of times to send message
//           -o        Send messages only; don't receive
//
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <iostream>
#include "../common/buffer.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN      2048
#define DEFAULT_PORT        "5150"
#define	DEFAULT_ADDR		"127.0.0.1"

Buffer rwBuf(DEFAULT_BUFLEN);
//char szServer[128];				// Server to connect to
//char szMessage[1024];				// Message to send to sever
//int   iPort     = DEFAULT_PORT;		// Port on server to connect to
//DWORD dwCount   = DEFAULT_COUNT;	// Number of times to send message
//BOOL  bSendOnly = FALSE;			// Send data only; don't receive

std::string username;						// - "Screen name"; attached to text messages to display who wrote it
std::vector<std::string> connectedRooms;	// - The "room" the current user is in.
											// - On server side, determines who sees messages
bool isInARoom = false;						// - Determines if certain commands are eligible
bool isGoingToQuit = false;					// - Set to 'true' when "!quit" command is used

void help()
{
	printf("You have asked for help? Please see the list of available commands: \n");
	printf("    !join [room]           ---  Join/create a room by that name\n");
	printf("    !leave [room]          ---  Leave the current room [only if user is in one]\n");
	printf("    !refresh               ---  Check for new messages\n");
	printf("    !msg [room] [msg]      ---  Sends a message in current chat [if user's in one]\n");
	printf("    !quit                  ---  Close the connection to server and leave app\n");
	printf("\n");
}

// Function: main
// Description:
//    Main thread of execution. Initialize Winsock, parse the
//    command line arguments, create a socket, connect to the
//    server, and then send and receive data.
int __cdecl main(int argc, char **argv)
{
	FD_SET clientMaster, readset;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	char sendbuf[DEFAULT_BUFLEN];
	std::string inputStr;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	bool hasNewMessages = false;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s ip-addr\n", argv[0]);
		return 1;
	}

	FD_ZERO(&clientMaster);

	// - Have client insert a "username" so that other clients can understand who sent the text -
	// - Not important to server per se, because it reads incoming activity via socket IDs -
	printf("Read the README before starting the program.\n");
	printf("Please enter a username. Hit 'Enter' to confirm.\n");
	getline(std::cin, username);
	
	printf("Connecting to server; please wait...\n");
	
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
    }

	memset( &hints, 0, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if ( iResult != 0 ) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL; ptr=ptr->ai_next) {
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen );
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	FD_SET(ConnectSocket, &clientMaster);
	
	printf("Now connected to server! Please enter '!help' to receive a list of helpful commands!\n");

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500 * 1000;	// 500 ms

	do {
		getline(std::cin, inputStr);
		if (!inputStr.compare(0, inputStr.size(), "!help")) {
			help();
			//continue;
		} else if (!inputStr.compare(0, 5, "!join")) {
			// Extract the room name
			std::string msgStr, room;
			size_t pos = inputStr.find(" ");
			if (pos != std::string::npos) {
				room = inputStr.substr(pos + 1, std::string::npos);
				msgStr.append(inputStr);
				msgStr.append(" ");
				msgStr.append(username);
			}
			if (room != "") {
				strncpy_s(sendbuf, msgStr.c_str(), DEFAULT_BUFLEN);
				iResult = send( ConnectSocket, sendbuf, DEFAULT_BUFLEN, 0 );
				if (iResult == SOCKET_ERROR) {
					printf("Send failed with error: %d\n", WSAGetLastError());
					WSACleanup();
					return 1;
				}
				printf("You have joined the room [%s].\n", room.c_str());
				connectedRooms.push_back(room);
				if (connectedRooms.size() >= 1)
					isInARoom = true;
			}
			//continue;
		} else if (!inputStr.compare(0, 6, "!leave")) {
			// Disconnect from the room if you're connected to one
			if (isInARoom) {
				strncpy_s(sendbuf, inputStr.c_str(), DEFAULT_BUFLEN);
				iResult = send( ConnectSocket, sendbuf, DEFAULT_BUFLEN, 0 );
				if (iResult == SOCKET_ERROR) {
					printf("Send failed with error: %d\n", WSAGetLastError());
					WSACleanup();
					return 1;
				}
				printf("You have left the room [%s].\n", inputStr.substr(6+1, std::string::npos));
				for (size_t i = 0; i < connectedRooms.size(); i++) {
					if (connectedRooms[i] == inputStr.substr(6+1, std::string::npos)) {
						connectedRooms.erase(connectedRooms.begin()+i);
					}
				}
				if (connectedRooms.size() == 0)
					isInARoom = false;
			} else { // Otherwise, tell the client they're silly
				printf("Silly; you're not in a room!");
			}
			//continue;
		} else if (!inputStr.compare(0, 4, "!msg")) {
			if (isInARoom) {
				std::string msgStr;
				msgStr.append("!msg ");
				size_t pos = inputStr.find(" ");
				if (pos != std::string::npos) {
					size_t nextPos = inputStr.find(" ", pos + 1);
					if (nextPos != std::string::npos) {
						msgStr.append(inputStr.substr(pos + 1, nextPos - (pos + 1)));
						msgStr.append(" ");
						msgStr.append(username);
						msgStr.append("[");
						msgStr.append(inputStr.substr(pos + 1, nextPos - (pos + 1)));
						msgStr.append("]: ");
						msgStr.append(inputStr.substr(nextPos + 1, std::string::npos));
						strncpy_s(sendbuf, msgStr.c_str(), DEFAULT_BUFLEN);
						iResult = send( ConnectSocket, sendbuf, DEFAULT_BUFLEN, 0 );
						if (iResult == SOCKET_ERROR) {
							printf("Send failed with error: %d\n", WSAGetLastError());
							WSACleanup();
							return 1;
						}
					}
					else {
						printf("Usage: !msg [roomname] [msg] Please try again!\n");
					}
				} else {
					printf("Message error: Please try again!\n");
				}
			} else {
				printf("Please join a room before trying this...\n");
			}
			//continue;
		} else if (!inputStr.compare(0, inputStr.size(), "!quit")) {
			isGoingToQuit = true;
			// Disconnect from the room[s] if you're connected to [at least] one
			if (isInARoom) {
				for (size_t i = 0; i < connectedRooms.size(); i++) {
					std::string msgStr;
					msgStr.append("!leave ");
					msgStr.append(connectedRooms[i]);
					strncpy_s(sendbuf, msgStr.c_str(), DEFAULT_BUFLEN);
					iResult = send( ConnectSocket, sendbuf, DEFAULT_BUFLEN, 0 );
					if (iResult == SOCKET_ERROR) {
						printf("Send failed with error: %d\n", WSAGetLastError());
						WSACleanup();
						return 1;
					}
					printf("You have left the room [%s].\n", connectedRooms[i].c_str());
				}
				isInARoom = false;
			}
			//continue;
		} else if (inputStr.compare(0, 8, "!refresh")) {
			printf("Command error! Reminder: Please enter "
				"'!help' to receive a list of commands.\n");
		}

		// Check for new messages coming in (runs as long as there are some
		bool firstTime = true;
		do {
			hasNewMessages = false;
			readset = clientMaster;
			if (select(ConnectSocket + 1, &readset, NULL, NULL, &tv) == -1) {
				// ???
				perror("select");
			}

			for(int i = 0; i <= ConnectSocket; i++) {
				if (FD_ISSET(i, &readset)) {
					// If we're here, there's a new message.
					hasNewMessages = true;
					iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
					if( iResult > 0 )
						printf("%s\n", recvbuf);
				} else {
					continue;
				}
			}
			if (firstTime && !hasNewMessages) {
				printf("No new messages.\n");
			}
			firstTime = false;
		} while (hasNewMessages == true);
	} while (!isGoingToQuit);

	// Shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("Shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	// Ensure server acknowledges disconnection
	do {
		iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFLEN, 0);
		if ( iResult == 0 )
			printf("You have been disconnected from the server.\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());
	} while ( iResult > 0 );

	printf("Bye-bye!\n");
	
	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	// Helper code
	/*
	WSADATA       wsd;
	SOCKET        sClient = INVALID_SOCKET;
	char          szBuffer[DEFAULT_BUFFER];
	int           ret, i;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;
	struct sockaddr_in server;
	struct hostent    *host = NULL;
	if(argc < 2)
	{
		usage();
		exit(1);
	}

	// Parse the command line and load Winsock
	//ValidateArgs(argc, argv);
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		printf("Failed to load Winsock library! Error %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("Winsock library loaded successfully!\n");

	strcpy_s(szMessage, sizeof(szMessage),DEFAULT_MESSAGE);

	// Create the socket, and attempt to connect to the server
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		printf("socket() failed with error code %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("socket() looks fine!\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(iPort);
	server.sin_addr.s_addr = inet_addr(DEFAULT_ADDR);

	// If the supplied server address wasn't in the form
	// "aaa.bbb.ccc.ddd" it's a hostname, so try to resolve it
	if (server.sin_addr.s_addr == INADDR_NONE)
	{
		CopyMemory(&server.sin_addr, host->h_addr_list[0], host->h_length);
	}

	if (connect(sClient, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("connect() failed with error code %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("connect() is pretty damn fine!\n");

	// Send and receive data
	printf("Sending and receiving data if any...\n");

	for(i = 0; i < (int)dwCount; i++)
	{
		ret = send(sClient, szMessage, strlen(szMessage), 0);
		if (ret == 0)
			break;
		else if (ret == SOCKET_ERROR)
		{
			printf("send() failed with error code %d\n", WSAGetLastError());
			break;
		}

		printf("send() should be fine. Send %d bytes\n", ret);
		if (!bSendOnly)
		{
			ret = recv(sClient, szBuffer, DEFAULT_BUFFER, 0);
			if (ret == 0)        // Graceful close
			{
				printf("It is a graceful close!\n");
				break;
			}
			else if (ret == SOCKET_ERROR)
			{
				printf("recv() failed with error code %d\n", WSAGetLastError());
				break;
			}
			szBuffer[ret] = '\0';
			printf("recv() is OK. Received %d bytes: %s\n", ret, szBuffer);
		}
	}

	if(closesocket(sClient) == 0)
		printf("closesocket() is OK!\n");
	else
		printf("closesocket() failed with error code %d\n", WSAGetLastError());

	if (WSACleanup() == 0)
		printf("WSACleanup() is fine!\n");
	else
		printf("WSACleanup() failed with error code %d\n", WSAGetLastError());
	*/
	return 0;
}