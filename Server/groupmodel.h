
/*
*	操作数据库中的  allgroup 和 groupuser
*/

#ifndef _GROUP_MODEL_H_
#define _GROUP_MODEL_H_

#include "group.h"
#include <vector>

class GroupModel
{
public:
	// 创建群组
	bool createGroup(Group& group);

	// 添加群组
	void addGroup(int userid, int groupid, std::string role);

	// 查询群组信息
	std::vector<Group> query(int userid);

	// 得到具体的群中的好友id
	std::vector<int> getCurGroupUsers(int userid, int groupid);

private:


};



#endif





