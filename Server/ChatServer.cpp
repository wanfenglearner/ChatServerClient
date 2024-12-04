#include "ChatServer.h"

ChatServer::ChatServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress& listenAddr,
	const std::string& nameArg)
	:server_(loop, listenAddr, nameArg), loop_(loop)
{
	// 设置连接回调
	server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
	// 设置消息回调
	server_.setMessageCallback(std::bind(&ChatServer::onMessage, this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	server_.setThreadNum(4);		// 线程的数量

}

// 专门用来处理连接的函数
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	// 客户端断开了连接
	if (!conn->connected())
	{
		// 把当前用户从connMap_ 里删除并设置下线
		ChatService::instance()->clientCloseException(conn);
		conn -> shutdown();
	}
}
// 专门用来处理消息的函数
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buffer, muduo::Timestamp receiveTime)
{
	std::string buf = buffer->retrieveAllAsString();
	Json js = Json::parse(buf);
	LOG_INFO << __FILE__ << "服务端收到的消息:" << buf;
	// 根据js["msgid"]从服务层中调入相应的函数进行处理, 避免网络层和服务层在一起
	int msgid = js["msgid"].get<int>();
	auto handlerFunc = ChatService::instance()->getHandler(msgid);
	// 调入相应的消息处理函数 
	handlerFunc(conn, js, receiveTime);

}
// 启动服务器
void ChatServer::start()
{
	server_.start();
}
