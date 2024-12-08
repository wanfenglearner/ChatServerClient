
#ifndef _CHAT_CLIENT_H_
#define _CHAT_CLIENT_H_

#include <iostream>
#include <string>
#include <time.h>
#include <chrono>
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

// 在主菜单中不同的命令对应不同的处理函数
using Handler = std::function<void(std::string)>;
class ChatClient
{
public:
	ChatClient() = default;
	// 构造函数
	ChatClient(std::string ip  , int port);
	// 析构
	~ChatClient();
	// 连接服务器
	bool connectServer();
	// 登录
	void login();
	// 注册
	void reg();
	
private:
	// 登录响应的处理函数
	void doLogResponse(Json& js);

	// 注册响应的处理函数
	void doRegResponse(Json& js);

	// 添加好友的处理函数
	void doAddFriend(Json& js);
	// 添加好友响应的处理函数
	void doAddFriendResponse(Json& js);

	// 创建群组响应的处理函数
	void doCreateGroupResponse(Json& js);

	// 添加群组处理函数
	void doAddGroup(Json& js);
	// 添加群组响应的处理函数
	void doAddGroupResponse(Json& js);

	// 群组聊天的处理函数
	void doGroupChat(Json& js);
	// 群组聊天响应的处理函数
	void doGroupChatResponse(Json& js);

	// 一对一聊天的处理函数
	void doOneChat(Json& js);
	// 一对一聊天响应的处理函数
	void doOneChatResponse(Json& js);

	// 注销响应的处理函数
	void doLoginouttResponse(Json& js);
	
private:
	// 主菜单页面
	void mainMenu();
	// 显示当前账号信息
	void showUserData(std::string s = "");
	// 查看当前支持的命令
	void help(std::string s = "");
	// 一对一聊天
	void onechat(std::string s);
	// 添加朋友
	void addfriend(std::string s);
	// 创建群组
	void creategroup(std::string s);
	// 添加群组
	void addgroup(std::string s);
	// 群组聊天
	void groupchat(std::string s);
	// 注销登录
	void loginout(std::string s);

private:
	// 专门用来处理接受的函数
	void recvTask();	
	// 专门用来发送消息的函数
	bool sendMsg(std::string msg);
	// 获取当前的时间
	std::string getCurTime();
private:
	std::string ip_;		// 服务器ip
	int port_;			// 端口号
	int cfd_;		// 连接的通信描述
	
	sem_t rwsem_;

	User user_;		// 当前用户的信息
	std::vector<User> friends_;	// 当前用户的朋友
	std::vector<Group> group_;	// 当前用户的群组

	std::atomic_bool isLoginSuccess_;	// 是否登录成功

	bool isMainMenuRunning_;	// 主菜单是否运行
	// 支持的命令
	std::unordered_map<std::string, std::string> handlerCommamdMap_;
	// 不同的命令对应不同的处理函数
	std::unordered_map<std::string, Handler> handlerMap_;
};



#endif

