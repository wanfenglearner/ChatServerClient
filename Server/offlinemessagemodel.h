
/*
	操作数据库中的 offlinemessage 表
*/

#ifndef _OFFLINMESSAGEMODEL_H_
#define _OFFLINMESSAGEMODEL_H_
#include <string>
#include <vector>
class OfflineMsgModel
{
public:
	// 存储用户id的离线消息
	void insert(int id, std::string msg);
	// 删除用户的离线消息
	void remove(int id);
	// 查询用户的离线消息
	std::vector<std::string> query(int id);

private:
};

#endif
