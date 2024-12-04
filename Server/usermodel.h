
/*
*	操作user表中的类
*/

#ifndef _USER_MODEL_H_
#define _USER_MODEL_H_

#include "user.h"

class UserModel
{
public:

	// 向user表中添加元素
	bool insert(User& user);
	// 根据id获取用户信息
	User query(int id);
	// 更新用户的状态信息
	bool updateState(User& user);
	// 重新更新user表中的在线状态
	void resetState();
private:

};


#endif 

