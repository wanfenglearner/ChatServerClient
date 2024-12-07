#ifndef _CHATSERVICE_H_
#define _CHATSERVICE_H_

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/base/LogStream.h>
#include <functional>
#include <mutex>
#include <string>
#include <iostream>
#include <json.hpp>
#include <unordered_map>
#include "public.h"
#include "offlinemessagemodel.h"
#include "usermodel.h"
#include "friendmodel.h"
#include "groupmodel.h"
// 使用命名空间包含 json
using Json = nlohmann::json;

// 处理消息回调的函数类型
using MyHandler = std::function<void(const muduo::net::TcpConnectionPtr&, Json&, muduo::Timestamp)>;

class ChatService
{
public:
	// 得到单例对象
	static ChatService* instance();
	// 登录
	void login(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 注销
	void loginout(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 注册
	void reg(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 一对一聊天
	void oneChat(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 处理用户下线
	void clientCloseException(const muduo::net::TcpConnectionPtr& conn);
	// 从消息js["msgid"]id中得到对应的处理函数handle
	MyHandler getHandler(int msgid);

	// 服务器异常终止处理用户的在线状态
	void reset();
	// 添加好友
	void addFriend(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 删除好友
	void deleteFriend(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 创建群组
	void createGroup(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 添加群组
	void addGroup(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTime);
	// 群组聊天
	void chatGroup(const muduo::net::TcpConnectionPtr& conn,
		Json& js,
		muduo::Timestamp receiveTim);
private:
	// 构造函数
	ChatService();
private:
	std::unordered_map<int, MyHandler> myHandleMap_;// 根据msgID找到对应的处理函数

	// 存储每个用户对应的连接
	std::unordered_map<int, const muduo::net::TcpConnectionPtr&> connMap_;
	// 互斥锁, 保证在多线程下connMap_ 安全
	std::mutex connMutex_;

	// 操作数据库 user 表
	UserModel userModel_;
	// 操作数据库 offlinemessage 表
	OfflineMsgModel offlineMsgModel_;
	// 操作数据库 friend 表
	FriendModel friendModel_;
	// 操作数据库 allgroup 和 groupuser
	GroupModel groupModel_;
};

#endif
