#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

#define _CRT_SECURE_NO_WARNINGS
//����ȷ����ʹ�� scanf ʱ���ᷢ����ȫ���⣬��ϣ�����øþ���
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//�궨�壬���������ṩinet_ntoa

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
	std::vector<SOCKET> g_clients;  //�洢����Ŀͻ���
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}
	~EasyTcpServer()
	{
		Close();
	}

	//��ʼ��Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		//����Windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//��socket API��������TCP�ͻ��ˣ�
		//����һ��socket
		if (INVALID_SOCKET != _sock) //�����ظ�����
		{
			printf("<socket=%d>�ر��˾�����...\n", (int)_sock);
			Close();
		}
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //����������socket
		if (INVALID_SOCKET == _sock) {
			printf("ERROR������socketʧ��\n");
		}
		else {
			printf("TRUE������socket�ɹ�\n");
		}
		return _sock;
	}
	//��ip�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) //����û�г�ʼ��socket
		{
			InitSocket();
		}
		sockaddr_in _sin = {}; //��ʼ��
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //host to nnet unsigned short
#ifdef _WIN32
		//_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //����޶�����ip��ַ
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY; //���е�ip��ַ��������
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY; //���е�ip��ַ��������
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
		if (SOCKET_ERROR == ret) {  //(sockaddr*)��������ǿ��ת������
			printf("ERRORR�������ڽ��ܿͻ������ӵ�����˿�<%d>ʧ��\n",port);
		}
		else {
			printf("TRUE�������ڽ��ܿͻ������ӵ�����˿�<%d>�ɹ�\n",port);
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n)
	{
		int ret = listen(_sock, n);//nΪ�˿�����������������Լ�����
		if (SOCKET_ERROR == ret) { 
			printf("ERRORR����������˿�<socket=%d>ʧ��\n", (int)_sock);
		}
		else {
			printf("TRUE����������˿�<socket=%d>�ɹ�\n", (int)_sock);
		}
		return ret;
	}
	//���ܿͻ�������
	SOCKET Accept()
	{
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSock = INVALID_SOCKET;   //������ʾ�ͻ��˵�socket
#ifdef _WIN32
		_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen); //���տͻ��˵�socket
#else
		_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif

		if (INVALID_SOCKET == _cSock) {
			printf("<socket=%d>ERRORR��������Ч��SOCKET\n", (int)_sock);
		}
		else
		{
			printf("<socket=%d>TRUE��������Ч��SOCKET\n", (int)_sock);
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSock);
			printf("<socket=%d>���µĿͻ��˼��룺<socket = %d>,<IP = %s>\n",(int)_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr)); //inet_ntoaת��Ϊ�ɶ��Ե��ַ���

		}
		return _cSock;
	}
	//�ر�Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}

			//8.�ر�socket closesocket
			closesocket(_sock);
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			// 8 �ر��׽���closesocket
			close(_sock);
#endif
		}
	}
	
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSD socket �κ��˽�
			//selectģ��
			fd_set fdRead; //���������ϣ�socket������
			fd_set fdWrite;
			fd_set fdExp;

			//��ռ���
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);

			//���������ϣ�socket�����뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;

			for (int n = (int)g_clients.size() - 1; n >= 0; n--)  //size_t ���޷��ŵģ���ô����û�и���������Ͳ���ʹ������Ϊn--
			{
				FD_SET(g_clients[n], &fdRead);
				if (maxSock < g_clients[n])
				{
					maxSock = g_clients[n];
				}
			}

			//nfds��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ��������ֵ+1����windows�� _sock + 1 �����������д0
			timeval t = { 1,0 };     //�����������Ϊ������
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			if (ret < 0)
			{
				printf("select��������� \n");
				Close();
				return false;
			}

			//�ж���������socket���Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead))
			{
				FD_CLR(_sock, &fdRead);

				//�ȴ����ܿͻ�������accept
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
	//�Ƿ���������
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�������� ֮����Ҫ����ճ���Ͳ��
	int RecvData(SOCKET _cSock)
	{
		//������
		char szRecv[4096] = {};
		//5.���տͻ��˵�����
		int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0); //ȡ�յ���header����
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("�ͻ���<Socket=%d>���˳����������\n", (int)_cSock);
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		// +sizeof(DataHeader)����ָ��ƫ������ʹ��ָ��ƫ����header���棬�Ӷ�����header����Ϊheaderǰ���Ѿ����չ���
		OnNetMsg(_cSock, header);
		return 0;
	}
	//��Ӧ��������
	virtual void OnNetMsg(SOCKET _cSock, DataHeader* header)
	//_cSock����ǰ����Ŀͻ��ˣ���Ϊ����˷����ڶ���ͻ��ˣ�����������ͻ��˲�ͬ
	{
		//6.����ͻ��˵�����
			//if(nLen >= sizeof(DataHeader)) ����ͻ���ʱ����Ҫ��������
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			printf("�ͻ��ˣ�<Socket=%d>, �յ����CMD_LOGIN�����ݳ��ȣ�%d��userName = %s��PassWord = %s \n", (int)_cSock, login->dataLength, login->userName, login->PassWord);
			//�����ж��û��������Ƿ���ȷ
			LoginResult ret;
			//7.��ͻ��˷���һ������send
			//send(_cSock, (char*)&header, sizeof(header), 0);  header����Ϣ��Ϊһ�壬���ԾͲ��õ���������
			send(_cSock, (char*)&ret, sizeof(ret), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			printf("�ͻ��ˣ�<Socket=%d>,�յ����CMD_LOGOUT�����ݳ��ȣ�%d��userName = %s \n", (int)_cSock, logout->dataLength, logout->userName);
			//�����ж��û��������Ƿ���ȷ
			LogoutResult ret;
			//7.��ͻ��˷�������send
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
	//����ָ��Socket����
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
			//(const char*)header
		}
		return SOCKET_ERROR;
	}
	//����ָ��Socket����
	void SendDataToAll(DataHeader* header)
	{
		if (isRun() && header)
		{
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				//ѭ���������ͻ��˷������ݣ���������
				SendData(g_clients[n], header);
			}
		}
	}

};


#endif // !_EasyTcpServer_hpp_