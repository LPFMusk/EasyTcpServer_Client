#include "EasyTcpServer.hpp"



int main() {

	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);

	while (server.isRun() ){               //循环，使得可以接受多个客户端连接

		server.OnRun();
		//printf("空闲时间处理其他业务\n");

	}

	server.Close();

	printf("服务端已退出，任务结束。\n");
	getchar(); //防止窗口一闪而过
	return 0;
}