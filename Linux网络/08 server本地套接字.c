/*
#   本地套接字-服务端
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>

#define SERV_ADDR  "serv.socket"

int main(void)
{
    int lfd, cfd, len, size, i;
    struct sockaddr_un servaddr, cliaddr;
    char buf[4096];

    lfd = socket(AF_UNIX, SOCK_STREAM, 0);

    //初始化 服务端sockaddr_un
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERV_ADDR);
    //计算服务端地址结构有效长度
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);
    //确保bind之前serv.sock文件不存在，bind会创建该文件
    unlink(SERV_ADDR);
    //确保参3不能是sizeof(servaddr)
    bind(lfd, (struct sockaddr *)&servaddr, len);
    listen(lfd, 20);

    while (1) {
        len = sizeof(cliaddr);
        //阻塞等待客户端建立连接
        cfd = accept(lfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);

        //得到文件名的长度
        len -= offsetof(struct sockaddr_un, sun_path);
        //确保打印时,没有乱码出现
        cliaddr.sun_path[len] = '\0';
        printf("client bind filename %s\n", cliaddr.sun_path);

        while ((size = read(cfd, buf, sizeof(buf))) > 0) 
        {
            for (i = 0; i < size; i++)
                buf[i] = toupper(buf[i]);
            write(cfd, buf, size);
        }
        close(cfd);
    }

    close(lfd);

    return 0;
}
