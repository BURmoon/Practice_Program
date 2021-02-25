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

//���¼��ص�
//	���������ں˵Ķ���������bufferevent�Ķ���������ʱ�򣬻ᴥ��bufferevent�Ķ��¼��ص����ù�������bufferevent�ڲ�������
//	���ݶ�����������ͨ�����¼��ص������ݼ��ص��ڴ�
void read_cb(struct bufferevent *bev, void *arg) 
{
	char buf[1024] = {0};
	
	//���������壬�ӿͻ���������
	bufferevent_read(bev, buf, sizeof(buf));
	printf("clinet write: %s\n", buf);
	
	//����д���壬д���ݻظ��ͻ���
	bufferevent_write(bev, "abcdefg", 7);	
}

//д�¼��ص�
// ���û���������д��bufferevent��д������֮��bufferevent���Զ�������д���ں˵�д���������������ں˳������ݷ��ͳ�ȥ
// ����д���������󣬲ŵ���д�¼��ص�
void write_cb(struct bufferevent *bev, void *arg) 
{
	printf("has wrote\n");	
}

//�����¼��ص�
void event_cb(struct bufferevent *bev,  short events, void *ctx) 
{
	
}

//���ص���˵���пͻ��˳ɹ����ӣ�cfd�Ѿ�����ò����ڲ�������bufferevent�¼�����
// ��ͻ�����ɶ�д����
 void listener_cb(struct evconnlistener *listener, evutil_socket_t sock, 
 			struct sockaddr *addr, int len, void *ptr)
 {
 	struct event_base *base = (struct event_base *)ptr;
 	
 	//����bufferevent ����
 	struct bufferevent *bev = NULL;	
 	bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
 	
 	//��bufferevent ���� ���ûص� read��write��event
 	/*
 	void bufferevent_setcb(struct bufferevent * bufev,
				bufferevent_data_cb readcb,
				bufferevent_data_cb writecb,
				bufferevent_event_cb eventcb,
				void *cbarg );
	*/
		
	//���ûص�����		
	bufferevent_setcb(bev, read_cb, write_cb, NULL, NULL);
	
	//���� read �������� ʹ��״̬
	bufferevent_enable(bev, EV_READ); 
 	
 	return ;	
 }
 			 

int main(int argc, char *argv[])
{
	//�����������ַ�ṹ
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(8765);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//����event_base
	struct event_base *base = event_base_new();

	//���������������� --> socket()/bind()/listen()/accept();
	// �����ӵ���ʱ�����������ûص�����
	struct evconnlistener *listener = NULL;
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base, 
						LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, 
						(struct sockaddr *)&srv_addr, sizeof(srv_addr));
	
	//��������ѭ��
	event_base_dispatch(base);
	
	//����event_base
	evconnlistener_free(listener);
	event_base_free(base);

	return 0;
}
