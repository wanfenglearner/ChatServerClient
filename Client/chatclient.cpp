
#include "chatclient.h"
#include <unordered_set>

// 构造函数
ChatClient::ChatClient(std::string ip, int port)
	:ip_(ip)
	,port_(port)
	,isLoginSuccess_(false)
	, isMainMenuRunning_(false)
{
	// 初始化信号量
	sem_init(&rwsem_, 0, 0);
	sem_init(&commandsem_, 0, 0);

	handlerCommamdMap_.insert({ "onechat" ,"一对一聊天 格式 onechat:toid:msg" });
	handlerCommamdMap_.insert({ "help" ,"查看当前支持的命令 格式 help" });
	handlerCommamdMap_.insert({ "addfriend" ,"添加朋友 格式 addfriend:friendid" });
	handlerCommamdMap_.insert({ "creategroup" ,"创建群组 格式 creategroup:groupname:groupdesc" });
	handlerCommamdMap_.insert({ "addgroup" ,"加入群组 格式 addgroup:groupid" });
	handlerCommamdMap_.insert({ "groupchat","群组聊天 格式 groupchat:groupid:msg" });
	handlerCommamdMap_.insert({ "loginout","注销 格式 loginout" });
	handlerCommamdMap_.insert({ "showuserdata","展示用户的个人信息 格式 showuserdata" });

	handlerMap_.insert({ "help" , std::bind(&ChatClient::help, this, std::placeholders::_1) });
	handlerMap_.insert({ "onechat" , std::bind(&ChatClient::onechat, this, std::placeholders::_1) });
	handlerMap_.insert({ "addfriend" , std::bind(&ChatClient::addfriend, this, std::placeholders::_1) });
	handlerMap_.insert({ "creategroup",std::bind(&ChatClient::creategroup, this, std::placeholders::_1) });
	handlerMap_.insert({ "addgroup" , std::bind(&ChatClient::addgroup, this, std::placeholders::_1) });
	handlerMap_.insert({ "groupchat" ,std::bind(&ChatClient::groupchat, this, std::placeholders::_1) });
	handlerMap_.insert({ "loginout" ,std::bind(&ChatClient::loginout, this, std::placeholders::_1) });
	handlerMap_.insert({ "showuserdata" ,std::bind(&ChatClient::showUserData, this, std::placeholders::_1) });

}

// 析构
ChatClient::~ChatClient()
{
	if (cfd_ != -1)
	{
		close(cfd_);
	}
	sem_destroy(&rwsem_);
	sem_destroy(&commandsem_);
}

// 连接服务器
bool ChatClient::connectServer()
{
	// 创建连接套接字
	cfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (cfd_ == -1)
	{
		std::cerr << "创建连接套接字失败" << std::endl;
		return false;
	}
	// 绑定服务器
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_);
	addr.sin_addr.s_addr = inet_addr(ip_.c_str());
	int res = connect(cfd_, (sockaddr*)&addr, sizeof(sockaddr));
	if (res == -1)
	{
		std::cerr << "连接服务器失败" << std::endl;
		return false;
	}

	// 启动接受线程进行接受
	std::thread t(std::bind(&ChatClient::recvTask, this));
	t.detach();

	return true;
}
// 登录
void ChatClient::login()
{
	// 输入 id + password

	int id;
	std::string password;
	std::cout << "输入账号-> ";
	std::cin >> id;
	std::cin.ignore(MaxCache, '\n');
	std::cout << "输入密码-> ";
	getline(std::cin, password);

	Json response;
	response["msgid"] = MSG_LOGIN;
	response["id"] = id;
	response["password"] = password;

	std::string buf = response.dump();

	bool ret = sendMsg(buf);
	if (!ret)
	{
		return;
	}
	sem_wait(&rwsem_);
	std::cout << "进入主菜单" << std::endl;
	if (isLoginSuccess_)
	{
		// 说明注册成功可以进入主菜单界面
		isMainMenuRunning_ = true;
		mainMenu();
	}
}
// 注册
void ChatClient::reg()
{
	// 输入name + password
	std::string name;
	std::string password;

	std::cout << "输入注册姓名-> ";
	getline(std::cin, name);
	std::cout << "输入注册密码-> ";
	getline(std::cin, password);
	
	Json request;
	request["msgid"] = MSG_REG;
	request["name"] = name;
	request["password"] = password;

	std::string buf = request.dump();

	bool ret = sendMsg(buf);
	if (!ret)
	{
		return;
	}
	// 阻塞等待
	sem_wait(&rwsem_);

}




