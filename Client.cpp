#include "EasyTcpClient.hpp"
#include<thread> //线程库


//线程处理函数
void cmdThread(EasyTcpClient* client)
{
	while (true)
	{

		char cmdBuf[256] = {};
		scanf("%s",cmdBuf);

		//处理请求
		if (0 == strcmp(cmdBuf, "exit")) {
			client->Close();
			printf("收到exit，任务结束 \n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login;
			strcpy(login.userName, "lpf");   //c++的字符串不能直接赋值，要这样拷贝
			strcpy(login.PassWord, "lpf666");
			client->SendData(&login);
			//接收服务器返回数据
			/*LoginResult loginRet = {};
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d \n", loginRet.result);*/
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "lpf");
			client->SendData(&logout);
			//接收服务器返回数据
			/*LogoutResult logoutRet = {};
			recv(_sock, (char*)&logoutRet, sizeof(logoutRet), 0);
			printf("LogoutResult: %d \n", logoutRet.result);*/
		}
		else {
			printf("命令不存在，请重新输入\n");
		}
	}

}

int main() {

	EasyTcpClient client;
	//client.InitSocket();
	client.Connect("127.0.0.1", 4567);

	//启动UI线程
	std::thread t1(cmdThread, &client); //&client传递指针，不同平台都可以
	t1.detach(); //主次线程分离

	while (client.isRun()){

		client.OnRun();

		//printf("空闲时间处理其他业务\n");

		//Sleep(1000);


	}
	client.Close();
	
	printf("客户端已退出，任务结束。");
	getchar();
	return 0;
}