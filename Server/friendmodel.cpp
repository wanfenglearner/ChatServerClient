#include "friendmodel.h"
#include "database.h"


// 添加好友
void FriendModel::insert(int userid, int friendid)
{
	char sql[1024] = { 0 };
	sprintf(sql, "insert into friend values(%d, %d)", userid, friendid);
	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}

}

// 获得好友的信息
std::vector<User> FriendModel::query(int id)
{
	char sql[1024] = { 0 };
	sprintf(sql, "select a.id, a.name, a.state from user a inner join"
		" friend b on b.friendid = a.id where b.userid = %d", id);
	MySQL mysql;
	std::vector<User> vec;
	if (mysql.connect())
	{
		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			MYSQL_ROW row;
			
			while ((row = mysql_fetch_row(res)) != nullptr)
			{
				User user;
				user.setid(atoi(row[0]));
				user.setname(row[1]);
				user.setstate(row[2]);
				vec.push_back(user);
			}
			mysql_free_result(res);
		}
	}
	return vec;
}

