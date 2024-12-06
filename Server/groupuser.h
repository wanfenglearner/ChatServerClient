
/*
*	记录每个群中的成员信息
*/
#ifndef _GROUP_USER_H_
#define _GROUP_USER_H_

#include "user.h"
#include <string>
class GroupUser : public User
{
public:
	
	void setRole(std::string role) { role_ = role; }
	std::string getRole() { return role_; };
private:
	// 多了一个在群中身份
	std::string role_;
};




#endif
