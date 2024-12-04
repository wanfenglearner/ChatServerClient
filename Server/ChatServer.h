
#ifndef _CHATSERVER_H_
#define _CHATSERVER_H_

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <functional>
#include <string>
#include <muduo/base/LogFile.h>
#include <muduo/base/Logging.h>
#include <iostream>
#include <json.hpp>
#include "public.h"
#include "ChatService.h"

// 使用命名空间包含 json
using Json = nlohmann::json;

class ChatServer
{
public:
	// 初始化服务器
	ChatServer(muduo::net::EventLoop* loop,
		const muduo::net::InetAddress& listenAddr,
		const std::string& nameArg);

	// 启动服务器
	void start();	
private:
	// 专门用来处理连接的函数
	void onConnection(const muduo::net::TcpConnectionPtr& conn);
	// 专门用来处理消息的函数
	void onMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buffer,
		muduo::Timestamp receiveTime);
private:
	muduo::net::TcpServer server_;		// 服务器
	muduo::net::EventLoop* loop_;		// 指向事件循环对象的指针
	
};

#endif


