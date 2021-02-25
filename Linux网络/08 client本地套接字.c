/*
#   本地套接字-客户端
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stddef.h>

#define SERV_ADDR "serv.socket"
#define CLIE_ADDR "clie.socket"

int main(void)
{
    int  cfd, len;
    struct sockaddr_un servaddr, cliaddr;
    char buf[4096];

    cfd = scket(AF_UNIX, SOCK_STREAM, 0);

    //初始化 客户端sockaddr_un
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sun_family = AF_UNIX;
    strcpy(cliaddr.sun_path, CLIE_ADDR);

    //计算客户端地址结构有效长度
    len = offsetof(struct sockaddr_un, sun_path) + strlen(cliaddr.sun_path);
    //客户端也需要bind, 不能依赖自动绑定
    unlink(CLIE_ADDR);
    bind(cfd, (struct sockaddr *)&cliaddr, len);

    //初始化 服务端sockaddr_un
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, SERV_ADDR);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);   /* 计算服务器端地址结构有效长度 */

    //与服务端建立连接
    connect(cfd, (struct sockaddr *)&servaddr, len);

    while (fgets(buf, sizeof(buf), stdin) != NULL) 
    {
        write(cfd, buf, strlen(buf));
        len = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, len);
    }

    close(cfd);

    return 0;
}
