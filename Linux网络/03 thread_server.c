/*
#   多线程服务端
*/

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE 8192
#define SERV_PORT 8000

//定义一个结构体, 将 客户端地址结构 跟 连接socket 捆绑
struct s_info {                     
    struct sockaddr_in cliaddr;
    int connfd;
};

//线程处理函数
void *do_work(void *arg)
{
    int n,i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];      //INET_ADDRSTRLEN = 16，可用"[+d"查看

    //处理客户端数据
    while (1) 
    {
        n = read(ts->connfd, buf, MAXLINE); 
        if (n == 0) 
        {
            printf("the client %d close\n", ts->connfd);
            break;
        }
        printf("received from %s at port %d\n",
                inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)),
                ntohs((*ts).cliaddr.sin_port));                 

        for (i = 0; i < n; i++) 
            buf[i] = toupper(buf[i]); 

        write(STDOUT_FILENO, buf, n);                          
        write(ts->connfd, buf, n);                             
    }
    
    Close(ts->connfd);

    return (void *)0;
}

int main(void)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int listenfd, connfd;
    pthread_t tid;

    //创建结构体数组
    struct s_info ts[256]; 
    int i = 0;

    //创建监听socket --> 初始化服务端地址 --> bind --> listen
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);         

    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 

    listen(listenfd, 128); 
    printf("Accepting client connect ...\n");

    while (1) 
    {
        cliaddr_len = sizeof(cliaddr);
        //阻塞等待客户端建立连接
        connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len); 
        ts[i].cliaddr = cliaddr;
        ts[i].connfd = connfd;

        //创建子线程
        pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
        //子线程分离，防止僵线程产生
        pthread_detach(tid);

        i++;
    }

    return 0;
}

