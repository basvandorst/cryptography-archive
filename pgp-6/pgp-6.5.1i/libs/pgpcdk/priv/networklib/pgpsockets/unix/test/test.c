/*
** Copyright (C)1997 Network Associates Inc. and affiliated companies.
** All rights reserved.
*/

#if PGP_WIN32
# include <windows.h>
#endif

#include <stdio.h>

#include "pgpTypes.h"

#include "pgpSockets.h"

int main(void)
{
    PGPHostEntry * pHostEnt;		// Internet host information structure
    PGPSocketAddressInternet sockAddr;	// Socket address structure
    PGPSocketRef s;			// socket for our connection
    PGPInt32 result;				// Socket connection results
    char szMsg[100] = "Testing...1...2...3\r\n";	// Message buffer
    char szResponse[100] = {0x00};	// Response buffer
    
    PGPSocketsInit();

    pHostEnt = PGPGetHostByName("privnet.pgp.com");
    
    if (!pHostEnt) {
	fprintf(stderr, "Could not get IP address!\n");
	return PGPGetLastSocketsError();
    }
    
    // Configure the socket
    
    // Define the socket address
    sockAddr.sin_family = kPGPAddressFamilyInternet;
    sockAddr.sin_port = htons(7); //echo port
    sockAddr.sin_addr = *((PGPInternetAddress*)*pHostEnt->h_addr_list);

    
    s = PGPOpenSocket(kPGPAddressFamilyInternet, 
		      kPGPSocketTypeStream, 
		      kPGPTCPProtocol);
    
    if ((int)s == -1) {
	fprintf(stderr, "Invalid socket!\n");
	return PGPGetLastSocketsError();
    }
    
    // Connect the socket
    result = PGPConnect(s, (PGPSocketAddress*)&sockAddr, 
			sizeof(sockAddr));
    
    PGPSend(s, szMsg, strlen(szMsg), kPGPSendFlagNone);
    
    PGPReceive(s, szResponse, sizeof(szResponse), kPGPReceiveFlagNone);
    
    fprintf(stderr, "response: %s\n", szResponse);

    PGPCloseSocket(s);
    
    PGPSocketsCleanup();
    
    return 0;
}



/*

#include <winsock.h>

int main(void)
{
	WSADATA wsaData;
	LPHOSTENT pHostEnt;		// Internet host information structure
	SOCKADDR_IN sockAddr;	// Socket address structure
	SOCKET s;				// socket for our connection
	int result;				// Socket connection results
	char szMsg[100] = "Testing...1...2...3\r\n";	// Message buffer
	char szResponse[100] = {0x00};		// Response buffer

	WSAStartup(MAKEWORD(1,1), &wsaData);

	pHostEnt = gethostbyname("privnet.pgp.com");

	if (!pHostEnt)
	{
		MessageBox(NULL, "Could not get IP address!", 0,MB_OK);
		return INVALID_SOCKET;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
										
	if (s == INVALID_SOCKET)
	{
		MessageBox(NULL, "Invalid socket!!", 0,MB_OK|MB_ICONSTOP);
		return INVALID_SOCKET;
	}

	// Configure the socket

	// Define the socket address
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(7); //echo port
	sockAddr.sin_addr = *((LPIN_ADDR)*pHostEnt->h_addr_list);

	// Connect the socket
	result = connect(s, (LPSOCKADDR)&sockAddr, sizeof(sockAddr));

	send(s, szMsg, strlen(szMsg), 0);

	recv(s, szResponse, sizeof(szResponse), 0);
	
	MessageBox(NULL, szResponse, "szResponse",MB_OK);

	closesocket(s);

	WSACleanup();

	return 0;

}
*/