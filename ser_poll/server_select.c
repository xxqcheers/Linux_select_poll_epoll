/*************************************************************************
	> File Name: server_select.c
	> Author: xuxiaoqiang
	> Mail:353271054@qq.com 
	> Created Time: 2014年07月30日 星期三 12时53分00秒
 ************************************************************************/

#include<stdio.h>
#include<sys/select.h>
#include<signal.h>
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
    int i;
    int clients[FD_SETSIZE];
    for(i=0;i<FD_SETSIZE;++i)
        clients[i]=-1;
    int maxi=0;
    int maxfd=listenfd;
    int nready;
    fd_set allset;
    fd_set rset;
    //预处理部分
    FD_ZERO(&allset);
    FD_ZERO(&rset);
    FD_SET(listenfd,&allset);

    while(1)
    {
        //传递的参数：listenfd,
        rset=allset;
        nready=select(maxfd+1,&rset,NULL,NULL,NULL);
        if(nready==-1)
        {
            if(errno=EINTR)
                continue;
            else
                ERR_EXIT("select");
        }
        if(nready==0)
        {
            continue;
        }
        //测试准备好的
        if(FD_ISSET(listenfd,&rset))
        {
            //用于接收请求
            struct sockaddr_in peeraddr;
            memset(&peeraddr,0,sizeof(peeraddr));
            len=sizeof(peeraddr);
            //accept
            int peerfd=accept(listenfd,(struct sockaddr*)&peeraddr,&len);
            if(peerfd==-1)
            {
                ERR_EXIT("accept");
            }
            //将accept得到的描述符加入到clients中区
            int i;
            for(i=0;i<FD_SETSIZE;++i)
            {
                if(clients[i]==-1)
                {
                    clients[i]=peerfd;
                    if(i>maxi)
                    {
                        maxi=i;
                    }
                    break;
                }
            }
            //客户端太多
            if(i==FD_SETSIZE)
            {
                fprintf(stderr,"too many clients\n");
                exit(EXIT_FAILURE);
            }
            //将accept的那个描述符加入到allset中去
            FD_SET(peerfd,&allset);
            if(peerfd>maxfd)
            {
                maxfd=peerfd;
            }
            fprintf(stdout,"IP=%s,port=%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
            //要注意nready这个值中存放的是什么东东
            if(--nready<=0)
            {
                continue;
            }
        }
        int i;
        //遍历clients数组中的东东
        //轮询客户fd
        for(i=0;i<=maxi;i++)
        {
            if(FD_ISSET(clients[i],&rset))
            {
                char recvbuf[1024]={0};
                int fd=clients[i];
                int ret=readline(fd,recvbuf,1024);
                if(ret==-1)
                {
                    ERR_EXIT("readline");
                }
                if(ret==0)
                {
                    fputs("client close\n",stdout);
                    FD_CLR(fd,&allset);
                    clients[i]=-1;
                    close(fd);
                    continue;//继续下一次循环变量查找准备好的描述符
                }
                fprintf(stdout,"receive:%s",recvbuf);
                writen(fd,recvbuf,strlen(recvbuf));
                //跟上面的那个检查的效果是一样的
                if(--nready<=0)
                {
                    break;
                }
            }
        }
    }
    close(listenfd);
    return 0;
}

void do_poll(int listenfd)
{
    //采用poll模型
    struct pollfd clients[2048];
    int i;
    for(i=0;i<2048;i++)
    {
        clients[i]=-1;
    }
    clients[0].fd=listenfd;//将用于监听的那个文件描述符放入里面
    clients[0].events=POLLIN;

    int maxi=0;
    int nready;
    while(1)
    {
        nready=poll(clients,)
    }
}