// 专门用来处理接受的函数
void ChatClient::recvTask()
{
	while (1)
	{
		
		char buf[MaxCache] = { 0 };
		int len = recv(cfd_, buf, sizeof(buf), 0);
		if (len == -1 || len == 0)
		{
			exit(0);
		}
		Json jsreponse = Json::parse(buf);
		// 根据返回 msgid 不同调入不同的函数
		int msgid = jsreponse["msgid"].get<int>();
		switch (msgid)
		{
		case MSG_LOGIN_ACK:		// 登录响应消息
			doLogResponse(jsreponse);
			// 通知主线程已经完成登录的消息处理
			sem_post(&rwsem_);
			break;

		case MSG_REG_ACK:	// 注册响应消息
			doRegResponse(jsreponse);
			// 通知主线程已经完成注册的消息处理
			sem_post(&rwsem_);
			break;

		case MSG_ONE_CHAT:	// 一对一聊天
			doOneChat(jsreponse);
			break;

		case MSG_ONE_CHAT_ACK:	// 一对一聊天响应
			doOneChatResponse(jsreponse);
			break;
		case MSG_ADD_FRIEND:	// 添加朋友
			doAddFriend(jsreponse);
			break;
		case MSG_ADD_FRIEND_ACK:	// 添加朋友响应
			doAddFriendResponse(jsreponse);
			break;

		case MSG_CREATE_GROUP:	// 创建群组响应
			doCreateGroupResponse(jsreponse);
			break;

		case MSG_ADD_GROUP: // 添加群组
			doAddGroup(jsreponse);
			break;
		case MSG_ADD_GROUP_ACK: // 添加群组响应
			doAddGroupResponse(jsreponse);
			break;

		case MSG_GROUP_CHAT:	// 群组聊天响应
			doGroupChat(jsreponse);
			break;

		case MSG_GROUP_CHAT_ACK:	// 群组聊天响应
			doGroupChatResponse(jsreponse);
			break;
			
		case MSG_LOGINOUT:		// 注销
			doLoginout(jsreponse);
			break;
		case MSG_LOGINOUT_ACK:		// 注销响应
			doLoginoutResponse(jsreponse);
			break;
		case MSG_FRIENDS_LOGIN:		// 朋友上线的响应
			doFriendsLogin(jsreponse);
			break;
		}

	}
}

// 专门用来发送消息的函数
 bool ChatClient::sendMsg(std::string msg)
{
	int len = send(cfd_, msg.c_str(), strlen(msg.c_str()) + 1, 0);
	if (len == -1)
	{
		std::cout << "发送消息失败 " << msg << std::endl;
		return false;
	}
	else
	{
		std::cout << "发送消息成功 " << msg << std::endl;
		return true;
	}
}

