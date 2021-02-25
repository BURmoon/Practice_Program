#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <pthread.h>

//读事件回调
//	当数据由内核的读缓冲区到bufferevent的读缓冲区的时候，会触发bufferevent的读事件回调，该过程是由bufferevent内部操作的
//	数据读到缓冲区后，通过读事件回调将数据加载到内存
void read_cb(struct bufferevent *bev, void *arg) 
{
	char buf[1024] = {0};
	
	//借助读缓冲，从客户端拿数据
	bufferevent_read(bev, buf, sizeof(buf));
	printf("clinet write: %s\n", buf);
	
	//借助写缓冲，写数据回给客户端
	bufferevent_write(bev, "abcdefg", 7);	
}

//写事件回调
// 当用户程序将数据写到bufferevent的写缓冲区之后，bufferevent会自动将数据写到内核的写缓冲区，最终由内核程序将数据发送出去
// 数据写到缓冲区后，才调用写事件回调
void write_cb(struct bufferevent *bev, void *arg) 
{
	printf("has wrote\n");	
}

//其他事件回调
void event_cb(struct bufferevent *bev,  short events, void *ctx) 
{
	
}

//被回调，说明有客户端成功连接，cfd已经传入该参数内部，创建bufferevent事件对象
// 与客户端完成读写操作
 void listener_cb(struct evconnlistener *listener, evutil_socket_t sock, 
 			struct sockaddr *addr, int len, void *ptr)
 {
 	struct event_base *base = (struct event_base *)ptr;
 	
 	//创建bufferevent 对象
 	struct bufferevent *bev = NULL;	
 	bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
 	
 	//给bufferevent 对象 设置回调 read、write、event
 	/*
 	void bufferevent_setcb(struct bufferevent * bufev,
				bufferevent_data_cb readcb,
				bufferevent_data_cb writecb,
				bufferevent_event_cb eventcb,
				void *cbarg );
	*/
		
	//设置回调函数		
	bufferevent_setcb(bev, read_cb, write_cb, NULL, NULL);
	
	//启动 read 缓冲区的 使能状态
	bufferevent_enable(bev, EV_READ); 
 	
 	return ;	
 }
 			 

int main(int argc, char *argv[])
{
	//定义服务器地址结构
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(8765);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//创建event_base
	struct event_base *base = event_base_new();

	//创建服务器监听器 --> socket()/bind()/listen()/accept();
	// 新连接到达时，监听器调用回调函数
	struct evconnlistener *listener = NULL;
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base, 
						LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, 
						(struct sockaddr *)&srv_addr, sizeof(srv_addr));
	
	//启动监听循环
	event_base_dispatch(base);
	
	//销毁event_base
	evconnlistener_free(listener);
	event_base_free(base);

	return 0;
}
