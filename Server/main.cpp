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

int main()
{
	signal(SIGINT, handler);
	muduo::net::EventLoop loop;
	muduo::net::InetAddress addr("127.0.0.2", 8989);
	ChatServer server(&loop, addr, "ChatServer");
	server.start();
	loop.loop();
	
	return 0;
}