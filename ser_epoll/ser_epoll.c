/*************************************************************************
	> File Name: ser_epoll.c
	> Author: xuxiaoqiang
	> Mail:353271054@qq.com 
	> Created Time: 2014年07月30日 星期三 16时56分06秒
 ************************************************************************/

#include"network.h"
#include<sys/epoll.h>
int get_listen_fd();
void do_epoll(int listenfd);
int main(int argc, const char *argv[])
{
    if(signal(SIGPIPE,SIG_IGN)==SIG_ERR)
        ERR_EXIT("signal");

    int listenfd=get_listen_fd();

    do_epoll(listenfd);
    
    return 0;
}

int get_listen_fd()
{
    //创建socket
    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    if(listenfd==-1)
    {
        ERR_EXIT("socket");
    }
    //设置端口复用
    int on=1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADD,&on,sizeof(on))<0)
        ERR_EXIT("setsockopt");
    struct sockaddr_in servaddr;
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(8989);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    //bind端口
}
