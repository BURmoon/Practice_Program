#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

//读缓冲区回调
// typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
void read_cb(struct bufferevent *bev, void *arg)
{
    char buf[1024] = {0};   
    //size_t bufferevent_read(struct bufferevent *bufev, void *data, size_t size);
    // bufferevent_read 是将bufferevent的读缓冲区数据读到data中, 同时将读到的数据从bufferevent的读缓冲清除
    bufferevent_read(bev, buf, sizeof(buf));
    printf("client say: %s\n", buf);

    char *p = "服务端：收到数据";
    // 发数据给客户端
    //int bufferevent_write(struct bufferevent *bufev, const void *data, size_t size);
    //bufferevent_write是将data的数据写到bufferevent的写缓冲区
    bufferevent_write(bev, p, strlen(p)+1);
    sleep(1);
}

//写缓冲区回调
// typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
void write_cb(struct bufferevent *bev, void *arg)
{
    printf("服务端：成功写数据给客户端，写缓冲区回调函数被回调\n"); 
}

//事件回调
// typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short what, void *ctx);
// 客户端关闭连接或者是被信号终止进程会触发事件回调函数
void event_cb(struct bufferevent *bev, short events, void *arg)
{
    if (events & BEV_EVENT_EOF)
    {
        printf("connection closed\n");  
    }
    else if(events & BEV_EVENT_ERROR)   
    {
        printf("some other error\n");
    }
    
    //bufferevent_free()释放bufferevent
    bufferevent_free(bev);    
    printf("buffevent 资源已经被释放\n"); 
}


//typedef void (*evconnlistener_cb)(struct evconnlistener *evl, 
//                    evutil_socket_t fd, struct sockaddr *cliaddr, int socklen, void *ptr);
void cb_listener(
        struct evconnlistener *listener, 
        evutil_socket_t fd, 
        struct sockaddr *addr, 
        int len, void *ptr)
{
   printf("connect new client\n");

   struct event_base* base = (struct event_base*)ptr;
   //通信操作，添加新事件
   struct bufferevent *bev;
   // struct bufferevent *bufferevent_socket_new(struct event_base *base, evutil_socket_t fd, int options);
   // bufferevent_socket_new对已经存在socket创建bufferevent事件
   // 当往缓冲区中写数据的时候会触发写回调函数，由于数据最终是写入了内核的写缓冲区中, 这个事件只是通知功能
   // 当数据从socket的内核缓冲区读到bufferevent读缓冲区中的时候会触发读回调函数
   bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
   // BEV_OPT_CLOSE_ON_FREE--当bufferevent被释放以后, 文件描述符也随之被close

   //bufferevent_setcb用于设置bufferevent的回调函数
   //void bufferevent_setcb(struct bufferevent *bufev,
   //                 bufferevent_data_cb readcb, bufferevent_data_cb writecb,
   //                 bufferevent_event_cb eventcb, void *cbarg);
   //typedef void (*bufferevent_data_cb)(struct bufferevent *bev, void *ctx);
   //typedef void (*bufferevent_event_cb)(struct bufferevent *bev, short what, void *ctx);
   bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
   //bufferevent_enable是设置事件是生效, 如果设置为bufferevent_disable(), 事件回调将不会被触发
   bufferevent_enable(bev, EV_READ);
}


int main(int argc, const char* argv[])
{
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(9876);
    serv.sin_addr.s_addr = htonl(INADDR_ANY);

    //创建event_base
    struct event_base* base;
    base = event_base_new();

    //链接监听器-evconnlistener，创建套接字-->绑定-->接收连接请求
    //链接监听器封装了底层的socket通信相关函数, 比如socket, bind, listen, accept这几个函数
    //链接监听器创建后实际上相当于调用了socket, bind, listen
    //此时等待新的客户端连接到来, 如果有新的客户端连接, 那么内部先进行调用accept处理, 然后调用用户指定的回调函数
    //evconnlistener_new_bind()函数--在当前没有套接字的情况下对链接监听器进行初始化
    //struct evconnlistener *evconnlistener_new_bind(struct event_base *base,
    //                    evconnlistener_cb cb, void *ptr, unsigned flags, int backlog,
    //                    const struct sockaddr *sa, int socklen);
    //回调函数：
    //typedef void (*evconnlistener_cb)(struct evconnlistener *evl, 
    //                    evutil_socket_t fd, struct sockaddr *cliaddr, int socklen, void *ptr);
    //fd参数是与客户端通信的描述符, 并非是等待连接的监听的那个描述符
    //cliaddr对应的也是新连接的对端地址信息, 已经是accept处理好的
    struct evconnlistener* listener;
    listener = evconnlistener_new_bind(base, cb_listener, base, 
                                  LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 
                                  36, (struct sockaddr*)&serv, sizeof(serv));

    //进入事件循环等待(等待事件产生)
    event_base_dispatch(base);

    //释放链接监听器
    evconnlistener_free(listener);
    event_base_free(base);

    return 0;
}