// 获取当前的时间
std::string ChatClient::getCurTime()
{
	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm* ptm = localtime(&tt);
	char buf[MaxCache] = {0};
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
		(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
		(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
	return std::string(buf);
}

// -----------------响应的处理函数--------------------------

	// 登录响应的处理函数
void ChatClient::doLogResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		// 登录失败
		isLoginSuccess_ = false;
		std::cout << js["msgack"].get<std::string>() << std::endl;
		return;
	}
	std::cout << "登录成功" << std::endl;
	// 登录成功
	isLoginSuccess_ = true;

	// 记录当前的用户信息
	user_.setid(js["id"].get<int>());
	user_.setname(js["name"].get<std::string>());
	user_.setstate("online");

	// 查询有没有 离线消息
	if (js.contains("offlinemessage"))
	{
		std::cout << "************离线消息************" << std::endl;
		std::vector<std::string> vec = js["offlinemessage"];
		for (auto& msg : vec)
		{
			Json jsmsg = Json::parse(msg);
			int msgid = jsmsg["msgid"].get<int>();
			char buf[MaxCache] = { 0 };
			if (msgid == MSG_ONE_CHAT)
			{
				sprintf(buf, "个人消息:\n%s->id:%d 姓名:%s 消息:%s",
					jsmsg["time"].get<std::string>().c_str(),
					jsmsg["id"].get<int>(),
					jsmsg["name"].get<std::string>().c_str(),
					jsmsg["msg"].get<std::string>().c_str()
				);
				std::cout << buf << std::endl;
			}
			else
			{
				sprintf(buf, "群消息:\n%s->群号:%d id:%d 姓名:%s 消息:%s",
					jsmsg["time"].get<std::string>().c_str(),
					jsmsg["groupid"].get<int>(),
					jsmsg["id"].get<int>(),
					jsmsg["name"].get<std::string>().c_str(),
					jsmsg["msg"].get<std::string>().c_str()
				);
				std::cout << buf << std::endl;
			}

		}
		std::cout << "*****************************" << std::endl;
	}
	// 是否有 friends
	if (js.contains("friends"))
	{
		// 先初始化 friends_
		friends_.clear();

		// 存储当前的朋友信息
		std::vector<std::string> vec = js["friends"];
		for (auto& pfriend : vec)
		{
			Json jsfriend = Json::parse(pfriend);
			User user;
			user.setid(jsfriend["id"].get<int>());
			user.setname(jsfriend["name"]);
			user.setstate(jsfriend["state"]);
			friends_.push_back(user);
		}
	}
	// 是否包含群组信息
	if (js.contains("group"))
	{
		group_.clear();
		std::vector<std::string> vec = js["group"];
		for (auto& pgroup : vec)
		{
			Json jsgroup = Json::parse(pgroup);
			Group group;
			group.setid(jsgroup["groupid"].get<int>());
			group.setname(jsgroup["groupname"]);
			group.setdesc(jsgroup["groupdesc"]);
			// 该群组的成员信息
			std::vector<std::string> vec2 = jsgroup["groupuser"];
			for (auto& pguser : vec2)
			{
				Json jsguser = Json::parse(pguser);
				GroupUser guser;
				guser.setid(jsguser["id"].get<int>());
				guser.setname(jsguser["name"]);
				guser.setRole(jsguser["role"]);
				guser.setstate(jsguser["state"]);

				group.getusers().push_back(guser);
			}
			group_.push_back(group);
		}
	}

}
// 注册响应的处理函数
void ChatClient::doRegResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
		return;
	}

	char buf[MaxCache] = { 0 };
	sprintf(buf, "***********注册成功***********\n"
		"---------你的账号:%d 注册的姓名:%s---------"
		, js["id"].get<int>(), js["name"].get<std::string>().c_str());
	std::cout << buf << std::endl;
}

// 添加好友的处理函数
void ChatClient::doAddFriend(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s 添加的好友:%d",
			js["msgack"].get<std::string>().c_str(), js["friendid"].get<int>());
		std::cout << buf << std::endl;

		// 将用户信息进行更新
		User friuser;
		friuser.setid(js["friendid"].get<int>());
		friuser.setname(js["friendname"].get<std::string>());
		friuser.setstate(js["friendstate"].get<std::string>());
		friends_.push_back(friuser);
	}


}

// 添加好友响应的处理函数
void ChatClient::doAddFriendResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s 账号:%d 姓名:%s 添加了你",
			js["msgack"].get<std::string>().c_str(), 
			js["id"].get<int>(),
			js["name"].get<std::string>().c_str());
		std::cout << buf << std::endl;

		// 将用户信息进行更新
		User friuser;
		friuser.setid(js["id"].get<int>());
		friuser.setname(js["name"].get<std::string>());
		friuser.setstate(js["state"].get<std::string>());
		friends_.push_back(friuser);
	}


}

