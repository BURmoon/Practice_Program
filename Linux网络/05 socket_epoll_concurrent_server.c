/*
#   epoll多路IO实现服务端
#   多并发连接
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>

#define MAXLINE 8192
#define SERV_PORT 8000

#define OPEN_MAX 5000

int main(int argc, char *argv[])
{
    int i, listenfd, connfd, sockfd;
    int  n, num = 0;
    ssize_t nready, efd, res;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;

    struct sockaddr_in cliaddr, servaddr;

    //创建监听socket --> 设置端口复用 --> 初始化服务端地址 --> bind --> listen
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, 20);

    //创建epoll模型, efd指向红黑树根节点
    efd = epoll_create(OPEN_MAX);
    if (efd == -1)
        perror("epoll_create error");

    //tep执行上树操作，ep保存满足的监听事件
    struct epoll_event tep, ep[OPEN_MAX];

    //tep添加 监听socket，并设为读事件
    tep.events = EPOLLIN; 
    tep.data.fd = listenfd;

    //将lfd及对应的结构体设置到树上
    res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);    
    if (res == -1)
        perror("epoll_ctl error");

    for ( ; ; ) 
    {
        //阻塞监听，等待内核返回事件发生
        nready = epoll_wait(efd, ep, OPEN_MAX, -1); 
        if (nready == -1)
            perror("epoll_wait error");

        for (i = 0; i < nready; i++) 
        {
            //epoll返回的事件不是读事件，则跳过
            if (!(ep[i].events & EPOLLIN))
                continue;

            //epoll返回读事件
            //当前事件为 监听socket 的读事件时，接受连接并将新的 连接socket 上树
            if (ep[i].data.fd == listenfd) 
            {
                clilen = sizeof(cliaddr);
                connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

                printf("received from %s at PORT %d\n", 
                        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
                        ntohs(cliaddr.sin_port));
                printf("cfd %d---client %d\n", connfd, ++num);

                //新的 连接socket 加入红黑树上
                tep.events = EPOLLIN; 
                tep.data.fd = connfd;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
                if (res == -1)
                    perror("epoll_ctl error");

            } 
            //当前事件不是 监听socket 的读事件时，处理客户端数据
            else 
            { 
                sockfd = ep[i].data.fd;
                n = read(sockfd, buf, MAXLINE);

                //read()返回 0，说明客户端关闭链接
                if (n == 0) 
                {           
                    //将该 连接socket 从红黑树说摘除                                
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);  
                    if (res == -1)
                        perror("epoll_ctl error");

                    //关闭与该客户端的连接
                    close(sockfd);                                      
                    printf("client[%d] close connection\n", sockfd);

                } 
                //read()返回 <0，客户端出现错误
                else if (n < 0) 
                {  
                    perror("read n < 0 error");
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    close(sockfd);

                } 
                //read()返回 <0，处理客户端发送的数据
                else 
                {                                               
                    for (i = 0; i < n; i++)
                        buf[i] = toupper(buf[i]); 

                    write(STDOUT_FILENO, buf, n);
                    writen(sockfd, buf, n);
                }
            }
        }
    }

    close(listenfd);
    close(efd);

    return 0;
}

