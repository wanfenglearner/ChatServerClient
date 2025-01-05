
#include "usermodel.h"
#include "database.h"

// 向user表中添加元素
bool UserModel::insert(User& user)
{
	// 组装sql语句
	char sql[1024] = { 0 };
	sprintf(sql, "insert into user(name,password,state) values('%s','%s','%s')",
		user.getname().c_str(), user.getpassword().c_str(), user.getstate().c_str());
	MySQL mysql;
	if (mysql.connect())
	{
		if (mysql.update(sql))
		{
			user.setid((int)mysql_insert_id(mysql.getConnect()));

			return true;
		}
	}
	return false;
}
// 根据id获取用户信息
User UserModel::query(int id)
{
	// 组装sql语句
	char sql[1024] = { 0 };
	sprintf(sql, "select * from user where id = %d", id);

	User user;
	MySQL mysql;
	if (mysql.connect())
	{
		MYSQL_RES* res = mysql.query(sql);
		if (res != nullptr)
		{
			// 获得从数据库中的消息
			MYSQL_ROW row = mysql_fetch_row(res);
			if (row != nullptr)
			{
				user.setid(atoi(row[0]));
				user.setname(row[1]);
				user.setpassword(row[2]);
				user.setstate(row[3]);
			}
			mysql_free_result(res);
		}
	}

	return user;
}
// 更新用户的状态信息
bool UserModel::updateState(User& user)
{
	char sql[1024] = { 0 };
	sprintf(sql, "update user set state = '%s' where id = %d",
		user.getstate().c_str(), user.getid());

	MySQL mysql;
	if (mysql.connect())
	{
		if (mysql.update(sql))
		{
			return true;
		}
	}
	return false;
}
// 重新更新user表中的在线状态
void UserModel::resetState()
{
	char sql[1024] = { 0 };
	sprintf(sql, "update user set state = 'offline' where state = 'online'");
	MySQL mysql;
	if (mysql.connect())
	{
		mysql.update(sql);
	}
}