// 创建群组响应的处理函数
void ChatClient::doCreateGroupResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s 群号:%d, 群名:%s 群描述:%s",
			js["msgack"].get<std::string>().c_str(), 
			js["groupid"].get<int>(),
			js["groupname"].get<std::string>().c_str(),
			js["groupdesc"].get<std::string>().c_str()
		);
		std::cout << buf << std::endl;

		// 将添加群组的信息加入到 group_ 中
		Group group;
		group.setid(js["groupid"].get<int>());
		group.setname(js["groupname"].get<std::string>());
		group.setdesc(js["groupdesc"].get<std::string>());

		GroupUser guser;
		guser.setid(js["id"].get<int>());
		guser.setname(js["name"].get<std::string>());
		guser.setstate(js["state"].get<std::string>());
		guser.setRole(js["role"].get<std::string>());
		group.getusers().push_back(guser);

		group_.push_back(group);
	}

}

// 添加群组处理函数
void ChatClient::doAddGroup(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s 加入群号:%d",
			js["msgack"].get<std::string>().c_str(),
			js["groupid"].get<int>()
		);
		std::cout << buf << std::endl;
		
		// 将添加群组的信息加入到 group_ 中
		Group group;
		group.setid(js["groupid"].get<int>());
		group.setname(js["groupname"].get<std::string>());
		group.setdesc(js["groupdesc"].get<std::string>());

		std::vector<std::string> vecguser = js["groupuser"];
		for (auto& pguser : vecguser)
		{
			Json jsguser = Json::parse(pguser);
			GroupUser guser;
			guser.setid(jsguser["id"].get<int>());
			guser.setname(jsguser["name"].get<std::string>());
			guser.setstate(jsguser["state"].get<std::string>());
			guser.setRole(jsguser["role"].get<std::string>());
			group.getusers().push_back(guser);
		}

		group_.push_back(group);
	}

}

// 添加群组响应的处理函数
void ChatClient::doAddGroupResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s 群号:%d 账号:%d 姓名:%s 加入了群",
			js["msgack"].get<std::string>().c_str(),
			js["groupid"].get<int>(),
			js["id"].get<int>(),
			js["name"].get<std::string>().c_str()
		);
		std::cout << buf << std::endl;

		// 找到这个群, 然后将加入者的信息添加到即可
		Group group;
		group.setid(js["groupid"].get<int>());
		group.setname(js["groupname"].get<std::string>());
		group.setdesc(js["groupdesc"].get<std::string>());

		auto it = std::find_if(group_.begin(), group_.end(), [=](Group& g1)
			{
				return g1.getid() == group.getid()
					&& g1.getname() == group.getname()
					&& g1.getdesc() == group.getdesc();
			});
		if (it != group_.end())
		{
			GroupUser gruser;
			gruser.setid(js["id"].get<int>());
			gruser.setname(js["name"].get<std::string>());
			gruser.setstate(js["state"].get<std::string>());
			gruser.setRole(js["role"].get<std::string>());

			it->getusers().push_back(gruser);
		}
	}

}

// 群组聊天的处理函数
void ChatClient::doGroupChat(Json& js)
{
	char buf[MaxCache] = { 0 };
	sprintf(buf, "群消息:\n%s->群号:%d id:%d 姓名:%s 消息:%s",
		js["time"].get<std::string>().c_str(),
		js["groupid"].get<int>(),
		js["id"].get<int>(),
		js["name"].get<std::string>().c_str(),
		js["msg"].get<std::string>().c_str()
	);
	std::cout << buf << std::endl;
}

// 群组聊天响应的处理函数
void ChatClient::doGroupChatResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s %-10s 向群号:%d 发的消息:%s",
			js["msgack"].get<std::string>().c_str(),
			js["time"].get<std::string>().c_str(),
			js["groupid"].get<int>(),
			js["msg"].get<std::string>().c_str()
		);
		std::cout << buf << std::endl;
	}
}

