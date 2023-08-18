#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#define _CRT_SECURE_NO_WARNINGS
//定义确信在使用 scanf 时不会发生安全问题，并希望禁用该警告
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//宏定义，这里用来提供inet_ntoa

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
#include<vector>
#include "MessageHeader.hpp"

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<SOCKET> g_clients;  //存储加入的客户端
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	~EasyTcpServer()
	{
		Close();
	}

	//初始化Socket
	SOCKET InitSocket()
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
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //服务端自身的socket
		if (INVALID_SOCKET == _sock) {
			printf("ERROR，建立socket失败\n");
		}
		else {
			printf("TRUE，建立socket成功\n");
		}
		return _sock;
	}
	//绑定ip和端口号
	int Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) //避免没有初始化socket
		{
			InitSocket();
		}
		sockaddr_in _sin = {}; //初始化
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to nnet unsigned short
#ifdef _WIN32
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //这个限定本地ip地址
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; //所有的ip地址都可以用
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY; //所有的ip地址都可以用
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {  //(sockaddr*)的作用是强制转换类型
			printf("ERRORR，绑定用于接受客户端连接的网络端口<%d>失败\n",port);
		}
		else {
			printf("TRUE，绑定用于接受客户端连接的网络端口<%d>成功\n",port);
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);//n为端口最大连接数，可以自己设置
		if (SOCKET_ERROR == ret) { 
			printf("ERRORR，监听网络端口<socket=%d>失败\n", (int)_sock);
		}
		else {
			printf("TRUE，监听网络端口<socket=%d>成功\n", (int)_sock);
		}
		return ret;
	}
	//接受客户端连接
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;   //用来表示客户端的socket
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); //接收客户端的socket
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif

		if (INVALID_SOCKET == _cSock) {
			printf("<socket=%d>ERRORR，接收无效的SOCKET\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>TRUE，接收有效的SOCKET\n", (int)_sock);
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("<socket=%d>有新的客户端加入：<socket = %d>,<IP = %s>\n",(int)_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr)); //inet_ntoa转换为可读性的字符串

		}
		return _cSock;
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}

			//8.关闭socket closesocket
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			// 8 关闭套节字closesocket
			close(_sock);
#endif
		}
	}
	
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利套接字 BSD socket 课后了解
			//select模型
			fd_set fdRead; //描述符集合（socket）集合
			fd_set fdWrite;
			fd_set fdExp;

			//清空集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			//描述符集合（socket）加入集合
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;

			for (int n = (int)g_clients.size() - 1; n >= 0; n--)  //size_t 是无符号的，那么就是没有负数，这里就不能使用来作为n--
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}

			//nfds是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			//既是所有文件描述最大值+1，在windows中 _sock + 1 这个参数可以写0
			timeval t = { 1,0 };     //将服务端设置为非阻塞
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select任务结束。 \n");
				Close();
				return false;
			}

			//判断描述符（socket）是否在集合中
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				//等待接受客户端连接accept
				Accept();

			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (-1 == RecvData(g_clients[n]))
					{
						auto iter = g_clients.begin() + n;//std::vector<SOCKET>::iterator
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
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
		//5.接收客户端的请求
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0); //取收到的header部分
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("客户端<Socket=%d>已退出，任务结束\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		// +sizeof(DataHeader)代表指针偏移量，使得指针偏移至header后面，从而忽略header，因为header前面已经接收过了
		OnNetMsg(_cSock, header);
		return 0;
	}
	//响应网络数据
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	//_cSock代表当前处理的客户端，因为服务端服务于多个客户端，所以这里与客户端不同
	{
		//6.处理客户端的请求
			//if(nLen >= sizeof(DataHeader)) 多个客户端时就需要这样考虑
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("客户端：<Socket=%d>, 收到命令：CMD_LOGIN，数据长度：%d，userName = %s，PassWord = %s \n", (int)_cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户名密码是否正确
			LoginResult ret;
			//7.向客户端发送一条数据send
			//send(_cSock, (char*)&header, sizeof(header), 0);  header与信息合为一体，所以就不用单独发送了
			send(_cSock, (char*)&ret, sizeof(ret), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("客户端：<Socket=%d>,收到命令：CMD_LOGOUT，数据长度：%d，userName = %s \n", (int)_cSock, logout->dataLength, logout->userName);
			//忽略判断用户名密码是否正确
			LogoutResult ret;
			//7.向客户端发送数据send
			//send(_cSock, (char*)&header, sizeof(header), 0);
			send(_cSock, (char*)&ret, sizeof(ret), 0);
		}
		break;
		default:
		{
			DataHeader header = { CMD_ERROR,0 };
			send(_cSock, (char*)&header, sizeof(header), 0);
		}
		break;
		}
	}
	//发送指定Socket数据
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
			//(const char*)header
		}
		return SOCKET_ERROR;
	}
	//发送指定Socket数据
	void SendDataToAll(DataHeader* header)
	{
		if (isRun() && header)
		{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				//循环向其他客户端发送数据，告诉他们
				SendData(g_clients[n], header);
			}
		}
	}

};


#endif // !_EasyTcpServer_hpp_