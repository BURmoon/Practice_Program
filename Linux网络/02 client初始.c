/*
#   简易客户端
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

//服务端IP+Port
#define SERV_IP "127.0.0.1"
#define SERV_PORT 9527

int main(void)
{
    int sfd, len;
    //保存服务端地址
    struct sockaddr_in serv_addr;
    char buf[BUFSIZ]; 

    //创建套接字
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    //初始化服务端地址
    bzero(&serv_addr, sizeof(serv_addr));                       
    serv_addr.sin_family = AF_INET;                             
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);    
    serv_addr.sin_port = htons(SERV_PORT);                      

    //套接字sfd与服务端建立连接
    connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    //与服务端通信处理
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        //向服务端写数据
        int r = Write(sfd, buf, strlen(buf));       
        printf("Write [%d]", r);

        //从服务端读数据
        len = Read(sfd, buf, sizeof(buf));
        printf("Read [%d] --> ", len);
        Write(STDOUT_FILENO, buf, len);
    }

    close(sfd);

    return 0;
}

