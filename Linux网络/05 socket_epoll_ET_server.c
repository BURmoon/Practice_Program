/*
#   epoll实现单个客户端-服务端连接
#   ET模式(边沿触发)
*/

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 10
#define SERV_PORT 8000

int main(void)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int efd, flag;

    //创建监听socket --> 设置端口复用 --> 初始化服务端地址 --> bind --> listen
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, 20);

    struct epoll_event event;
    struct epoll_event res_event[10];
    int res, len;

    //创建epoll模型, efd指向红黑树根节点
    efd = epoll_create(10);

    //阻塞等待客户端建立连接
    cliaddr_len = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
    printf("Accepting connections ...\n");
    printf("received from %s at PORT %d\n",
            inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
            ntohs(cliaddr.sin_port));

    //将 连接socket 修改为非阻塞读
    flag = fcntl(connfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(connfd, F_SETFL, flag);

    //将 连接socket 上树
    // 设置为ET边沿触发模式
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = connfd;
    epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);
    while (1) 
    {
        printf("epoll_wait begin\n");
        //阻塞监听
        res = epoll_wait(efd, res_event, 1, -1);
        printf("epoll_wait end res %d\n", res);

        if (res_event[0].data.fd == connfd) 
        {
            //非阻塞读, 轮询
            while ((len = read(connfd, buf, MAXLINE/2)) >0 )    
                write(STDOUT_FILENO, buf, len);
        }
    }

    return 0;
}


