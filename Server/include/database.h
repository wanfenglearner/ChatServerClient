
/*
*	MYSQL 数据库
*/
#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <string>
#include <mysql/mysql.h>


class MySQL
{
public:
	// 构造函数
	MySQL();
	// 析构函数
	~MySQL();
	// 连接数据库
	bool connect();
	// 更新数据库信息
	bool update(std::string sql);
	// 查询数据库的信息 获得结果
	MYSQL_RES* query(std::string sql);
	// 获得当前数据库连接的句柄
	MYSQL* getConnect();

private:
	MYSQL* conn_;	// 连接数据库的句柄

};


#endif