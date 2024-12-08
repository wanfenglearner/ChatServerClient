
#ifndef _CHAT_CLIENT_H_
#define _CHAT_CLIENT_H_

#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <vector>
#include <json.hpp>
#include <atomic>
#include <functional>
#include <thread>
#include <unordered_map>
#include "user.h"
#include "group.h"
#include "public.h"
#include <cstring>

using Json = nlohmann::json;

class ChatClient
{
public:
	ChatClient() = default;
	// 构造函数
	ChatClient(std::string ip  , int port);
	// 析构
	~ChatClient();
	// 连接服务器
	bool connect();
	// 登录
	void login();
	// 注册
	void reg();
	
private:
	// 登录响应的处理函数
	void doLogResponse(Json& js);
	// 注册响应的处理函数
	void doRegResponse(Json& js);
	
private:
	// 专门用来处理接受的函数
	void recvTask();		
private:
	std::string ip_;		// 服务器ip
	int port_;			// 端口号
	int cfd_;		// 连接的通信描述
	
	sem_t rwsem_;

	User user_;		// 当前用户的信息
	std::vector<User> friends_;	// 当前用户的朋友
	std::vector<Group> group_;	// 当前用户的群组

	std::atomic_bool isLoginSuccess_;	// 是否登录成功
	bool isMainMenu_;
	
};



#endif

