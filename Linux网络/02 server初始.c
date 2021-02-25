/*
#   简易服务端
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

#define SERV_PORT 9527

int main(void)
{
    //sfd --> 监听socket，cfd --> 连接socket
    int sfd, cfd;
    int len, i;
    char buf[BUFSIZ], clie_IP[BUFSIZ];

    //保存 服务端/客户端 地址
    struct sockaddr_in serv_addr, clie_addr;
    //保存客户端地址长度
    socklen_t clie_addr_len;

    //创建一个socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);

    //初始化服务端的地址结构
    bzero(&serv_addr, sizeof(serv_addr));           //将整个结构体清零
    serv_addr.sin_family = AF_INET;                 //选择协议族为IPv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //监听本地所有IP地址
    serv_addr.sin_port = htons(SERV_PORT);          //绑定端口号    

    //将 sfd 与 服务端地址 绑定
    bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    //设置 sfd 进行监听，同时设定链接上限
    listen(sfd, 64);                                //同一时刻允许向服务器发起链接请求的数量
    printf("wait for client connect ...\n");

    //初始化客户端地址结构大小 
    clie_addr_len = sizeof(clie_addr_len);
    //阻塞等待客户端建立连接
    // 参数1是sfd; 参2传出参数, 参3传入传入参数, 全部是client端的参数
    // 返回一个与客户端成功连接的socket文件描述符
    cfd = accept(sfd, (struct sockaddr *)&clie_addr, &clie_addr_len);

    //打印客户端地址信息
    printf("client IP[%s]\tport[%d]\n", 
            inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_IP, sizeof(clie_IP)), 
            ntohs(clie_addr.sin_port));

    while (1)
    {
        //读取客户端发送数据
        len = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);

        //处理客户端数据
        for (i = 0; i < len; i++)
            buf[i] = toupper(buf[i]);

        //处理完数据回写给客户端
        write(cfd, buf, len); 
    }

    //关闭连接
    close(sfd);
    close(cfd);

    return 0;
}
