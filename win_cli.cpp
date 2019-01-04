#include <string>
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#include <stdio.h>

#include<windows.h>
#include<winnls.h>
#include <locale>
#include <cstdlib>
#include <time.h>
#pragma comment(lib,kernel32.lib)
#pragma comment(lib,ws2_32.lib)

#define SA struct sockaddr
using namespace std;

string old_string, new_string;

string GBKToUTF8(const char* strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	string strTemp = str;
	if (wstr) delete[] wstr;
	if (str) delete[] str;
	return strTemp;
}
string UTF8ToGBK(const char* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

BOOL CopyToClipboard(char* pszData)
{
	if (::OpenClipboard(NULL))
	{
		::EmptyClipboard();
		HGLOBAL clipbuffer;
		char *buffer;
		clipbuffer = ::GlobalAlloc(GMEM_DDESHARE, strlen(pszData) + 1);
		buffer = (char *)::GlobalLock(clipbuffer);
		strcpy_s(buffer, strlen(pszData) + 1, pszData);
		::GlobalUnlock(clipbuffer);
		::SetClipboardData(CF_TEXT, clipbuffer);
		::CloseClipboard();
		return TRUE;
	}
	return FALSE;
}

BOOL GetTextFromClipboard(string& str)
{
	if (::OpenClipboard(NULL))
	{
		HGLOBAL hMem = GetClipboardData(CF_TEXT);
		if (NULL != hMem)
		{
			char* lpStr = (char*)::GlobalLock(hMem);
			if (NULL != lpStr)
			{
				str = lpStr;
				::GlobalUnlock(hMem);
			}
		}
		::CloseClipboard();
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI
check(LPVOID arg)
{
	//old_string = paste();
	GetTextFromClipboard(old_string);
	while (1)
	{
		Sleep(1000);
		//new_string = paste();
		GetTextFromClipboard(new_string);
		if (new_string.compare(old_string) == 0) continue;
		cout << check <<clock() << endl;
		old_string = new_string;
		string temp = GBKToUTF8(old_string.c_str());
		int size = temp.size();
		/*for (int i = 0; i < size; i++)
			printf(%x , (int)old_string[i]);*/
		cout << endl;
		if (send(*(int*)arg, temp.c_str(), size, NULL) != size)
		{
			cout << write ERROR! << endl;
			return NULL;
		}
	}
}

void
update(int sockfd)
{
	int n;
	char mesg[5000];

again:
	while ((n = recv(sockfd, mesg, 5000, NULL)) > 0)
	{
		mesg[n] = 0;
		string temp = mesg;
		char p[5000];
		strcpy_s(p, temp.c_str());
		old_string = UTF8ToGBK(p);
		char in[5000];
		strcpy_s(in, old_string.c_str());
		CopyToClipboard(in);
		/*for (int i = 0; i < n; i++)
			printf(%x , (int)mesg[i]);*/
		cout << updata  << clock()<<endl;
		
	}

	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
	{
		cout << update ERROR! << endl;
		return;
	}
}

int main(int argc, char * argv[])
{
	SOCKET sockfd;
	struct sockaddr_in servaddr;


	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9090);
	if (InetPton(AF_INET, 172.30.70.90, (void*)&servaddr.sin_addr) <= 0)
	{
		cout << inet_pton error occured << endl;
		return 1;
	}
	/*
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1)
	{
		cout << IPaddress ERROR! << endl;
		return -1;
	}
	*/
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		cout << WSAStartup failed << endl;
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd == INVALID_SOCKET)
		{
			cout << socket ERROR! << endl;
			cout << socket failed with error :  << WSAGetLastError() << endl;
			int a;
			cin >> a;
			return -1;
		}
	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR)
	{
		cout << connect ERROR! << endl;
		int a;
		cin >>  a;
		return -1;
	}
	//pthread_create(&tid, NULL, &check, &sockfd);
	CreateThread(NULL, 0, check, &sockfd, 0, NULL);
	update(sockfd);
	return 0;
}//?? aaa
