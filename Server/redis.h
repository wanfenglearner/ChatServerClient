
#ifndef _REDIS_H_
#define _REDIS_H_

#include <hiredis/hiredis.h>
#include <thread>
#include <string>
#include <functional>
class Redis
{
public:

	// 构造函数
	Redis();
	// 析构函数
	~Redis();
	// 连接
	bool connect();

	// 发布消息
	bool publish(int channel, std::string msg);
	// 订阅通道
	bool subscribe(int channel);

	// 取消订阅
	bool unsubscribe(int channel);

	// 初始化订阅消息的回调函数
	void initMsgCallBack(std::function<void(int, std::string)> func);

private:
	/* 由于 subscribe 订阅通道会阻塞线程, 因此需要另外开一个线程进行专门
	* 接受订阅的消息
	*/
	void  recvSubscribeMsg();
private:

	redisContext* publishContext_;		// 用于发布的连接
	redisContext* subscribeContext_;	// 用于订阅的连接
	std::function<void(int, std::string)> func_;	// 传递订阅的消息的回调函数
};



#endif





