#include "head.h"

#define BUF_SIZE 1024

void *snd_file(void *IP)
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
    addr.sin_addr.s_addr = inet_addr((char *)IP);
    addr.sin_port = htons(54321); /* short */

    /* 连接服务器 */
    if (connect(sockfd, (struct sockaddr *)&addr, len) != 0)
    {
        perror("连接失败");
        exit(EXIT_FAILURE);
    }
    printf("连接成功\n");

    /* 获取文件名 */
    char *buf = calloc(1, 256);
    printf("请输入要发送的文件：");
    scanf("%s", buf);

    /* 先发文件名 */
    char *name = strrchr(buf, '/');
    name = name ? name + 1 : buf;
    printf("已发送%ld个字节，", write(sockfd, name, strlen(name)));
    printf("发送文件名：%s\n", name);
    write(sockfd, "\0", 1); /* 发送分割符 */

    /* 获取文件大小 */
    FILE *fp = fopen(buf, "r");
    if (fp == NULL)
    {
        perror("打开文件失败");
        free(buf);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    /* 发送文件大小 */
    char *file_len = calloc(1, 16);
    snprintf(file_len, 16, "%ld", size);
    printf("已发送%ld个字节，", write(sockfd, file_len, strlen(file_len)));
    printf("文件大小：%ld\n", size);
    write(sockfd, "\0", 1); /* 发送分割符 */

    /* 释放无用的内存 */
    free(buf);
    free(file_len);

    /* 发送文件数据 */
    char *data = calloc(1, BUF_SIZE);
    long begin, end;
    int M = 0;
    while (1)
    {
        bzero(data, BUF_SIZE);
        begin = ftell(fp);
        int n = fread(data, BUFSIZ, 1, fp);
        if (n == 0)
        {
            if (feof(fp))
            {
                end = ftell(fp);
                int m = write(sockfd, data, end - begin);
                if (m == -1)
                {
                    printf("sendto()失败");
                    break;
                }
                fprintf(stderr, "\r已发送%d个字节", M += m);
            }
            if (ferror(fp))
            {
                perror("读取文件失败");
            }
            break;
        }
        int m = write(sockfd, data, BUF_SIZE);
        if (m == -1)
        {
            printf("sendto()失败");
            break;
        }
        fprintf(stderr, "\r已发送%d个字节", M += m);
    }
    printf("\n");

    close(sockfd);
    fclose(fp);
    free(data);
}

void *rcv_file(void *arg)
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
    char *data = calloc(1, BUFSIZ);
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
}

/**
 * @brief   主控程序
 * @param   argv[1] 对方的IP地址
 * @return  无
 */
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("请指定对方的IP地址!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t rcv_tid;
    pthread_t snd_tid;

    pthread_create(&rcv_tid, NULL, rcv_file, NULL);
    pthread_create(&snd_tid, NULL, snd_file, (void *)argv[1]);

    pthread_exit(NULL);
    return 0;
}