void ChatClient::doOneChat(Json& js)
{
	char buf[MaxCache] = { 0 };
	sprintf(buf, "个人消息:\n%s->id:%d 姓名:%s 消息:%s",
		js["time"].get<std::string>().c_str(),
		js["id"].get<int>(),
		js["name"].get<std::string>().c_str(),
		js["msg"].get<std::string>().c_str()
	);
	std::cout << buf << std::endl;
}

// 一对一聊天响应的处理函数
void ChatClient::doOneChatResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		std::cout << js["msgack"] << std::endl;
	}
	else
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-10s %-10s 向账号:%d 发的消息:%s",
			js["msgack"].get<std::string>().c_str(),
			js["time"].get<std::string>().c_str(),
			js["toid"].get<int>(),
			js["msg"].get<std::string>().c_str()
		);
		std::cout << buf << std::endl;
	}
}

// 注销响应的处理函数
void ChatClient::doLoginout(Json& js)
{
	if (js["errno"].get<int>() == 0)
	{
		isMainMenuRunning_ = false;

	}
}



void ChatClient::doLoginoutResponse(Json& js)
{
	char buf[MaxCache] = { 0 };

	int id = js["id"].get<int>();

	// 取好友列表寻找这个下线id
	for (auto& it : friends_)
	{
		if (it.getid() == id )
		{
			sprintf(buf, "账号:%d 好友:%s 下线", id, it.getname().c_str());
			std::cout << buf << std::endl;
			it.setstate("offline");
		}
	}
	// 取群友列表寻找这个下线id
	for (auto it = group_.begin(); it != group_.end(); ++it)
	{
		for (auto& pit : it->getusers())
		{
			if (pit.getid() == id )
			{
				sprintf(buf, "群号:%d 账号:%d 群友:%s 下线", it->getid(), id, pit.getname().c_str());
				std::cout << buf << std::endl;
				pit.setstate("offline");
			}
		}
	}
	
}

// 处理朋友上线的响应消息
void ChatClient::doFriendsLogin(Json& js)
{
	char buf[MaxCache] = { 0 };
	int id = js["id"].get<int>();
	// 取好友列表寻找这个上线id
	for (auto& it : friends_)
	{
		if (it.getid() == id)
		{
			sprintf(buf, "账号:%d 好友:%s 上线", id, it.getname().c_str());
			std::cout << buf << std::endl;
			it.setstate("online");
		}
	}
	// 取群友列表寻找这个下线id
	for (auto it = group_.begin(); it != group_.end(); ++it)
	{

		for (auto& pit : it->getusers())
		{
			if (pit.getid() == id )
			{
				sprintf(buf, "群号:%d 账号:%d 群友:%s 上线", it->getid(), id, pit.getname().c_str());
				std::cout << buf << std::endl;
				pit.setstate("online");
			}
		}
	}
}

//--------------------主菜单操作函数----------------



