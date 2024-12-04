#ifndef _PUBLIC_H_
#define _PUBLIC_H_

// 这里保存 Server 和Client的 公共消息规定格式

enum MSG_TYPE
{
	MSG_LOGIN = 1,	// 登录消息
	MSG_LOGIN_ACK,	// 登录响应消息
	MSG_REG,		// 注册消息
	MSG_REG_ACK,	// 注册响应消息
	MSG_ONE_CHAT	// 一对一聊天消息

};


#endif 

