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

	myHandleMap_.insert({ MSG_CREATE_GROUP, std::bind(&ChatService::createGroup, this,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

	myHandleMap_.insert({ MSG_ADD_GROUP, std::bind(&ChatService::addGroup, this,
	std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

	myHandleMap_.insert({ MSG_GROUP_CHAT, std::bind(&ChatService::chatGroup, this,
	std::placeholders::_1, std::placeholders::_2, std::placeholders::_3) });

	myHandleMap_.insert({ MSG_LOGINOUT, std::bind(&ChatService::loginout, this,
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
			response["msgack"] = "账号已经登录,无需再次登录";
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
			response["id"] = user.getid();
			response["state"] = user.getstate();
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
			// 查询有没有群组消息
			std::vector<Group> vecgroup = groupModel_.query(id);
			if (!vecgroup.empty())
			{
				std::vector<std::string> vec;
				for (auto& group : vecgroup)
				{
					// 拼接群组消息
					Json jsgroup;
					jsgroup["groupid"] = group.getid();
					jsgroup["groupname"] = group.getname();
					jsgroup["groupdesc"] = group.getdesc();
					// 该群组的成员信息
					std::vector<std::string> vec2;
					for (auto& user : group.getusers())
					{
						Json jsuser;
						jsuser["id"] = user.getid();
						jsuser["name"] = user.getname();
						jsuser["state"] = user.getstate();
						jsuser["role"] = user.getRole();
						vec2.push_back(jsuser.dump());
					}
					jsgroup["groupuser"] = vec2;

					// 将本群组的信息进行打包
					vec.push_back(jsgroup.dump());
				}
				response["group"] = vec;
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
		response["msgack"] = "输入的账号或者密码错误";
		conn->send(response.dump());
	}

}
// 注销
void ChatService::loginout(const muduo::net::TcpConnectionPtr& conn,Json& js,muduo::Timestamp receiveTime)
{
	int id = js["id"].get<int>();
	std::vector<int>friendid = friendModel_.getId(id);
	std::vector<Group> groups = groupModel_.query(id);

	User users = userModel_.query(id);
	User user;
	js["name"] = users.getname();

	{
		std::unique_lock<std::mutex> lock(connMutex_);
		auto it = connMap_.find(id);
		if (it != connMap_.end())
		{
			connMap_.erase(it);

			js["errno"] = 0;
			js["msgack"] = "注销登录成功";
			conn->send(js.dump());
			user.setid(id);
		}
		// 查找该用户的所有朋友向他们发送下线消息
		for (auto& i : friendid)
		{
			auto it = connMap_.find(i);
			if (it != connMap_.end())
			{
				js["msgid"] = MSG_LOGINOUT_ACK;
				it->second->send(js.dump());
			}
		}
		// 查找该用户的所有群友向他们发送下线消息
		for (auto& pgroup : groups)
		{
			for (auto& puser : pgroup.getusers())
			{
				auto it = connMap_.find(puser.getid());
				std::cout << "***************************" << std::endl;
				//std::cout << it->first << std::endl;
				std::cout << "***************************" << std::endl;
				if (it != connMap_.end())
				{
					js["msgid"] = MSG_LOGINOUT_ACK;
					it->second->send(js.dump());
				}
			}
		}
	}

	if (user.getid() != -1)
	{
		userModel_.updateState(user);
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
		response["msgack"] = "注册成功";
		response["id"] = user.getid();
		response["name"] = user.getname();
		response["state"] = user.getstate();
		// 向客户端发送消息
		conn->send(response.dump());
	}
	else
	{
		// 注册失败
		Json response;
		response["msgid"] = MSG_REG_ACK;
		response["errno"] = -1;
		response["msgack"] = "注册失败";
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
	int toid = js["toid"].get<int>();
	Json response;
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
	offlineMsgModel_.insert(toid, js.dump());
	response["msgid"] = MSG_ONE_CHAT_ACK;
	response["errno"] = 0;
	response["toid"] = toid;
	response["time"] = js["time"];
	response["msgack"] = "发送消息成功";
	response["msg"] = js["msg"];

	conn->send(response.dump());
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
	
	User user = userModel_.query(id);
	User friuser = userModel_.query(friendid);

	js["errno"] = 0;
	js["msgack"] = "添加好友成功";

	js["state"] = user.getstate();
	js["name"] = user.getname();

	js["friendstate"] = friuser.getstate();
	js["friendname"] = friuser.getname();

	{
		std::unique_lock<std::mutex> lock(connMutex_);
		auto it = connMap_.find(friendid);
		if (it != connMap_.end())
		{
			js["msgid"] = MSG_ADD_FRIEND_ACK;
			it->second->send(js.dump());
		}
	}

	js["msgid"] = MSG_ADD_FRIEND;
	conn->send(js.dump());
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

		User user = userModel_.query(userid);

		js["errno"] = 0;
		js["msgack"] = "创建群组成功";
		js["groupid"] = group.getid();
		
		
		js["name"] = user.getname();
		js["state"] = user.getstate();
		js["role"] = std::string("creator");
	}
	else
	{
		js["errno"] = -1;
		js["msgack"] = "创建群组失败";
	}
	conn->send(js.dump());
}
// 添加群组
void ChatService::addGroup(const muduo::net::TcpConnectionPtr& conn,Json& js,muduo::Timestamp receiveTime)
{
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();
	groupModel_.addGroup(userid, groupid, "normal");

	User user =  userModel_.query(userid);

	std::vector<int> vec1 = groupModel_.getCurGroupUsers(userid, groupid);
	js["errno"] = 0;
	js["msgack"] = "添加群组成功";
	js["role"] = "normal";
	js["state"] = user.getstate();
	js["name"] = user.getname();
	// 要知道这个群的信息
	Group group = groupModel_.getgetCurGroupData(groupid);

	js["groupname"] = group.getname();
	js["groupdesc"] = group.getdesc();

	{
		std::unique_lock<std::mutex> lock(connMutex_);
		for (auto& i : vec1)
		{
			auto it = connMap_.find(i);
			if (it != connMap_.end())
			{
				js["msgid"] = MSG_ADD_GROUP_ACK;
				it->second->send(js.dump());
			}
		}
	}
	
	js["msgid"] = MSG_ADD_GROUP;

	std::vector<std::string> vec2;
	for (auto &user : group.getusers())
	{
		Json jsuser;
		jsuser["id"] = user.getid();
		jsuser["name"] = user.getname();
		jsuser["state"] = user.getstate();
		jsuser["role"] = user.getRole();

		vec2.push_back(jsuser.dump());
	}
	js["groupuser"] = vec2;

	conn->send(js.dump());
}

// 群组聊天
void ChatService::chatGroup(const muduo::net::TcpConnectionPtr& conn, Json& js, muduo::Timestamp receiveTim)
{
	int userid = js["id"].get<int>();
	int groupid = js["groupid"].get<int>();

	Json response;
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

	response["msgid"] = MSG_GROUP_CHAT_ACK;
	response["id"] = userid;
	response["msg"] = js["msg"];
	response["groupid"] = groupid;
	response["errno"] = 0;
	response["time"] = js["time"];
	response["msgack"] = "群聊消息成功";

	conn->send(response.dump());
}


