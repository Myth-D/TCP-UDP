#include "head.h"

#define BUF_SIZE 1024 * 1024 * 4

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("请指定对方的IP!\n");
        exit(EXIT_FAILURE);
    }

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
    addr.sin_addr.s_addr = inet_addr(argv[1]);
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
        int n = fread(data, BUF_SIZE, 1, fp);
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

    return 0;
}
