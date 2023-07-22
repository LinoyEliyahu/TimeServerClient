#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h> 
#include <string.h>

#define TIME_PORT	27015

void menu();
void citiesMenu();
char getUserChoice(void (*print) (), bool (*isValid) (char));
bool isValidChoice(char userChoice);
bool isValidCityChoice(char userChoice);
float average(float* arr);
void clear_screen();

void main()
{

	// Initialize Winsock (Windows Sockets).
	WSAData wsaData;
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Client: Error at WSAStartup()\n";
		return;
	}

	// Client side:
	// Create a socket and connect to an internet address.
	SOCKET connSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == connSocket)
	{
		cout << "Time Client: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a client to communicate on a network, it must connect to a server.
	// Need to assemble the required data for connection in sockaddr structure.
	// Create a sockaddr_in object called server. 
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(TIME_PORT);

	// Send and receive data.
	int bytesSent = 0;
	int bytesRecv = 0;
	char sendBuff[255];
	char recvBuff[255];
	float time[100];
	float delay[100];

	while (sendBuff[0] != 'n' && sendBuff[1] != '6')
	{

		sendBuff[0] = getUserChoice(menu, isValidChoice);
		sendBuff[1] = '\0';
		if (sendBuff[0] == 'l')
		{
			sendBuff[1] = getUserChoice(citiesMenu, isValidCityChoice);
			sendBuff[2] = '\0';
		}
		if (sendBuff[0] == 'd')// Get Client To Server Delay Estimation
		{
			for (int i = 0; i < 100; i++)
			{
				bytesSent = sendto(connSocket, sendBuff, 1, 0, (const sockaddr*)&server, sizeof(server));
				time[i] = GetTickCount();
				if (bytesSent == SOCKET_ERROR)
				{
					cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
					closesocket(connSocket);
					WSACleanup();
					return;
				}
			}
			for (int i = 0; i < 100; i++)
			{
				bytesRecv = recv(connSocket, recvBuff, 255, 0);
				delay[i] = GetTickCount() - time[i];
				if (bytesRecv == SOCKET_ERROR)
				{
					cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
					closesocket(connSocket);
					WSACleanup();
					return;
				}
			}
			cout << "The client to server delay estimation is: " << average(delay) << endl;
		}
		else if (sendBuff[0] == 'e') // Measure RTT
		{
			for (int i = 0; i < 100; i++)
			{
				bytesSent = sendto(connSocket, sendBuff, 1, 0, (const sockaddr*)&server, sizeof(server));
				time[i] = GetTickCount();
				if (bytesSent == SOCKET_ERROR)
				{
					cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
					closesocket(connSocket);
					WSACleanup();
					return;
				}
				bytesRecv = recv(connSocket, recvBuff, 255, 0);
				delay[i] = GetTickCount() - time[i];

				if (bytesRecv == SOCKET_ERROR)
				{
					cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
					closesocket(connSocket);
					WSACleanup();
					return;
				}
			}
			cout << "The RTT is: " << average(delay) << endl;
		}
		else if (sendBuff[0] != 'n' && sendBuff[1] != '8')
		{
			// Asks the server what's the currnet time.
			// The send function sends data on a connected socket.
			// The buffer to be sent and its size are needed.
			// The fourth argument is an idicator specifying the way in which the call is made (0 for default).
			// The two last arguments hold the details of the server to communicate with. 
			// NOTE: the last argument should always be the actual size of the client's data-structure (i.e. sizeof(sockaddr)).
			if (sendBuff[0] == 'l')
				bytesSent = sendto(connSocket, sendBuff, 2, 0, (const sockaddr*)&server, sizeof(server));
			else
				bytesSent = sendto(connSocket, sendBuff, 1, 0, (const sockaddr*)&server, sizeof(server));
			if (SOCKET_ERROR == bytesSent)
			{
				cout << "Time Client: Error at sendto(): " << WSAGetLastError() << endl;
				closesocket(connSocket);
				WSACleanup();
				return;
			}
			cout << "Time Client: Sent: " << bytesSent << "/" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";

			// Gets the server's answer using simple recieve (no need to hold the server's address).
			bytesRecv = recv(connSocket, recvBuff, 255, 0);
			if (SOCKET_ERROR == bytesRecv)
			{
				cout << "Time Client: Error at recv(): " << WSAGetLastError() << endl;
				closesocket(connSocket);
				WSACleanup();
				return;
			}

			recvBuff[bytesRecv] = '\0'; //add the null-terminating to make it a string
			cout << "Time Client: Recieved: " << bytesRecv << " bytes of \"" << recvBuff << "\" message.\n";
		}
	}
	// Closing connections and Winsock.
	cout << "Time Client: Closing Connection.\n";
	closesocket(connSocket);

	system("pause");
}

void menu()
{
	cout << "-This is the Time Server!--------------------------\n"
		<< "---------------------------------------------------\n"
		<< "-Press a - n to choose your option:----------------\n"
		<< "-To exit choose n ---------------------------------\n"
		<< " a  -  GetTime ------------------------------------\n"
		<< " b  -  GetTimeWithoutDate -------------------------\n"
		<< " c  -  GetTimeSinceEpoch --------------------------\n"
		<< " d  -  GetClientToServerDelayEstimation -----------\n"
		<< " e  -  MeasureRTT ---------------------------------\n"
		<< " f  -  GetTimeWithoutDateOrSeconds ----------------\n"
		<< " g  -  GetYear ------------------------------------\n"
		<< " h  -  GetMonthAndDay -----------------------------\n"
		<< " i  -  GetSecondsSinceBeginingOfMonth -------------\n"
		<< " j  -  GetWeekOfYear ------------------------------\n"
		<< " k  -  GetDaylightSavings -------------------------\n"
		<< " l  -  GetTimeWithoutDateInCity -------------------\n"
		<< " m  -  MeasureTimeLap -----------------------------\n"
		<< "---------------------------------------------------\n";
}

void citiesMenu()
{
	cout << "-In which city do you want to know the time ? -----\n"
		<< "-Press 1 - 5 to choose your option:----------------\n"
		<< "-To exit choose 6 ---------------------------------\n"
		<< " 1  -  DOHA ---------------------------------------\n"
		<< " 2  -  PRAGUE -------------------------------------\n"
		<< " 3  -  NEW-YORK -----------------------------------\n"
		<< " 4  -  BERLIN -------------------------------------\n"
		<< " 5  -  ELSE ---------------------------------------\n"
		<< "---------------------------------------------------\n";
}

char getUserChoice(void (*print) (), bool (*isValid) (char))
{
	char userChoice;
	print();
	cin >> userChoice;
	while (!isValid(userChoice))
	{
		clear_screen();
		cout << "Your choice is not valid, please choose again" << endl;
		print();
		cin >> userChoice;
	}
	clear_screen();
	return userChoice;
}

bool isValidChoice(char userChoice)
{
	return (userChoice >= 'a' && userChoice <= 'n');
}

bool isValidCityChoice(char userChoice)
{
	return (userChoice >= '1' && userChoice <= '6');
}

float average(float* arr)
{
	float sum = 0;
	for (int i = 0; i < 100; i++)
		sum += arr[i];
	return sum / 100;
}

void clear_screen()
{
	system("cls");
}
