#include "head.h"

#define BUF_SIZE 1024 * 1024 * 4

int main(int argc, char const *argv[])
{
    /* 创建TCP通信端点 */
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1)
    {
        perror("socket()失败");
        exit(EXIT_FAILURE);
    }

    /* 准备服务端IP:PORT */
    struct sockaddr_in addr; /* IPv4协议的结构体 */
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 自动获取本机IP */
    addr.sin_port = htons(54321);             /* short */

    /* 将socket与本机IP:PORT绑定 */
    if (bind(sockfd, (struct sockaddr *)&addr, len) != 0)
    {
        perror("绑定地址失败");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 0); /* 设置监听状态 */
    printf("等待接收文件...\n");

    /* 静静滴等待对方的连接请求（每来一个请求就会创建一个信的connfd） */
    struct sockaddr_in cliaddr;
    len = sizeof(cliaddr);
    bzero(&cliaddr, len);
    int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
    if (connfd == -1)
    {
        perror("accept()失败");
        exit(EXIT_FAILURE);
    }
    printf("收到来自[%s:%hu]的连接！\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    /* 先收文件名 */
    char *name = calloc(1, 256);
    for (int i = 0;; i++)
    {
        read(connfd, &name[i], 1);
        if (name[i] == '\0')
            break;
    }
    printf("接收的文件名：%s\n", name);

    /* 再收文件大小 */
    char *file_len = calloc(1, 16);
    int size;
    for (int i = 0;; i++)
    {
        read(connfd, &file_len[i], 1);
        if (file_len[i] == '\0')
            break;
    }
    printf("文件大小：%d\n", size = atoi(file_len));

    /* 接收文件数据 */
    FILE *fp = fopen(name, "w");
    char *data = calloc(1, BUF_SIZE);
    int M = 0;
    while (size > 0)
    {
        bzero(data, 100);
        int m = read(connfd, data, BUF_SIZE);
        if (m == -1)
        {
            perror("read()失败");
            break;
        }

        size -= m;
        printf("\r已接收%d个字节", M += m);
        fwrite(data, m, 1, fp);
        fflush(fp);
    }
    printf("\n");

    free(name);
    free(data);
    free(file_len);
    close(sockfd);
    close(connfd);
    fclose(fp);

    return 0;
}
