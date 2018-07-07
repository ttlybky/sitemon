#pragma once
#include"windows.h"
#ifndef SITEMON_H
#define SITEMON_H
class sitemon
{
public:
	BOOL GetIpByDomainName(char *szHost, char* szIp);
	BOOL sendGetRequest();
	void EncodeBase64(char* src, char* encode);                             // Base64编码
	int SendMail(char *from, char *pwd, char* to, char* title, char* text); // 发送邮件
};


#endif // !GET_H
