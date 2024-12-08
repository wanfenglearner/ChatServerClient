
/*
	操作数据库中的 friend 表
*/
#ifndef _FRIEND_MODEL_H_
#define _FRIEND_MODEL_H_

#include "user.h"
#include <vector>

class FriendModel
{
public:
	// 添加好友
	void insert(int userid, int friendid);

	// 获得好友的信息
	std::vector<User> query(int id);


};

#endif