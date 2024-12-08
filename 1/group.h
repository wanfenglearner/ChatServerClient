
/*
*	数据库中的  allgroup 表
*/
#ifndef _GROUP_H_
#define _GROUP_H_

#include <string>
#include <vector>
#include "groupuser.h"

class Group
{
public:
	Group(int id = -1, std::string name = "", std::string desc = "")
		:id_(id), name_(name), desc_(desc)
	{}

	// 设置群信息
	void setid(int id) { id_ = id; }
	void setname(std::string name) { name_ = name; }
	void setdesc(std::string desc) { desc_ = desc; }

	// 得到群信息
	int getid() const { return id_; }
	std::string getname() const { return name_; }
	std::string getdesc() const { return desc_; }
	std::vector<GroupUser>& getusers() { return users_; }


private:
	int id_;	// 群组id
	std::string name_;	// 群名
	std::string desc_;	// 群的描述
	std::vector<GroupUser> users_;	// 群里的成员
};



#endif