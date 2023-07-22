#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <map>

#define TIME_PORT	27015

void handleRequest(char* recvBuff, char* sendBuff);

void main()
{
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called m_socket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							datagram sockets (SOCK_DGRAM), 
	//							and the UDP/IP protocol (IPPROTO_UDP).
	SOCKET m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The "if" statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == m_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must first bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigdned long (4 bytes) data type).
	// INADDR_ANY means to listen on all interfaces.
	// inet_addr (Internet address) is used to convert a string (char *) into unsigned int.
	// inet_ntoa (Internet address) is the reverse function (converts unsigned int to char *)
	// The IP address 127.0.0.1 is the host itself, it's actually a loop-back.
	serverService.sin_addr.s_addr = INADDR_ANY;	//inet_addr("127.0.0.1");
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		return;
	}

	// Waits for incoming requests from clients.

	// Send and receive data.
	sockaddr client_addr;
	int client_addr_len = sizeof(client_addr);
	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];

	// Get client's requests and answer them.
	// The recvfrom function receives a datagram and stores the source address.
	// The buffer for data to be received and its available size are 
	// returned by recvfrom. The fourth argument is an idicator 
	// specifying the way in which the call is made (0 for default).
	// The two last arguments are optional and will hold the details of the client for further communication. 
	// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
	cout << "Time Server: Wait for clients' requests.\n";

	while (true)
	{
		bytesRecv = recvfrom(m_socket, recvBuff, 255, 0, &client_addr, &client_addr_len);
		if (SOCKET_ERROR == bytesRecv)
		{
			cout << "Time Server: Error at recvfrom(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";

		// Answer client's request 
		handleRequest(recvBuff, sendBuff);
		sendBuff[strlen(sendBuff) - 1] = '\0'; //to remove the new-line from the created string

		// Sends the answer to the client, using the client address gathered
		// by recvfrom. 
		bytesSent = sendto(m_socket, sendBuff, (int)strlen(sendBuff), 0, (const sockaddr*)&client_addr, client_addr_len);
		if (SOCKET_ERROR == bytesSent)
		{
			cout << "Time Server: Error at sendto(): " << WSAGetLastError() << endl;
			closesocket(m_socket);
			WSACleanup();
			return;
		}

		cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(m_socket);
	WSACleanup();
}

void handleRequest(char* recvBuff, char* sendBuff)
{
	static time_t measureSec = 0;

	enum OPTIONS
	{
		GetTime = 1, GetTimeWithoutDate = 2, GetTimeSinceEpoch = 3, GetClientToServerDelayEstimation = 4,
		MeasureRTT = 5, GetTimeWithoutDateOrSeconds = 6, GetYear = 7, GetMonthAndDay = 8,
		GetSecondsSinceBeginingOfMonth = 9, GetWeekOfYear = 10, GetDaylightSavings = 11,
		GetTimeWithoutDateInCity = 12, MeasureTimeLap = 13
	};

	enum CITIES
	{
		DOHA = 3, PRAGUE = 1, NEW_YORK = -5, BERLIN = 1, OTHER = 0
	};

	map<char, int> cityNames;

	cityNames['1'] = CITIES::DOHA;
	cityNames['2'] = CITIES::PRAGUE;
	cityNames['3'] = CITIES::NEW_YORK;
	cityNames['4'] = CITIES::BERLIN;
	cityNames['5'] = CITIES::OTHER;

	int recv = recvBuff[0] - 'a' + 1;
	int dayLightSavings;
	time_t gmt_time, local_time, sec;
	gmt_time = time(nullptr);
	time(&local_time);
	tm* tm_local = localtime(&local_time);
	tm* tm_gmt;
	int seconds;

	switch (recv)
	{
	case OPTIONS::GetClientToServerDelayEstimation:
	case OPTIONS::MeasureRTT:
	case OPTIONS::GetTime:
		// Get the current time.
		time_t timer;
		time(&timer);
		// Parse the current time to printable string.
		strcpy(sendBuff, ctime(&timer));
		break;
	case OPTIONS::GetTimeWithoutDate:
		strftime(sendBuff, 255, "The time in hours::minutes::seconds: %H:%M:%S ", tm_local);
		break;
	case OPTIONS::GetTimeSinceEpoch:
		sec = time(NULL);
		sprintf(sendBuff, "The seconds since 1.1.1970: %d ", sec);
		break;
	case OPTIONS::GetTimeWithoutDateOrSeconds:
		strftime(sendBuff, 255, "The time in hours::minuets: %H:%M ", tm_local);
		break;
	case OPTIONS::GetYear:
		strftime(sendBuff, 255, "The current year: %Y ", tm_local);
		break;
	case OPTIONS::GetMonthAndDay:
		strftime(sendBuff, 255, "The month, day are: %B , %A ", tm_local);
		break;
	case OPTIONS::GetSecondsSinceBeginingOfMonth:
		seconds = tm_local->tm_mday * 24 * 60 * 60 + tm_local->tm_hour * 60 * 60 + tm_local->tm_min * 60 + tm_local->tm_sec;
		sprintf(sendBuff, "The seconds since the begining of the month: %d ", seconds);
		break;
	case OPTIONS::GetWeekOfYear:
		strftime(sendBuff, 255, "The current week of the year: %U ", tm_local);
		break;
	case OPTIONS::GetDaylightSavings:
		tm_local->tm_isdst >= 1 ? dayLightSavings = 1 : dayLightSavings = 0;
		sprintf(sendBuff, "1-Day light saving 0-else: %d ", dayLightSavings);
		break;
	case OPTIONS::GetTimeWithoutDateInCity:
		tm_local->tm_isdst >= 1 ? dayLightSavings = 1 : dayLightSavings = 0;
		gmt_time = time(nullptr);
		tm_gmt = gmtime(&gmt_time);
		tm_gmt->tm_hour += cityNames[recvBuff[1]] + 24;
		if (cityNames[recvBuff[1]] != CITIES::DOHA && cityNames[recvBuff[1]] != CITIES::OTHER)
			tm_gmt->tm_hour += dayLightSavings;
		tm_gmt->tm_hour = tm_gmt->tm_hour % 24;
		strftime(sendBuff, 255, "The time in hours::minutes::seconds: %H:%M:%S ", tm_gmt);
		break;
	case OPTIONS::MeasureTimeLap:
		if (measureSec == 0)
		{
			measureSec = time(NULL);
			strcpy(sendBuff, "The measurement was activated! ");
		}
		else
		{
			measureSec = time(NULL) - measureSec;
			if (measureSec <= 180)
			{
				sprintf(sendBuff, "The measurement was %d sec ", measureSec);
			}
			measureSec = 0;
		}
		break;
	default:
		break;
	}
}