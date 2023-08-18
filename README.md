## EasyTcpServer_Client
此项目为本人学习c++服务器开发的学习项目，目前为简易版，后续一边学习，一边更新
目前包含网络Socket编程和select模型的基础，功能待完善。项目为跨平台，可在Windows, Linux, Macos等系统下编译运行。

##### 在Windows和MacOS系统下，使用常规的编译器软件即可编译运行，如Visual Studio, Xcode.

##### 在Linux系统中可使用gcc进行编译运行：

g++ server.cpp -std=c11 -o server

./server

g++ client.cpp -std=c++11 -pthread -o client

./client
