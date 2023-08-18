#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//宏定义，这里用来提供inet_ntoa
#define _CRT_SECURE_NO_WARNINGS
//定义确信在使用 scanf 时不会发生安全问题，并希望禁用该警告

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include<unistd.h> //uni std
#include<arpa/inet.h>
#include<string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <stdio.h>
#include "MessageHeader.hpp"

class EasyTcpClient
{
	SOCKET _sock;
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	}

	//虚析构函数
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//初始化socket
	void InitSocket()
	{
#ifdef _WIN32
		//启动Windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//用socket API建立简易TCP客户端：
		//建立一个socket
		if (INVALID_SOCKET != _sock) //避免重复创建
		{
			printf("<socket=%d>关闭了旧连接...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("ERROR，建立socket失败\n");
		}
		else {
			printf("TRUE，建立socket成功\n");
		}
	}
	//连接服务器
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) //避免没有初始化socket
		{
			InitSocket();
		}
		//连接服务器connect
		sockaddr_in _sin = {}; //初始化
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		//192.168.59.128为对应虚拟机的ubuntu的虚拟地址，通过虚拟机终端输入ifconfig查看
		//如果是本地的话就是127.0.0.0，或者192.168.59.1
		//pc端查看通过终端输入ipconfig
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
		//the ip get by server in pc, which is VMware Network Adapter VMnet8
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("ERROR，连接服务器失败\n");
		}
		else {
			printf("TRUE，连接服务器成功\n");
		}
		return ret;
	}
	//关闭socket
	void Close()
	{
		//关闭socket closesocket
		if (_sock != INVALID_SOCKET) //避免重复关闭
		{
#ifdef _WIN32
			closesocket(_sock);
			//清除Windows socket环境
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}

	//处理网络数据
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;  //这里只设置了fdReads，与服务端不同，没有设置fdWrite和fdExp
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select任务结束1\n", (int)_sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select任务结束2\n", (int)_sock);
					return false;
				}
			}
			return true;
		}
		return false;
	}
	//是否处理网络中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//接收数据 之后需要处理粘包和拆包
	int RecvData(SOCKET _cSock)
	{
		//缓冲区
		char szRecv[4096] = {};
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0); //取收到的header部分
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("与服务器断开连接，任务结束\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		// +sizeof(DataHeader)代表指针偏移量，使得指针偏移至header后面，从而忽略header，因为header前面已经接收过了
		OnNetMsg(header);

		return 0;
	}

	//响应网络数据
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			printf("服务端返回消息：收到命令：CMD_LOGIN_RESULT，数据长度：%d\n", login->dataLength);

		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logout = (LogoutResult*)header;
			printf("服务端返回消息：收到命令：CMD_LOGOUT_RESULT，数据长度：%d\n", logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("服务端返回消息：收到命令：CMD_NEW_USER_JOIN，数据长度：%d\n", userJoin->dataLength);
		}
		break;
		}
	}

	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
			//(const char*)header
		}
		return SOCKET_ERROR;
	}

private:

};


#endif // !_EasyTcpClient_hpp_
