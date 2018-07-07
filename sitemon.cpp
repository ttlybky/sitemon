#include"stdafx.h"
#include<iostream>
#include<string>
#include"windows.h"
#include"sitemon.h"
#include <stdio.h>
#include <WinSock.h>
using namespace std;


BOOL sitemon::GetIpByDomainName(char *szHost, char* szIp)
{
	WSADATA        wsaData;

	HOSTENT   *pHostEnt;
	int             nAdapter = 0;
	struct       sockaddr_in   sAddr;
	if (WSAStartup(0x0101, &wsaData))
	{
		printf(" gethostbyname error for host:\n");
		return FALSE;
	}

	pHostEnt = gethostbyname(szHost);
	if (pHostEnt)
	{
		if (pHostEnt->h_addr_list[nAdapter])
		{
			memcpy(&sAddr.sin_addr.s_addr, pHostEnt->h_addr_list[nAdapter], pHostEnt->h_length);
			sprintf(szIp, "%s", inet_ntoa(sAddr.sin_addr));
		}
	}
	else
	{
		//      DWORD  dwError = GetLastError();
		//      CString  csError;
		//      csError.Format("%d", dwError);
	}
	WSACleanup();
	return TRUE;
}

BOOL sitemon::sendGetRequest()
{
	//开始进行socket初始化;
	WSADATA wData;
	::WSAStartup(MAKEWORD(2, 2), &wData);

	SOCKET clientSocket = socket(AF_INET, 1, 0);
	struct sockaddr_in ServerAddr = { 0 };
	int Ret = 0;
	int AddrLen = 0;
	HANDLE hThread = 0;

	char *bufSend = "Get /check?+参数 HTTP/1.1\r\n"
		"Connection:Keep-Alive\r\n"
		"Accept-Encoding:gzip, deflate\r\n"
		"Accept-Language:zh-CN,en,*\r\n"
		"host:www.baidu.com\r\n"
		"User-Agent:Mozilla/5.0\r\n\r\n";

	char addIp[256] = { 0 };
	GetIpByDomainName("www.baidu.com", addIp);
	ServerAddr.sin_addr.s_addr = inet_addr(addIp);
	ServerAddr.sin_port = htons(80);;
	ServerAddr.sin_family = AF_INET;
	char bufRecv[3069] = { 0 };
	int errNo = 0;
	char bufData[10];
	errNo = connect(clientSocket, (sockaddr*)&ServerAddr, sizeof(ServerAddr));
	if (errNo == 0)
	{
		//如果发送成功，则返回TRUE;
		if (send(clientSocket, bufSend, strlen(bufData), 0)>0)
		{
			cout << "发送成功\n";
			return TRUE;
		}
		//如果接受成功，则返回TRUE;
		if (recv(clientSocket, bufRecv, 3069, 0)>0)
		{
			cout << "接受的数据:" << bufRecv << endl;
			return TRUE;
		}
	}
	else
	{
		errNo = WSAGetLastError();
		return FALSE;
	}
	//socket环境清理;
	::WSACleanup();
}

///////////////////////////

#pragma comment(lib, "ws2_32.lib")
#define SMTP_BUFSIZE 1024
// Base64编码
void sitemon::EncodeBase64(char* src, char* encode)
{
	char base64_table[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '=' };
	int len = strlen(src);
	int i = 0;
	for (i = 0; i < len / 3; i++)
	{
		int temp = byte(src[3 * i + 2]) + (byte(src[3 * i + 1]) << 8) + (byte(src[3 * i]) << 16);
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[(temp & 0xfc0) >> 6];
		encode[4 * i + 3] = base64_table[temp & 0x3f];
	}
	encode[4 * i] = 0;
	if (1 == len % 3)
	{
		int temp = byte(src[3 * i]) << 16;
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[64];
		encode[4 * i + 3] = base64_table[64];
		encode[4 * i + 4] = 0;
	}
	else if (2 == len % 3)
	{
		int temp = (byte(src[3 * i + 1]) << 8) + (byte(src[3 * i]) << 16);
		encode[4 * i] = base64_table[(temp & 0xfc0000) >> 18];
		encode[4 * i + 1] = base64_table[(temp & 0x3f000) >> 12];
		encode[4 * i + 2] = base64_table[(temp & 0xfc0) >> 6];
		encode[4 * i + 3] = base64_table[64];
		encode[4 * i + 4] = 0;
	}
}

// 发送邮件
int sitemon::SendMail(char *from, char *pwd, char* to, char* title, char* text)
{
	char buf[SMTP_BUFSIZE] = { 0 };
	char account[128] = { 0 };
	char password[128] = { 0 };
	// 连接邮件服务器
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(25);
	hostent* phost = gethostbyname("smtp.126.com");
	memcpy(&addr.sin_addr.S_un.S_addr, phost->h_addr_list[0], phost->h_length);
	SOCKET sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("smtp socket() error");
		return 1;
	}
	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) < 0)
	{
		printf("smtp connect() error");
		return 2;
	}
	// EHLO
	char pcname[128] = { 0 };
	DWORD size = 128;
	
	GetComputerName(pcname, &size); // 获取计算机名
	sprintf_s(buf, SMTP_BUFSIZE, "EHLO %s\r\n", pcname);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// AUTH LOGIN
	sprintf_s(buf, SMTP_BUFSIZE, "AUTH LOGIN\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 邮箱账号
	EncodeBase64(from, account);
	sprintf_s(buf, SMTP_BUFSIZE, "%s\r\n", account);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 密码
	EncodeBase64(pwd, password);
	sprintf_s(buf, SMTP_BUFSIZE, "%s\r\n", password);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// MAIL FROM 发件人
	sprintf_s(buf, SMTP_BUFSIZE, "MAIL FROM:<%s>\r\n", from);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// RCPT TO 收件人
	sprintf_s(buf, SMTP_BUFSIZE, "RCPT TO:<%s>\r\n", to);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// DATA 准备开始发送邮件内容
	sprintf_s(buf, SMTP_BUFSIZE, "DATA\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// 发送邮件内容
	sprintf_s(buf, SMTP_BUFSIZE,
		"From: \"yang\"<%s>\r\nTo: \"test\"<%s>\r\nSubject: %s\r\n\r\n%s\r\n.\r\n", from, to, title, text);
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	// QUIT 结束
	sprintf_s(buf, SMTP_BUFSIZE, "QUIT\r\n");
	send(sockfd, buf, strlen(buf), 0);
	recv(sockfd, buf, SMTP_BUFSIZE, 0);
	if (strlen(buf) >= 3)
	{
		if (buf[0] == '2' && buf[1] == '5' && buf[2] == '0')
		{
			printf("sucess\n");
		}
	}
	closesocket(sockfd);
	system("pause");
	return 0;
}
