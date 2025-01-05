#include "redis.h"
#include <iostream>
#include <string>

static const char* ip = "127.0.0.1";
static const int port = 6379;

// 构造函数
Redis::Redis()
    :publishContext_(nullptr),
    subscribeContext_(nullptr)
{
}
// 析构函数
Redis::~Redis()
{
    // 释放连接
    if (publishContext_ != nullptr)
    {
        redisFree(publishContext_);
    }
    if (subscribeContext_ != nullptr)
    {
        redisFree(subscribeContext_);
    }
}

// 连接
bool Redis::connect()
{
    publishContext_ = redisConnect(ip, port);
    if (publishContext_ == nullptr)
    {
        std::cerr << "redis connect publishContext_ err" << std::endl;
        return false;
    }
    subscribeContext_ = redisConnect(ip, port);
    if (subscribeContext_ == nullptr)
    {
        std::cerr << "redis connect subscribeContext_ err" << std::endl;
        return false;
    }

    // 开辟一个线程进行接受消息
    std::thread t([&]()
        {
            recvSubscribeMsg();
        });
    t.detach();
    return true;
}

// 发布消息
bool Redis::publish(int channel, std::string msg)
{
    redisReply* reply = (redisReply*)redisCommand(publishContext_, "publish %d %s", channel, msg.c_str());
    if (nullptr == reply)
    {
        std::cerr << "publish channel err" << std::endl;
        return false;
    }

    freeReplyObject(reply);

    return true;
}
// 订阅通道
bool Redis::subscribe(int channel)
{
    // 由于 订阅通道是线程阻塞的因此这里只做订阅, 不接受消息
    if (REDIS_ERR == (redisAppendCommand(subscribeContext_, "subscribe %d", channel)))
    {
        std::cerr << "subscribe channel err" << std::endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == (redisBufferWrite(subscribeContext_, &done)))
        {
            std::cerr << "subscribe channel err" << std::endl;
            return false;
        }
    }

    return true;
}


// 取消订阅
bool Redis::unsubscribe(int channel)
{
    // 由于 取消订阅通道是线程阻塞的因此这里只做订阅, 不接受消息
    if (REDIS_ERR == (redisAppendCommand(subscribeContext_, "unsubscribe %d", channel)))
    {
        std::cerr << "unsubscribe channel err" << std::endl;
        return false;
    }
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == (redisBufferWrite(subscribeContext_, &done)))
        {
            std::cerr << "unsubscribe channel err" << std::endl;
            return false;
        }
    }

    return true;
}

// 初始化订阅消息的回调函数
void Redis::initMsgCallBack(std::function<void(int, std::string)> func)
{
    func_ = func;
}
/* 由于 subscribe 订阅通道会阻塞线程, 因此需要另外开一个线程进行专门
* 接受订阅的消息
*/
void Redis::recvSubscribeMsg()
{
    redisReply* reply = nullptr;
    while (REDIS_OK == (redisGetReply(subscribeContext_, (void**)&reply)))
    {
        if (reply != nullptr && reply->element[1]->str != nullptr && reply->element[2]->str != nullptr)
        {
            func_(std::stoi(reply->element[1]->str), reply->element[2]->str);
        }
        
        freeReplyObject(reply);
    }

    std::cout << "接受订阅线程退出" << std::endl;

}
