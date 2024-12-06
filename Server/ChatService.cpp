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
	myHandleMap_.insert({ MSG_ADD_FRIEND, std::bind(&ChatService::addFriend, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_DELETE_FRIEND, std::bind(&ChatService::deleteFriend, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_CREATE_GROUP, std::bind(&ChatService::createGroup, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_ADD_GROUP, std::bind(&ChatService::addGroup, this,
	std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });
	myHandleMap_.insert({ MSG_GROUP_CHAT, std::bind(&ChatService::chatGroup, this,
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
			// 查询有没有离线消息
			std::vector<std::string> vec = offlineMsgModel_.query(id);
			if (!vec.empty())
			{
				// 有离线消息 删除存在的离线消息
				response["offlinemessage"] = vec;
				offlineMsgModel_.remove(id);
			}

			// 查询有没有好友信息
			std::vector<User> vecuser = friendModel_.query(id);
			if (!vecuser.empty())
			{
				// 得到该用户的好友列表
				std::vector<std::string>vec;
				for (auto& user : vecuser)
				{
					Json js;
					js["id"] = user.getid();
					js["name"] = user.getname();
					js["state"] = user.getstate();
					vec.push_back(js.dump());
				}
				response["friends"] = vec;
			}

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
		auto ittoid = connMap_.find(toid);
		auto itid = connMap_.find(id);
		if (itid == connMap_.end() || itid->second != conn)
		{
			return;
		}
		if (ittoid != connMap_.end())
		{
			// 找到了在线的对象
			// 把消息传出去
			ittoid->second->send(js.dump());
			return;
		}
	}

	// 存储离线消息
	offlineMsgModel_.insert(toid, js.dump());

}

// 服务器异常终止处理用户的在线状态
void ChatService::reset()
{
	userModel_.resetState();
}

// 添加好友
void ChatService::addFriend(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTime)
{
	// 用户id
	int id = js["id"].get<int>();
	int friendid = js["friendid"].get<int>();
	friendModel_.insert(id, friendid);
	friendModel_.insert(friendid, id);

}
// 删除好友
void ChatService::deleteFriend(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTime)
{
	int id = js["id"].get<int>();
	int friendid = js["friendid"].get<int>();
	friendModel_.remove(id, friendid);
	friendModel_.remove(friendid, id);
}

// 创建群组
void ChatService::createGroup(const muduo::net::TcpConnectionPtr& conn,Json& js,muduo::Timestamp receiveTime)
{
	std::string groupname = js["groupname"];
	std::string groupdesc = js["groupdesc"];
	int userid = js["id"].get<int>();

	Group group;
	group.setname(groupname);
	group.setdesc(groupdesc);
	if (groupModel_.createGroup(group))
	{
		// 将自己添加到群组之中 为 创建者
		groupModel_.addGroup(userid, group.getid(), "creator");
	}
	
}
// 添加群组
void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn,Json& js,muduo::Timestamp receiveTime)
{
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();
	groupModel_.addGroup(userid, groupid, "normal");
}

// 群组聊天
void ChatService::chatGroup(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTim)
{
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();

	std::vector<int> vecusers = groupModel_.getCurGroupUsers(userid, groupid);
	
	// 加锁
	std::unique_lock<std::mutex> lock(connMutex_);
	for (auto& id : vecusers)
	{
		auto it = connMap_.find(id);
		if (it != connMap_.end())
		{
			it->second->send(js.dump());
		}
		else
		{
			// 存储离线消息
			offlineMsgModel_.insert(id, js.dump());
		}
	}

}


