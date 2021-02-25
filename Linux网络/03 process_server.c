/*
#   多进程服务端
*/

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>

#define MAXLINE 8192
#define SERV_PORT 8000

//信号处理函数
void do_sigchild(int num)
{
    //回收子进程
    // 设置WNOHANG，调用中waitpid发现没有已退出的子进程可收集，则返回0
    while (waitpid(0, NULL, WNOHANG) > 0);
}

int main(void)
{
    //保存 服务端/客户端 地址
    struct sockaddr_in servaddr, cliaddr;
    //保存客户端地址长度
    socklen_t cliaddr_len;
    //listenfd --> 监听socket，connfd --> 连接socket
    int listenfd, connfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int i, n;
    pid_t pid;
    struct sigaction newact;

    //设置信号处理函数
    newact.sa_handler = do_sigchild;
    //清空需要阻塞的信号集
    sigemptyset(&newact.sa_mask);
    //表示使用默认标识 0
    newact.sa_flags = 0;
    //注册信号处理函数
    // 参数1是捕捉的信号; 参2新的处理方式, 参3(传出)旧的处理方式
    sigaction(SIGCHLD, &newact, NULL);

    //创建监听socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    //设置端口复用
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //初始化服务端地址结构
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //listenfd 绑定服务端地址
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    //设置监听
    listen(listenfd, 20);
    printf("Accepting connections ...\n");

    while (1) 
    {
        cliaddr_len = sizeof(cliaddr);
        //阻塞等待客户端建立连接
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
        printf("客户端建立连接[%d] --> \n", connfd);

        //创建子进程
        pid = fork();
        if (pid == 0) 
        {
            //子进程关闭 监听socket
            Close(listenfd);

            //处理客户端数据
            while (1) 
            {
                //打印客户端地址信息
                printf("received from %s at PORT %d\n",
                        inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                        ntohs(cliaddr.sin_port));

                n = read(connfd, buf, MAXLINE);
                if (n == 0) 
                {
                    printf("对端关闭\n");
                    break;
                }

                for (i = 0; i < n; i++)
                    buf[i] = toupper(buf[i]);

                write(STDOUT_FILENO, buf, n);
                write(connfd, buf, n);
            }

            close(connfd);
            return 0;
        } 
        else if (pid > 0) 
        {
            //父进程关闭 连接socke
            Close(connfd);
        }  else
            //创建子进程失败
            printf("fork error\n");
    }

    return 0;
}


