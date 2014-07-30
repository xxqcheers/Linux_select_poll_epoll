/*************************************************************************
	> File Name: server_process.c
	> Author: xuxiaoqiang
	> Mail:353271054@qq.com 
	> Created Time: 2014年07月30日 星期三 10时40分28秒
 ************************************************************************/

#include<stdio.h>
#include"network.h"
void do_service(int peerfd)
{
    char recvbuf[1024]={0};
    int ret;
    while(1)
    {
        ret=readline(peerfd,recvbuf,1024);
        if(ret==0)
        {
            close(peerfd);//也就是说当返回值的时候需要关心，这时一种什么情形: 是不是这种情形，客户端已经关闭了客户端的套接字描述符
            exit(EXIT_SUCCESS);
        }
    }
}
