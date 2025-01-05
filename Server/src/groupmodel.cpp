#include "groupmodel.h"
#include "database.h"
#include <cstring>

// 创建群组
bool GroupModel::createGroup(Group& group)
{
	char sql[1024] = { 0 };
	sprintf(sql, "insert into allgroup(groupname, groupdesc)"
		"values('%s','%s')", group.getname().c_str(), group.getdesc().c_str());

	MySQL mysql;
	if (mysql.connect())
	{
		if (mysql.update(sql))
		{
			group.setid((int)mysql_insert_id(mysql.getConnect()));
			return true;
		}
	}
	return false;
}
// 添加群组
void GroupModel::addGroup(int userid, int groupid, std::string role)
{
	char sql[1024] = { 0 };
	sprintf(sql, "insert into groupuser values(%d, %d, '%s')", groupid, userid, role.c_str());

	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}

}
// 查询群组信息
std::vector<Group> GroupModel::query(int userid)
{
	char sql[1024] = { 0 };
	sprintf(sql, "select a.id, a.groupname, a.groupdesc from allgroup a \
		inner join groupuser b on a.id = b.groupid where userid = %d", userid);

	std::vector<Group> vecgroup;
	MySQL mysql;
	if (mysql.connect())
	{
		// 先查询该用户的所有群组信息

		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				Group group;
				group.setid(atoi(row[0]));
				group.setname(row[1]);
				group.setdesc(row[2]);
				vecgroup.push_back(group);
			}
			mysql_free_result(res);
		}
	}

	// 再查新每个群组中的用户信息
	for (auto& group : vecgroup)
	{
		memset(sql, 0, sizeof(sql));
		sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a \
			inner join groupuser b on a.id = b.userid where groupid = %d", group.getid());
		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				GroupUser guser;
				guser.setid(atoi(row[0]));
				guser.setname(row[1]);
				guser.setstate(row[2]);
				guser.setRole(row[3]);
				group.getusers().push_back(guser);
			}
			mysql_free_result(res);
		}
	}

	return vecgroup;
}
// 得到具体的群中的好友id
std::vector<int> GroupModel::getCurGroupUsers(int userid, int groupid)
{
	char sql[1024] = { 0 };
	sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d",
		groupid, userid);

	std::vector<int> vecid;
	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				vecid.push_back(atoi(row[0]));
			}

			mysql_free_result(res);
		}
	}
	return vecid;
}
// 得到具体的群中的信息
Group GroupModel::getgetCurGroupData(int groupid)
{
	char sql[1024] = { 0 };
	sprintf(sql, "select groupname, groupdesc from allgroup where id = %d", groupid);

	Group group;
	group.setid(groupid);

	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			group.setname(row[0]);
			group.setdesc(row[1]);
		}
		mysql_free_result(res);

		memset(sql, 0, sizeof(sql));
		sprintf(sql, "select a.id, a.name, a.state, b.grouprole from user a \
			inner join groupuser b on a.id = b.userid where groupid = %d", group.getid());
		 res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				GroupUser guser;
				guser.setid(atoi(row[0]));
				guser.setname(row[1]);
				guser.setstate(row[2]);
				guser.setRole(row[3]);
				group.getusers().push_back(guser);
			}
			mysql_free_result(res);
		}
	}
	
	return group;
}
