#include <iostream>
#include "ChatServer.h"

int main()
{
	muduo::net::EventLoop loop;
	muduo::net::InetAddress addr("127.0.0.2", 8989);
	ChatServer server(&loop, addr, "ChatServer");
	server.start();
	loop.loop();
	
	return 0;
}