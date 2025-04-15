/*
*	MYSQL 数据库
*/

#include "database.h"
#include <muduo/base/Logging.h>
#include <string>
#include <muduo/base/LogFile.h>

// 初始化数据库的配置信息
static const std::string host = "127.0.0.1";	
static const std::string name = "root";
static const std::string password = "wss0501";
static const std::string dbname = "chat";
static const int port = 3306;

// 构造函数
MySQL::MySQL()
{
	// 初始化数据库连接
	conn_  = mysql_init(nullptr);
}
// 析构函数
MySQL::~MySQL()
{
	if (conn_ != nullptr)
	{
		// 关闭数据库连接
		mysql_close(conn_);
	}
}
// 连接数据库
bool MySQL::connect()
{
	
	MYSQL* res = mysql_real_connect(conn_, host.c_str(), name.c_str(),
		password.c_str(), dbname.c_str(), port, nullptr, 0);
	if (res == nullptr)
	{
		LOG_ERROR << "数据库连接失败";
	}
	else
	{
		LOG_INFO << "数据库连接成功";
	}
	return res;
}
// 更新数据库信息
bool MySQL::update(std::string sql)
{
	// 成功返回0 失败返回-1
	if (mysql_query(conn_, sql.c_str()))
	{
		LOG_INFO << __FILE__ << ":" << __LINE__ << sql << " 查询失败";
		return false;
	}
	LOG_INFO << __FILE__ << ":" << __LINE__ << sql << " 更新成功";
	return true;

}
// 查询数据库的信息 获得结果
MYSQL_RES* MySQL::query(std::string sql)
{
	// 成功返回0 失败返回-1
	if (mysql_query(conn_, sql.c_str()))
	{
		LOG_INFO << __FILE__ << ":" << __LINE__ << sql << " 查询失败";
		return nullptr;
	}
	LOG_INFO << __FILE__ << ":" << __LINE__ << sql << " 更新成功";
	return mysql_use_result(conn_);

}
// 获得当前数据库连接的句柄
MYSQL* MySQL::getConnect()
{
	return conn_;
}
