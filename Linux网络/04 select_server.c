/*
#   select多路IO实现服务端
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "wrap.h"

#define SERV_PORT 6666

int main(int argc, char *argv[])
{
    int i, j, n, nready;
    int maxfd = 0;
    int listenfd, connfd;
    char buf[BUFSIZ]; 

    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;

    //创建监听socket --> 设置端口复用 --> 初始化服务端地址 --> bind --> listen
    listenfd = socket(AF_INET, SOCK_STREAM, 0);  
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family= AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port= htons(SERV_PORT);
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 128);

    //rset --> 读事件文件描述符集合，allset --> 用来备份
    fd_set rset, allset;
    maxfd = listenfd;

    //构造select监控文件描述符集
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset); 

    while (1) 
    {
        //设置select监控信号集   
        rset = allset;         

        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (nready < 0)
            printf("select error");

        //有新的客户端链接请求
        if (FD_ISSET(listenfd, &rset)) 
        { 
            clie_addr_len = sizeof(clie_addr);
            //此时 accept() 不会阻塞
            connfd = Accept(listenfd, (struct sockaddr *)&clie_addr, &clie_addr_len);

            //向监控文件描述符集合allset添加新的文件描述符connfd
            FD_SET(connfd, &allset);

            //更新maxfd
            // 保证maxfd存的总是最大的文件描述符，select()要用
            if (maxfd < connfd)
                maxfd = connfd;

            //只有listenfd有事件, 后续的for不需执行
            if (0 == --nready)
                continue;
        } 

        //寻找client[]中已经就绪的读事件
        for (i = listenfd+1; i <= maxfd; i++) 
        {
            //判断当前 socket 是否在读文件描述符监听集合rset中
            if (FD_ISSET(i, &rset)) 
            {
                //在读文件描述符监听集合中
                if ((n = read(i, buf, sizeof(buf))) == 0) 
                {
                    close(i);
                    FD_CLR(i, &allset);
                } 
                else if (n > 0) 
                {
                    //处理客户端数据
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);
                    
                    write(i, buf, n);
                }
            }
        }
    }

    Close(listenfd);
    return 0;
}

