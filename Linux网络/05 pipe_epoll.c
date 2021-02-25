/*
#   epoll多路IO配合管道实现进程间通信
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <errno.h>
#include <unistd.h>

#define MAXLINE 10

int main(int argc, char *argv[])
{
    int efd, i;
    int pfd[2];
    pid_t pid;
    char buf[MAXLINE], ch = 'a';

    pipe(pfd);
    pid = fork();

    //子进程向管道写数据
    if (pid == 0) 
    {
        close(pfd[0]);

        while (1) 
        {
            //aaaa\n
            for (i = 0; i < MAXLINE/2; i++)
                buf[i] = ch;
            buf[i-1] = '\n';
            ch++;
            //bbbb\n
            for (; i < MAXLINE; i++)
                buf[i] = ch;
            buf[i-1] = '\n';
            ch++;
            //aaaa\nbbbb\n
            write(pfd[1], buf, sizeof(buf));
            sleep(5);
        }
        
        close(pfd[1]);
    } 
    //父进程从管道中读数据
    else if (pid > 0) 
    { 
        struct epoll_event event;
        struct epoll_event resevent[10];
        int res, len;

        close(pfd[1]);
        //创建监听红黑树
        efd = epoll_create(10);

        // ET 边沿触发
        event.events = EPOLLIN | EPOLLET;
        // LT 水平触发 (默认) 
        //event.events = EPOLLIN;

        //epoll_event设置监听管道读事件
        event.data.fd = pfd[0];
        //将event添加到树上
        epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);

        while (1) 
        {
            //阻塞监听，等待内核返回事件发生
            res = epoll_wait(efd, resevent, 10, -1);
            if (resevent[0].data.fd == pfd[0]) 
            {
                len = read(pfd[0], buf, MAXLINE/2);
                write(STDOUT_FILENO, buf, len);
            }
        }

        close(pfd[0]);
        close(efd);

    } 
    //进程创建失败
    else 
    {
        perror("fork");
        exit(-1);
    }

    return 0;
}

