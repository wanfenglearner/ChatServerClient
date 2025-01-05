#include <iostream>
#include "ChatServer.h"
#include "ChatService.h"
#include <signal.h>

// 服务器异常终止,清除用户的在线状态
void handler(int)
{
	ChatService::instance()->reset();
	exit(-1);

}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "错的的输入" << "./Server.out ip port" << std::endl;
		exit(-1);
	}

	// 读到 ip + 端口
	std::string ip = argv[1];
	int port = std::stoi(argv[2]);

	signal(SIGINT, handler);
	muduo::net::EventLoop loop;
	muduo::net::InetAddress addr(ip, port);
	ChatServer server(&loop, addr, "ChatServer");
	server.start();
	loop.loop();
	
	return 0;
}