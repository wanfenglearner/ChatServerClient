#include "ChatService.h"
#include "user.h"


// 构造函数
ChatService::ChatService()
{
	// 注册消息的回调函数
	myHandleMap_.insert({ MSG_REG, std::bind(&ChatService::reg, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_LOGIN, std::bind(&ChatService::login, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_ONE_CHAT, std::bind(&ChatService::oneChat, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

}
// 得到单例对象
ChatService* ChatService::instance()
{
	// 线程安全
	static ChatService service;
	return &service;
}
// 从消息js["msgid"]id中得到对应的处理函数handle
MyHandler ChatService::getHandler(int msgid)
{
	auto it = myHandleMap_.find(msgid);
	if (it == myHandleMap_.end())
	{
		// 说明没有这个操作指令
		return [&](const muduo::net::TcpConnectionPtr& conn,
			Json& js,
			muduo::Timestamp receiveTime)->void
			{
				LOG_ERROR << msgid << "没有相应的事件回调函数";
			};
	}
	else
	{
		return myHandleMap_[msgid];
	}
}
// 登录 id + password
void ChatService::login(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTime)
{
	// 获得客户端的信息
	int id = js["id"].get<int>();
	std::string pwd = js["password"];

	User user = userModel_.query(id);
	if (user.getid() == id && user.getpassword() == pwd)
	{
		// 输入的信息正确
		if (user.getstate() == "online")
		{
			// 账号已经登录了, 不能重复登录

			// 打包响应数据
			Json response;
			response["msgid"] = MSG_LOGIN_ACK;
			response["errno"] = 1;
			response["errmsg"] = "账号已经登录,无需再次登录";
			conn->send(response.dump());
		}
		else
		{
			// 第一次登录
			// 把当前用户的状态改为在线
			user.setstate("online");
			userModel_.updateState(user);

			// 查询有没有离线消息
			std::vector<std::string> vec = offlineModel_.query(id);
			

			// 记录此用户对应的连接
			{
				// 因为connMap_不是线程安全的, 需要加锁
				std::unique_lock<std::mutex> lock(connMutex_);
				connMap_.insert({ id, conn });
			}

			// 打包响应数据
			Json response;
			response["msgid"] = MSG_LOGIN_ACK;
			response["errno"] = 0;
			response["name"] = user.getname();
			response["offlinemessage"] = vec;
			conn->send(response.dump());
		}
	}
	else
	{
		// 输入的信息失败
		// 打包响应数据
		Json response;
		response["msgid"] = MSG_LOGIN_ACK;
		response["errno"] = -1;
		response["errmsg"] = "输入的账号或者密码错误";
		conn->send(response.dump());
	}

}

// 注册
void ChatService::reg(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTime)
{
	// 获得客户端的注册消息
	std::string name = js["name"];
	std::string pwd = js["password"];

	User user;
	user.setname(name);
	user.setpassword(pwd);

	// 向数据库中插入该注册用户信息
	bool state = userModel_.insert(user);
	if (state)
	{
		// 注册成功
		Json response;
		response["msgid"] = MSG_REG_ACK;
		response["errno"] = 0;
		response["errmsg"] = "注册成功";
		response["id"] = user.getid();
		// 向客户端发送消息
		conn->send(response.dump());
	}
	else
	{
		// 注册失败
		Json response;
		response["msgid"] = MSG_REG_ACK;
		response["errno"] = -1;
		response["errmsg"] = "注册失败";
		// 向客户端发送消息
		conn->send(response.dump());
	}

}
// 处理用户下线
void ChatService::clientCloseException(const muduo::net::TcpConnectionPtr& conn)
{
	User user;
	{
		// 操作 connMap_ 需要加锁
		std::unique_lock<std::mutex> lock(connMutex_);
		for (auto it = connMap_.begin(); it != connMap_.end(); ++it)
		{
			if (it->second == conn)
			{
				// 找到断开链接的用户
				// 更新此用户的信息
				user.setid(it->first);
				connMap_.erase(it);
				break;
			}
		}
	}
	if (user.getid() != -1)
	{
		// 成功查找
		user.setstate("offline");
		userModel_.updateState(user);
	}

}

// 一对一聊天
void ChatService::oneChat(const muduo::net::TcpConnectionPtr& conn,Json& js,muduo::Timestamp receiveTime)
{
	// 获得要聊天的对象
	int toid = js["to"].get<int>();
	int id = js["id"].get<int>();
	if (toid == id)
	{
		// 不能自己给自己发消息
		return;
	}
	{
		std::unique_lock<std::mutex> lock(connMutex_);
		auto it = connMap_.find(toid);
		if (it != connMap_.end())
		{
			// 找到了在线的对象
			// 把消息传出去
			it->second->send(js.dump());
			return;
		}
	}

	// 存储离线消息
	offlineModel_.insert(toid, js.dump());

}









