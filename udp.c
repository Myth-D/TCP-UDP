#include "head.h"

#define BUF_LEN 128

/**
 * @brief   接收信息模块
 * @param   arg 无
 * @return  无
 */
void *rcv_msg(void *arg)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket()失败");
        pthread_exit(NULL);
    }

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); /* 自动获取本地IP */
    addr.sin_port = htons(54321);             /* host to network short */

    bind(sockfd, (struct sockaddr *)&addr, len);

    char *buf = calloc(1, BUF_LEN);
    while (1)
    {
        bzero(buf, BUF_LEN);
        recvfrom(sockfd, buf, BUF_LEN, 0, NULL, NULL);
        printf("rcv: %s", buf);
    }
}

/**
 * @brief   发送消息模块
 * @param   IP 对方的IP的地址
 * @return  无
 */
void *snd_msg(void *IP)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket()失败");
        pthread_exit(NULL);
    }

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr((char *)IP);
    addr.sin_port = htons(54321); /* host to network short */

    /* 给对方发数据 */
    char *buf = calloc(1, BUF_LEN);
    while (1)
    {
        bzero(buf, BUF_LEN);
        fgets(buf, BUF_LEN, stdin);

        sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, len);
    }
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

    pthread_create(rcv_tid, NULL, rcv_msg, NULL);
    pthread_create(snd_tid, NULL, snd_msg, (void *)argv[1]);

    pthread_exit(NULL);
    return 0;
}
