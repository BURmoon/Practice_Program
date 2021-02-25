/*
#   select多路IO实现服务端
#   使用自定义数组记录 连接socket
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
    int i, j, n, maxi;

    //自定义数组client, 防止遍历1024个文件描述符  
    // FD_SETSIZE = 1024
    int nready, client[FD_SETSIZE]; 
    int maxfd, listenfd, connfd, sockfd;
    char buf[BUFSIZ], str[INET_ADDRSTRLEN]; 

    struct sockaddr_in clie_addr, serv_addr;
    socklen_t clie_addr_len;
    //rset --> 读事件文件描述符集合，allset --> 用来备份
    fd_set rset, allset;

    //创建监听socket --> 设置端口复用 --> 初始化服务端地址 --> bind --> listen
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family= AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port= htons(SERV_PORT);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 128);

    //未建立连接时，listenfd 即为最大文件描述符
    maxfd = listenfd;

    maxi = -1;
    //初始化client[]
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1; 

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
            connfd = accept(listenfd, (struct sockaddr *)&clie_addr, &clie_addr_len);
            printf("received from %s at port %d\n",
                    inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)),
                    ntohs(clie_addr.sin_port));

            //在client[]找到空位，保存 连接socket
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (client[i] < 0) 
                { 
                    client[i] = connfd; 
                    break;
                }
            }
            //在client[]没找到空位
            if (i == FD_SETSIZE) 
            {
                fputs("too many clients\n", stderr);
                exit(1);
            }

            //将 连接socket 保存在client[]中
            // 向监控文件描述符集合allset添加新的文件描述符connfd
            FD_SET(connfd, &allset);

            //更新maxfd
            // 保证maxfd存的总是最大的文件描述符，select()要用
            if (connfd > maxfd)
                maxfd = connfd;

            //更新maxi
            // 保证maxi存的总是client[]最后一个元素下标
            if (i > maxi)
                maxi = i;

            //只有客户端连接，跳过本次循环
            if (--nready == 0)
                continue;
        } 

        //寻找client[]中已经就绪的读事件
        for (i = 0; i <= maxi; i++) 
        {
            //当前client[i]没保存连接socket，则跳过当前循环
            if ((sockfd = client[i]) < 0)
                continue;

            //判断当前 连接socket 是否在读文件描述符监听集合rset中
            if (FD_ISSET(sockfd, &rset)) 
            {
                //在读文件描述符监听集合中
                if ((n = read(sockfd, buf, sizeof(buf))) == 0) 
                {    
                    //client关闭链接时，服务器端关闭对应链接
                    close(sockfd);
                    //在allset中移除当前 连接socket
                    // 解除select对当前 连接socket 的监控
                    FD_CLR(sockfd, &allset); 
                    //将client[]中 连接socke 处置为-1
                    client[i] = -1;
                } 
                else if (n > 0) 
                {
                    //处理客户端数据
                    for (j = 0; j < n; j++)
                        buf[j] = toupper(buf[j]);

                    write(sockfd, buf, n);
                    write(STDOUT_FILENO, buf, n);
                }

                //监听的读事件处理完，跳出当前for循环
                if (--nready == 0)
                    break;
            }
        }
    }

    close(listenfd);
    return 0;
}

