
/*
*	数据库中 user表
*/
#ifndef _USER_H_
#define _USER_H_

#include <string>

class User
{
public:
	User(int id = -1, std::string name = "", std::string password = "", std::string state = "offline")
		:id_(id)
		,name_(name)
		,password_(password)
		,state_(state)
	{}
	// 设置参数
	void setid(int id) { id_ = id; }

	void setname(std::string name) { name_ = name; }

	void setpassword(std::string password) { password_ = password; }

	void setstate(std::string state) { state_ = state; }

	// 获得参数
	int getid()const { return id_; }

	std::string getname()const { return name_; }

	std::string getpassword()const { return password_; }

	std::string getstate()const { return state_; }

private:
	int id_;				// id号
	std::string name_;		// 姓名
	std::string password_;	// 密码
	std::string state_;		// 状态
};

#endif


