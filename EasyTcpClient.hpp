#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#define _WINSOCK_DEPRECATED_NO_WARNINGS
//�궨�壬���������ṩinet_ntoa
#define _CRT_SECURE_NO_WARNINGS
//����ȷ����ʹ�� scanf ʱ���ᷢ����ȫ���⣬��ϣ�����øþ���

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

	//����������
	virtual ~EasyTcpClient()
	{
		Close();
	}
	//��ʼ��socket
	void InitSocket()
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
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			printf("ERROR������socketʧ��\n");
		}
		else {
			printf("TRUE������socket�ɹ�\n");
		}
	}
	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) //����û�г�ʼ��socket
		{
			InitSocket();
		}
		//���ӷ�����connect
		sockaddr_in _sin = {}; //��ʼ��
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port);
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		//192.168.59.128Ϊ��Ӧ�������ubuntu�������ַ��ͨ��������ն�����ifconfig�鿴
		//����Ǳ��صĻ�����127.0.0.0������192.168.59.1
		//pc�˲鿴ͨ���ն�����ipconfig
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
		//the ip get by server in pc, which is VMware Network Adapter VMnet8
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf("ERROR�����ӷ�����ʧ��\n");
		}
		else {
			printf("TRUE�����ӷ������ɹ�\n");
		}
		return ret;
	}
	//�ر�socket
	void Close()
	{
		//�ر�socket closesocket
		if (_sock != INVALID_SOCKET) //�����ظ��ر�
		{
#ifdef _WIN32
			closesocket(_sock);
			//���Windows socket����
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}
	}

	//������������
	bool OnRun()
	{
		if (isRun())
		{
			fd_set fdReads;  //����ֻ������fdReads�������˲�ͬ��û������fdWrite��fdExp
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 1,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket=%d>select�������1\n", (int)_sock);
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (-1 == RecvData(_sock))
				{
					printf("<socket=%d>select�������2\n", (int)_sock);
					return false;
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
		int nLen = (int)recv(_cSock, szRecv, sizeof(DataHeader), 0); //ȡ�յ���header����
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0) {
			printf("��������Ͽ����ӣ��������\n");
			return -1;
		}
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		// +sizeof(DataHeader)����ָ��ƫ������ʹ��ָ��ƫ����header���棬�Ӷ�����header����Ϊheaderǰ���Ѿ����չ���
		OnNetMsg(header);

		return 0;
	}

	//��Ӧ��������
	virtual void OnNetMsg(DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			printf("����˷�����Ϣ���յ����CMD_LOGIN_RESULT�����ݳ��ȣ�%d\n", login->dataLength);

		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			LogoutResult* logout = (LogoutResult*)header;
			printf("����˷�����Ϣ���յ����CMD_LOGOUT_RESULT�����ݳ��ȣ�%d\n", logout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("����˷�����Ϣ���յ����CMD_NEW_USER_JOIN�����ݳ��ȣ�%d\n", userJoin->dataLength);
		}
		break;
		}
	}

	//��������
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
