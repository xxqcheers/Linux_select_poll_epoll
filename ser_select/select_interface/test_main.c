/*************************************************************************
	> File Name: server_select.c
	> Author: xuxiaoqiang
	> Mail:353271054@qq.com 
	> Created Time: 2014年07月30日 星期三 12时53分00秒
 ************************************************************************/

#include<stdio.h>
#include<sys/select.h>
#include<signal.h>
#include"select.h"
#include"network.h"
int main(int argc, const char *argv[])
{
    signal(SIGPIPE,SIG_IGN);//处理SIGPIPE信号

    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
        ERR_EXIT("socket");

    int on=1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0)
        ERR_EXIT("setsockopt");

    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(8989);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    int len=sizeof(servaddr);
    //bind
    int ret=bind(listenfd,(struct sockaddr*)&servaddr,len);
    if(ret<0)
        ERR_EXIT("bind");
    //listen
    ret=listen(listenfd,SOMAXCONN);
    if(ret<0)
        ERR_EXIT("listen");

    //采用select模型处理
    //初始化部分
    select_t sel;
    select_init(&sel,listenfd);
    while(1)
    {
        select_do_wait(&sel);
        select_handle_accept(&sel);
        select_handle_data(&sel);
    }
    close(listenfd);
    
    return 0;
}
