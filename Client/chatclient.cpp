#include "chatclient.h"


// 构造函数
ChatClient::ChatClient(std::string ip, int port)
	:ip_(ip)
	,port_(port)
	,isLoginSuccess_(false)
	, isMainMenu_(false)
{
	sem_init(&rwsem_, 0, 0);
}

// 析构
ChatClient::~ChatClient()
{
	if (cfd_ != -1)
	{
		close(cfd_);
	}
	sem_destroy(&rwsem_);
}

// 连接服务器
bool ChatClient::connect()
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
	int res = bind(cfd_, (sockaddr*)&addr, sizeof(addr));
	if (res == -1)
	{
		std::cerr << "绑定服务器失败" << std::endl;
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
	
	std::cin >> id;
	std::cin.get();

	getline(std::cin, password);

	Json response;
	response["id"] = id;
	response["password"] = password;

	std::string buf = response.dump();

	int ret = send(cfd_, buf.c_str(), strlen(buf.c_str()) + 1, 0);
	if (ret == -1)
	{
		std::cerr << "发送登录数据失败-> " << buf << std::endl;
		return;
	}

	sem_wait(&rwsem_);
	
}
// 注册
void ChatClient::reg()
{
	// 输入name + password
	std::string name;
	std::string password;
	getline(std::cin, name);
	getline(std::cin, password);
	
	Json request;
	request["name"] = name;
	request["password"] = password;

	std::string buf = request.dump();

	int ret = send(cfd_, buf.c_str(), strlen(buf.c_str()) + 1, 0);
	if (ret == -1)
	{
		std::cerr << "发送注册数据失败-> " << buf << std::endl;
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
		char buf[1024] = { 0 };
		int len = recv(cfd_, buf, sizeof(buf), 0);
		if (len == -1 || len == 0)
		{
			exit(0);
		}
		Json jsreponse = Json::parse(buf);
		// 根据返回 msgid 不同调入不同的函数
		int msgid = jsreponse["msgid"].get<int>();
		if (msgid == MSG_ONE_CHAT)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "个人消息:\n%s->id:%d 姓名:%s 消息:%s",
				jsreponse["time"].get<std::string>().c_str(),
				jsreponse["id"].get<int>(),
				jsreponse["name"].get<std::string>().c_str(),
				jsreponse["msg"].get<std::string>().c_str()
				);
			std::cout << buf << std::endl;
			continue;
		}
		if (msgid == MSG_GROUP_CHAT)
		{
			char buf[1024] = { 0 };
			sprintf(buf, "群消息:\n%s->群号:%d id:%d 姓名:%s 消息:%s",
				jsreponse["time"].get<std::string>().c_str(),
				jsreponse["groupid"].get<int>(),
				jsreponse["id"].get<int>(),
				jsreponse["name"].get<std::string>().c_str(),
				jsreponse["msg"].get<std::string>().c_str()
			);
			std::cout << buf << std::endl;
			continue;
		}
		if (msgid == MSG_REG_ACK)
		{
			doRegResponse(jsreponse);
			// 通知主线程已经完成注册的消息处理
			sem_post(&rwsem_);
			continue;
		}
		if (msgid == MSG_LOGIN_ACK)
		{
			doLogResponse(jsreponse);
			// 通知主线程已经完成登录的消息处理
			sem_post(&rwsem_);
			continue;
		}
	}
}





// -----------------响应的处理函数--------------------------

	// 登录响应的处理函数
void ChatClient::doLogResponse(Json& js)
{
	if (js["errno"].get<int>() != 0)
	{
		// 登录失败
		isLoginSuccess_ = false;
		std::cout << js["errmsg"].get<std::string>() << std::endl;
		return;
	}
	// 登录成功
	isLoginSuccess_ = true;

	// 记录当前的用户信息
	user_.setid(js["id"].get<int>());
	user_.setname(js["name"].get<std::string>());
	user_.setstate("online");

	// 查询有没有 离线消息
	if (js.contains("offlinemessage"))
	{
		std::vector<std::string> vec = js["offlinemessage"];
		for (auto& msg : vec)
		{
			Json jsmsg = Json::parse(msg);
			int msgid = jsmsg["msgid"].get<int>();
			char buf[1024] = { 0 };
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


}