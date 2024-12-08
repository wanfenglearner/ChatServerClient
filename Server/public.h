#ifndef _PUBLIC_H_
#define _PUBLIC_H_

// 这里保存 Server 和Client的 公共消息规定格式

enum MSG_TYPE
{
	MSG_LOGIN = 1,			// 登录
	MSG_LOGIN_ACK,			// 登录响应
	MSG_LOGINOUT,			// 注销
	MSG_LOGINOUT_ACK,		// 注销响应
	MSG_REG,				// 注册
	MSG_REG_ACK,			// 注册响应
	MSG_ONE_CHAT,			// 一对一聊天
	MSG_ONE_CHAT_ACK,		// 一对一聊天响应
	MSG_ADD_FRIEND,			// 添加好友
	MSG_ADD_FRIEND_ACK,		// 添加好友响应
	MSG_DELETE_FRIEND,		// 删除好友
	MSG_GROUP_CHAT,			// 群组聊天
	MSG_GROUP_CHAT_ACK,		// 群组聊天响应
	MSG_ADD_GROUP,			// 加入群组 
	MSG_ADD_GROUP_ACK,		// 加入群组响应
	MSG_CREATE_GROUP		// 创建群组
	
};


#endif 