// 主菜单页面
void ChatClient::mainMenu()
{
	
	help();
	while (isMainMenuRunning_)
	{
		// 输入具体的命令进行操作
		std::string commandbuf;
		getline(std::cin, commandbuf);
		std::string command;
		int index = commandbuf.find(":");
		if (index == -1)
		{
			command = commandbuf;
		}
		else
		{
			command = commandbuf.substr(0, index);
		}
		// 查找是否存在该命令
		auto it = handlerMap_.find(command);
		if (it != handlerMap_.end())
		{
			it->second(commandbuf.substr(index + 1));
		}
		else
		{
			std::cout << "无效的命令" << std::endl;
		}
		
		
	}
}
// 查看当前支持的命令
void ChatClient::help(std::string) 
{
	std::cout << "-------------帮助文档-------------" << std::endl;
	for (auto& p : handlerCommamdMap_)
	{
		char buf[MaxCache] = { 0 };
		sprintf(buf, "%-20s %-30s", p.first.c_str(), p.second.c_str());
		std::cout << buf << std::endl;
	}
	std::cout << std::endl;
	std::cout << "---------------------------------" << std::endl;


}
// 一对一聊天
void ChatClient::onechat(std::string s)
{
	int index = s.find(':');
	if (index == -1)
	{
		std::cout << "输入的命令错误 " << s << std::endl;
		return;
	}
	int toid = std::stoi(s.substr(0, index));
	std::string msg = s.substr(index + 1);

	// 拼接json解析字符串
	Json request;
	request["msgid"] = MSG_ONE_CHAT;
	request["name"] = user_.getname();
	request["id"] = user_.getid();
	request["toid"] = toid;
	request["msg"] = msg;
	request["time"] = getCurTime();

	sendMsg(request.dump());


}
// 添加朋友
void ChatClient::addfriend(std::string s)
{
	int friendid = std::stoi(s);
	Json request;
	request["msgid"] = MSG_ADD_FRIEND;
	request["id"] = user_.getid();
	request["name"] = user_.getname();
	request["friendid"] = friendid;
	
	
	sendMsg(request.dump());


}
// 创建群组
void ChatClient::creategroup(std::string s)
{
	int index = s.find(":");
	
	if (index == -1)
	{
		std::cout << "输入的命令错误 " << s << std::endl;
		return;
	}
	std::string groupname = s.substr(0, index);
	std::string groupdesc = s.substr(index + 1);

	Json response;
	response["msgid"] = MSG_CREATE_GROUP;
	response["id"] = user_.getid();
	response["name"] = user_.getname();
	response["groupname"] = groupname;
	response["groupdesc"] = groupdesc;

	sendMsg(response.dump());

}
// 添加群组
void ChatClient::addgroup(std::string s)
{
	int groupid = std::stoi(s);

	Json response;
	response["msgid"] = MSG_ADD_GROUP;
	response["id"] = user_.getid();
	response["name"] = user_.getname();
	response["groupid"] = groupid;

	sendMsg(response.dump());

}
// 群组聊天
void ChatClient::groupchat(std::string s)
{
	int index = s.find(":");
	if (index == -1)
	{
		std::cout << "输入的命令错误 " << s << std::endl;
		return;
	}
	int groupid = std::stoi(s.substr(0, index));
	std::string msg = s.substr(index + 1);

	Json response;
	response["msgid"] = MSG_GROUP_CHAT;
	response["id"] = user_.getid();
	response["name"] = user_.getname();
	response["groupid"] = groupid;
	response["msg"] = msg;
	response["time"] = getCurTime();

	sendMsg(response.dump());



}
// 注销登录
void ChatClient::loginout(std::string s)
{
	Json response;
	response["msgid"] = MSG_LOGINOUT;
	response["id"] = user_.getid();
	response["name"] = user_.getname();

	sendMsg(response.dump());

}
// 显示当前账号信息
void ChatClient::showUserData(std::string s)
{


	char buf[MaxCache] = { 0 };
	std::cout << "*************个人信息*************" << std::endl;
	sprintf(buf, "-------账号:%d 姓名:%s-------", user_.getid(), user_.getname().c_str());
	std::cout << buf << std::endl;

	std::cout << "--------------好友信息--------------" << std::endl;
	if (!friends_.empty())
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%-10s %-10s %-10s", "账号", "姓名", "状态");
		std::cout << buf << std::endl;
		// 输出朋友的信息
		for (auto& user : friends_)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%-10d %-10s %-10s",
				user.getid(), user.getname().c_str(), user.getstate().c_str());
			std::cout << buf << std::endl;
		}
	}
	std::cout << "---------------------------------" << std::endl;
	std::cout << "--------------群组信息--------------" << std::endl;
	if (!group_.empty())
	{
		
		for (auto& group : group_)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, "群号:%-10d 群名:%-10s 群描述:%-20s",
					group.getid(), group.getname().c_str(), group.getdesc().c_str());
			std::cout << buf << std::endl;
			for (auto& gruser : group.getusers())
			{
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "成员账号:%-10d 成员姓名:%-10s 成员状态:%-5s 角色:%-6s",
						gruser.getid(), gruser.getname().c_str(), 
					gruser.getstate().c_str(), gruser.getRole().c_str());
				std::cout << buf << std::endl;
			}
		}
	}
	std::cout << "---------------------------------" << std::endl;
	std::cout << "*********************************" << std::endl;


}