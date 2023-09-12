#include "head.h"

/**
 * @brief   接收信息模块
 * @param   arg 无
 * @return  无
 */
void *rcv_msg(void *arg)
{
}

/**
 * @brief   发送消息模块
 * @param   IP 对方的IP的地址
 * @return  无
 */
void *snd_msg(void *IP)
{
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
